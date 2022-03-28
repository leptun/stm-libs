#include "I2C_slave.hpp"
#include "I2C_slave_interrupt.h"

#if defined(HAL_I2C_MODULE_ENABLED)

void i2c_slave_manager_event_irq(I2C_HandleTypeDef *hi2c) {
	I2C_slave_manager * aista = (I2C_slave_manager *)((void *)hi2c->pBuffPtr);
	aista->event_isr();
}

void i2c_slave_manager_error_irq(I2C_HandleTypeDef *hi2c) {
	I2C_slave_manager * aista = (I2C_slave_manager *)((void *)hi2c->pBuffPtr);
		aista->error_isr();
}

#endif //#defined(HAL_I2C_MODULE_ENABLED)
