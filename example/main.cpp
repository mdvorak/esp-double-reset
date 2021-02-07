#include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <double_reset.h>
#include <driver/gpio.h>

static const char TAG[] = "example";

static const auto STATUS_LED_GPIO = GPIO_NUM_22;
static const uint8_t STATUS_LED_ON = 0;
static const uint8_t STATUS_LED_OFF = (~STATUS_LED_ON) & 1;

void setup()
{
	esp_log_level_set("wifi", ESP_LOG_WARN);
	esp_log_level_set("double_reset", ESP_LOG_DEBUG);

	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Detect double-reset
	bool reconfigure = false;
	ESP_ERROR_CHECK(double_reset_start(&reconfigure, DOUBLE_RESET_TIMEOUT_DEFAULT));

	// Status LED
	gpio_reset_pin(STATUS_LED_GPIO);
	gpio_set_direction(STATUS_LED_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_level(STATUS_LED_GPIO, STATUS_LED_ON);

	// Reconfigure logic
	if (reconfigure)
	{
		ESP_LOGI(TAG, "double reset detected!");

		// Custom mode of operation
		bool status_led = true;
		TickType_t started = xTaskGetTickCount();

		for (;;)
		{
			gpio_set_level(STATUS_LED_GPIO, (status_led = !status_led) ? STATUS_LED_ON : STATUS_LED_OFF);
			vTaskDelay(50 / portTICK_PERIOD_MS);

			// NOTE since double reset, due to its imperfect implementation, can be triggered randomly, special mode should always
			// drop to normal mode after some timeout
			if ((xTaskGetTickCount() - started) > 60000 / portTICK_PERIOD_MS)
			{
				break;
			}
		}
	}

	// Wait for reset
	// NOTE this is trivial implementation, delaying startup, just to turn of status led, you might want more sophisticated logic
	double_reset_wait();
	gpio_set_level(STATUS_LED_GPIO, STATUS_LED_OFF);

	// Setup complete
	ESP_LOGI(TAG, "started");
}

void loop()
{
	vTaskDelay(1000 / portTICK_PERIOD_MS);
}

extern "C" _Noreturn void app_main()
{
	setup();
	for (;;)
	{
		loop();
	}
}
