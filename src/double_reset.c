#include "double_reset.h"
#include <esp_log.h>
#include <nvs.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

static const char TAG[] = "double_reset";
static const char KEY[] = "state";

static volatile uint32_t double_reset_timeout = 5000;
static EventGroupHandle_t double_reset_group;

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
        goto finalize;
    }

    // Reset state
    err = double_reset_clear_state(handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "failed to erase nvs in the task: %d %s", err, esp_err_to_name(err));
        goto finalize;
    }

finalize:
    // Done
    nvs_close(handle);
    xEventGroupSetBits(double_reset_group, BIT0);

    // Delete self
    vTaskDelete(NULL);
}

esp_err_t double_reset_start(bool *result, uint32_t timeout_ms)
{
    esp_err_t err;

    // Prepare event group
    double_reset_group = xEventGroupCreate();
    configASSERT(double_reset_group);

    // Open nvs
    nvs_handle_t handle = 0;
    err = nvs_open(TAG, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        xEventGroupSetBits(double_reset_group, BIT0);
        return err;
    }

    uint8_t val = 0;

    // Detect reset only when reset button is pressed
    // NOTE there is no way to distinguish between first power-on and reset, unless you would use actual RTC
    esp_reset_reason_t reset_reason = esp_reset_reason();
    bool reset_state = false;

    if (reset_reason == ESP_RST_POWERON || reset_reason == ESP_RST_EXT)
    {
        // Get last state
        err = nvs_get_u8(handle, KEY, &val); // Ignore error

        if (val != 0)
        {
            // Positive, store result and reset state
            ESP_LOGI(TAG, "double reset detected");
            *result = true;
            reset_state = true;
        }
        else
        {
            // Negative, set flag and start background task
            *result = false;
        }
    }
    else
    {
        ESP_LOGI(TAG, "ignoring, reset reason: %d", reset_reason);

        // Always false
        *result = false;
        // But clear state
        reset_state = true;
    }

    // If reset detected, reset the value and don't start background task
    if (reset_state)
    {
        // Clear state
        err = double_reset_clear_state(handle);
        if (err != ESP_OK)
        {
            nvs_close(handle);
            xEventGroupSetBits(double_reset_group, BIT0);
            return err;
        }

        // Done
        nvs_close(handle);
        xEventGroupSetBits(double_reset_group, BIT0);
        return ESP_OK;
    }
    else
    {
        // Store flag
        err = nvs_set_u8(handle, KEY, 1);
        if (err != ESP_OK)
        {
            nvs_close(handle);
            xEventGroupSetBits(double_reset_group, BIT0);
            return err;
        }

        err = nvs_commit(handle);
        if (err != ESP_OK)
        {
            nvs_close(handle);
            xEventGroupSetBits(double_reset_group, BIT0);
            return err;
        }

        // Close handle
        nvs_close(handle);

        // Store timeout
        double_reset_timeout = timeout_ms;
        ESP_LOGI(TAG, "double reset flag set, waiting for %d ms", timeout_ms);

        // Create background task, that resets the status
        BaseType_t ret = xTaskCreate(double_reset_task, "double_reset", 4096, NULL, tskIDLE_PRIORITY + 1, NULL);
        return ret == pdPASS ? ESP_OK : ESP_FAIL;
    }
}

bool double_reset_pending()
{
    return (xEventGroupGetBits(double_reset_group) & BIT0) != 0;
}

void double_reset_wait()
{
    xEventGroupWaitBits(double_reset_group, BIT0, pdFALSE, pdTRUE, double_reset_timeout);
}
