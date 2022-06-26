#pragma once
#include "main.h"

#if defined(HAL_I2C_MODULE_ENABLED) && defined(I2C_OA2_MASK07)

#ifdef __cplusplus
extern "C" {
#endif

extern void i2c_slave_manager_event_irq(I2C_HandleTypeDef *hi2c);
extern void i2c_slave_manager_error_irq(I2C_HandleTypeDef *hi2c);

#ifdef __cplusplus
}
#endif

#endif //#defined(HAL_I2C_MODULE_ENABLED) && defined(I2C_OA2_MASK07)
