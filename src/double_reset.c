#include "double_reset.h"
#include <esp_log.h>
#include <nvs.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char TAG[] = "double_reset";

static volatile uint32_t double_reset_timeout = 10000;

static void double_reset_task(void *p)
{
    // Wait defined delay
    vTaskDelay(double_reset_timeout / portTICK_PERIOD_MS);

    // Open NVS
    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open(TAG, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "failed to open nvs in the task: %d %s", err, esp_err_to_name(err));
        vTaskDelete(NULL);
        return;
    }

    // Delete it
    err = nvs_erase_all(handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "failed to erase nvs in the task: %d %s", err, esp_err_to_name(err));
        nvs_close(handle);
        vTaskDelete(NULL);
        return;
    }

    // Commit
    err = nvs_commit(handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "failed to commit nvs in the task: %d %s", err, esp_err_to_name(err));
        nvs_close(handle);
        vTaskDelete(NULL);
        return;
    }

    // Done
    nvs_close(handle);
    ESP_LOGI(TAG, "double reset flag cleared");
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
    nvs_get_u8(handle, "state", &val); // Ignore error

    // Store result
    *result = val != 0;

    // If reset detected, reset the value and don't start background task
    if (val != 0)
    {
        ESP_LOGI(TAG, "double reset detected");

        // Reset, commit and close
        err = nvs_erase_all(handle);
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

        nvs_close(handle);
        return ESP_OK;
    }
    else
    {
        // Store flag
        err = nvs_set_u8(handle, "state", 1);
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

        // Create background task, that resets the status
        BaseType_t ret = xTaskCreate(double_reset_task, "double_reset", 4096, NULL, tskIDLE_PRIORITY + 1, NULL);
        return ret == pdPASS ? ESP_OK : ESP_FAIL;
    }
}
