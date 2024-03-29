#include <double_reset.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

static const char TAG[] = "example";

static const gpio_num_t STATUS_LED_GPIO = (gpio_num_t)CONFIG_STATUS_LED_GPIO;
static const uint32_t STATUS_LED_ON = CONFIG_STATUS_LED_ON;
static const uint8_t STATUS_LED_OFF = (~STATUS_LED_ON) & 1;

_Noreturn void app_main()
{
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
    ESP_ERROR_CHECK(double_reset_start(&reconfigure, DOUBLE_RESET_DEFAULT_TIMEOUT));

    // Status LED
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_reset_pin(STATUS_LED_GPIO));
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_direction(STATUS_LED_GPIO, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(STATUS_LED_GPIO, STATUS_LED_ON));

    // Reconfigure logic
    if (reconfigure)
    {
        ESP_LOGI(TAG, "double reset detected!");

        // Custom mode of operation
        bool status_led = true;
        const TickType_t started = xTaskGetTickCount();

        // NOTE since double reset, due to its imperfect implementation, can be triggered randomly, special mode should always
        // drop to normal mode after some timeout
        while (xTaskGetTickCount() - started < 60000 / portTICK_PERIOD_MS)
        {
            gpio_set_level(STATUS_LED_GPIO, (status_led = !status_led) ? STATUS_LED_ON : STATUS_LED_OFF);
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    }

    // Wait for reset - usually not needed
    double_reset_wait();
    gpio_set_level(STATUS_LED_GPIO, STATUS_LED_OFF);

    // Setup complete
    ESP_LOGI(TAG, "started");

    while (true)
    {
        vTaskDelay(1);
    }
}
