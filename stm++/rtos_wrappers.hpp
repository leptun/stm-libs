#pragma once

#include "cmsis_os.h"

class base_Wrapper {
public:
	base_Wrapper(const char *name, void *resource) : name(name), resource(resource) {};
	void Init();
	const char *name;
	void* resource;
	osMutexId_t lock;
	osEventFlagsId_t flags;
};

#if defined(HAL_I2C_MODULE_ENABLED) && USE_HAL_I2C_REGISTER_CALLBACKS
class I2C_Wrapper : public base_Wrapper {
public:
	I2C_Wrapper(I2C_HandleTypeDef *hi2c, const char *name) : base_Wrapper(name, hi2c) {};
};
#endif //defined(HAL_I2C_MODULE_ENABLED) && USE_HAL_I2C_REGISTER_CALLBACKS

#if defined(HAL_SPI_MODULE_ENABLED) && USE_HAL_SPI_REGISTER_CALLBACKS
class SPI_Wrapper : public base_Wrapper {
public:
	SPI_Wrapper(SPI_HandleTypeDef *hspi, const char *name) : base_Wrapper(name, hspi) {};
};
#endif //defined(HAL_SPI_MODULE_ENABLED) && USE_HAL_SPI_REGISTER_CALLBACKS
