#include "double_reset.h"
#include <esp_log.h>
#include <nvs.h>

static const char TAG[] = "double_reset";

esp_err_t double_reset_start(bool *result, uint32_t timeout)
{
    ESP_LOGI(TAG, "started");
    return ESP_OK;
}
