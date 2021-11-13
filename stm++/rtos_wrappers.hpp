#pragma once

#include "main.h"
#include "cmsis_os.h"

class base_Wrapper {
public:
	base_Wrapper(const char *name) : name(name) {};
	void Init();
	const char *name;
	osMutexId_t lock;
	osEventFlagsId_t flags;
};

#ifdef HAL_I2C
class I2C_Wrapper : public base_Wrapper {
public:
	I2C_Wrapper(I2C_HandleTypeDef *hi2c, const char *name) : base_Wrapper(name), hi2c(hi2c) {};
	I2C_HandleTypeDef *hi2c;
};
#endif

#ifdef HAL_SPI
class SPI_Wrapper : public base_Wrapper {
public:
	SPI_Wrapper(SPI_HandleTypeDef *hspi, const char *name) : base_Wrapper(name), hspi(hspi) {};
	SPI_HandleTypeDef *hspi;
};
#endif
