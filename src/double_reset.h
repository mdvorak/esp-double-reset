#ifndef DOUBLE_RESET_H
#define DOUBLE_RESET_H

#include <esp_err.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif
    const uint32_t DOUBLE_RESET_DEFAULT_TIMEOUT = CONFIG_DOUBLE_RESET_DEFAULT_TIMEOUT;

    esp_err_t double_reset_start(bool *result, uint32_t timeout_ms);

    bool double_reset_pending();

    void double_reset_wait();

#ifdef __cplusplus
}
#endif

#endif
