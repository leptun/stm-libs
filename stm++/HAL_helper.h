#pragma once
#include "main.h"


#if\
	defined(STM32F1xx_HAL_SPI_H) ||\
	defined(STM32G4xx_HAL_SPI_H)
#define HAL_HAS_SPI
#endif

#if\
	defined(__STM32F1xx_HAL_I2C_H) ||\
	defined(STM32G4xx_HAL_I2C_H)
#define HAL_HAS_I2C
#endif
