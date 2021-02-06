#include "double_reset.h"
#include <esp_log.h>
#include <nvs.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char TAG[] = "double_reset";
static const char KEY[] = "state";

static volatile uint32_t double_reset_timeout = 10000;

static esp_err_t double_reset_clear_state(nvs_handle_t handle)
{
    uint8_t val = 0;
    nvs_get_u8(handle, KEY, &val); // Ignore error

    // No-op
    if (val == 0)
    {
        return ESP_OK;
    }

    // Reset, commit and close
    esp_err_t err = nvs_erase_all(handle);
    if (err != ESP_OK)
        return err;

    err = nvs_commit(handle);
    if (err != ESP_OK)
        return err;

    ESP_LOGI(TAG, "double reset flag cleared");
    return ESP_OK;
}

static void double_reset_task(void *p)
{
    ESP_LOGD(TAG, "double reset task started");

    // Wait defined delay
    vTaskDelay(double_reset_timeout / portTICK_PERIOD_MS);

    // Open nvs
    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open(TAG, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "failed to open nvs in the task: %d %s", err, esp_err_to_name(err));
        vTaskDelete(NULL);
        return;
    }

    // Reset state
    err = double_reset_clear_state(handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "failed to erase nvs in the task: %d %s", err, esp_err_to_name(err));
        nvs_close(handle);
        vTaskDelete(NULL);
        return;
    }

    // Done
    nvs_close(handle);
    vTaskDelete(NULL);
}

esp_err_t double_reset_start(bool *result, uint32_t timeout)
{
    esp_err_t err;

    nvs_handle_t handle = 0;
    err = nvs_open(TAG, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        return err;
    }

    uint8_t val = 0;

    // Detect reset only when reset button is pressed
    // NOTE there is no way to distinguish between first power-on and reset, unless you would use actual RTC
    esp_reset_reason_t reset_reason = esp_reset_reason();

    if (reset_reason == ESP_RST_POWERON || reset_reason == ESP_RST_EXT)
    {
        // Get last state
        nvs_get_u8(handle, KEY, &val); // Ignore error
        *result = val != 0;
    }
    else
    {
        ESP_LOGI(TAG, "ignoring, reset reason: %d", reset_reason);

        // Always false
        *result = false;

        // Clear state
        err = double_reset_clear_state(handle);
        if (err != ESP_OK)
        {
            nvs_close(handle);
            return err;
        }

        // Return
        nvs_close(handle);
        return ESP_OK;
    }

    // If reset detected, reset the value and don't start background task
    if (val != 0)
    {
        ESP_LOGI(TAG, "double reset detected");

        // Clear state
        err = double_reset_clear_state(handle);
        if (err != ESP_OK)
        {
            nvs_close(handle);
            return err;
        }

        nvs_close(handle);
        return ESP_OK;
    }
    else
    {
        // Store flag
        err = nvs_set_u8(handle, KEY, 1);
        if (err != ESP_OK)
        {
            nvs_close(handle);
            return err;
        }

        err = nvs_commit(handle);
        if (err != ESP_OK)
        {
            nvs_close(handle);
            return err;
        }

        // Close handle
        nvs_close(handle);

        // Store timeout
        double_reset_timeout = timeout;
        ESP_LOGI(TAG, "double reset flag set, waiting for %d ms", timeout);

        // Create background task, that resets the status
        BaseType_t ret = xTaskCreate(double_reset_task, "double_reset", 4096, NULL, tskIDLE_PRIORITY + 1, NULL);
        return ret == pdPASS ? ESP_OK : ESP_FAIL;
    }
}
