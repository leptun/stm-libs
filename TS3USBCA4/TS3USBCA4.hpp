#pragma once

#include "stm++/IO.hpp"
#include "stm++/rtos_wrappers.hpp"

class TS3USBCA4 {
public:
	enum class Channels : uint8_t {
		LnB,
		MICGND,
		LnC,
		LnA,
		DISABLED = 0xFF,
	};
	enum class Registers : uint8_t {
		Revision_ID = 0x09,
		General_1 = 0x0A,
		General_2 = 0x0B,
	};

	TS3USBCA4(uint8_t address, I2C_Wrapper *i2c) : i2cAddress(address), i2c(i2c) {};
	void init();
	void setChannel(Channels ch, bool flipped);

private:
	uint8_t i2cAddress;
	IO rst;
	IO vbus;
	I2C_Wrapper *i2c;
	osMutexId_t lock;
	static int _TS3USBCA4_cnt;
	char name[configMAX_TASK_NAME_LEN];

	void Write(Registers reg, uint8_t data);

	static void _i2c_dma_complete(I2C_HandleTypeDef *hi2c);
};
