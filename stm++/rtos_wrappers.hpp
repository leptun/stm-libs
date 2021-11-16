#pragma once

#include "main.h"
#include "cmsis_os.h"

class base_Wrapper {
public:
	base_Wrapper(const char *name, void *resource) : name(name) {};
	void Init();
	const char *name;
	void* resource;
	osMutexId_t lock;
	osEventFlagsId_t flags;
};

class I2C_Wrapper : public base_Wrapper {
public:
	I2C_Wrapper(I2C_HandleTypeDef *hi2c, const char *name) : base_Wrapper(name, hi2c) {};
};

class SPI_Wrapper : public base_Wrapper {
public:
	SPI_Wrapper(SPI_HandleTypeDef *hspi, const char *name) : base_Wrapper(name, hspi) {};
};
