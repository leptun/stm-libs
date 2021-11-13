#pragma once

#include "stm++/rtos_wrappers.hpp"

class AT24C {
public:
	AT24C(uint8_t address, uint16_t size, I2C_Wrapper *i2c) : i2cAddress(address), memSize(size), i2c(i2c) {};
	void init();

	void Read(uint16_t address, uint8_t* pData, uint16_t len);
	template<typename T>
	T Read(uint16_t address) {
		T val;
		Read(address, (uint8_t*)&val, sizeof(T));
		return val;
	}

	void Write(uint16_t address, uint8_t* pData, uint16_t len);
	template<typename T>
	void Write(uint16_t address, T val) {
		Write(address, (uint8_t*)&val, sizeof(T));
	}

private:
	uint8_t i2cAddress;
	uint16_t memSize;
	I2C_Wrapper *i2c;
	osMutexId_t lock;
	static int _AT24C_cnt;
	char name[configMAX_TASK_NAME_LEN];

	static void _i2c_dma_complete(I2C_HandleTypeDef *hi2c);
};
