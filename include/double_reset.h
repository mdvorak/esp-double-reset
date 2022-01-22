#ifndef DOUBLE_RESET_H
#define DOUBLE_RESET_H

#include <esp_err.h>
#include <stdbool.h>
#include "sdkconfig.h"

#ifndef DOUBLE_RESET_DEFAULT_TIMEOUT
#define DOUBLE_RESET_DEFAULT_TIMEOUT CONFIG_DOUBLE_RESET_DEFAULT_TIMEOUT
#endif

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t double_reset_start(bool *result, uint32_t timeout_ms);

/**
 * @brief Overrides stored double reset state.
 *
 * @param state Desired state.
 * @return ESP_OK if operation was completed successfully.
 */
esp_err_t double_reset_set(bool state);

/**
 * @brief Returns true if double reset timer is still running.
 *
 * @return true if restart at that time will trigger double-reset mode.
 */
bool double_reset_pending();

void double_reset_wait();

#ifdef __cplusplus
}
#endif

#endif
