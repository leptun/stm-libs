#include "./L3GD20H.hpp"
#include <cstring>
#include <algorithm>

void L3GD20H_base::init()
{
	interface->Init();
}

#if defined(HAL_I2C_MODULE_ENABLED) && USE_HAL_I2C_REGISTER_CALLBACKS
void L3GD20H_i2c::WriteRegs(Registers reg, const uint8_t* buffer, size_t buff_size)
{
	HAL_I2C_Mem_Write((I2C_HandleTypeDef*)interface->resource, (i2cAddress << 1), ((uint8_t)reg) | 0x80, I2C_MEMADD_SIZE_8BIT, (uint8_t*)buffer, buff_size, HAL_MAX_DELAY);
}

void L3GD20H_i2c::ReadRegs(Registers reg, uint8_t* buffer, size_t buff_size)
{
	HAL_I2C_Mem_Read((I2C_HandleTypeDef*)interface->resource, (i2cAddress << 1), ((uint8_t)reg) | 0x80, I2C_MEMADD_SIZE_8BIT, buffer, buff_size, HAL_MAX_DELAY);
}

#endif //defined(HAL_I2C_MODULE_ENABLED) && USE_HAL_I2C_REGISTER_CALLBACKS

#if defined(HAL_SPI_MODULE_ENABLED) && USE_HAL_SPI_REGISTER_CALLBACKS
void L3GD20H_spi::WriteRegs(Registers reg, const uint8_t* buffer, size_t buff_size)
{
	uint8_t header = 0x40 | (uint8_t)reg;
	HAL_GPIO_WritePin(CS.port, CS.pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit((SPI_HandleTypeDef*)interface->resource, &header, 1, HAL_MAX_DELAY);
	HAL_SPI_Transmit((SPI_HandleTypeDef*)interface->resource, (uint8_t*)buffer, buff_size, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(CS.port, CS.pin, GPIO_PIN_SET);
}

void L3GD20H_spi::ReadRegs(Registers reg, uint8_t* buffer, size_t buff_size)
{
	uint8_t header = 0x80 | 0x40 | (uint8_t)reg;
	HAL_GPIO_WritePin(CS.port, CS.pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit((SPI_HandleTypeDef*)interface->resource, &header, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive((SPI_HandleTypeDef*)interface->resource, buffer, buff_size, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(CS.port, CS.pin, GPIO_PIN_SET);
}

#endif //defined(HAL_SPI_MODULE_ENABLED) && USE_HAL_SPI_REGISTER_CALLBACKS
