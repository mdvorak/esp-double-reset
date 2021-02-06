#ifndef DOUBLE_RESET_H
#define DOUBLE_RESET_H

#include <esp_err.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t double_reset_start(bool *result, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif
