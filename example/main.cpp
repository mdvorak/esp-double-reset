#include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <double_reset.h>

static const char TAG[] = "example";

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
	ESP_ERROR_CHECK(double_reset_start(&reconfigure, 10000));

	if (reconfigure)
	{
		ESP_LOGI(TAG, "double reset detected!");
	}

	// Setup complete
	ESP_LOGI(TAG, "started");
}

void loop()
{
	vTaskDelay(1);
}

extern "C" void app_main()
{
	setup();
	for (;;)
	{
		loop();
	}
}
