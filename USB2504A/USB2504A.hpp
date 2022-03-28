#pragma once

#include "stm++/IO.hpp"
#include "stm++/rtos_wrappers.hpp"

#if defined(HAL_I2C_MODULE_ENABLED) && USE_HAL_I2C_REGISTER_CALLBACKS

class USB2504A {
public:
	struct Config {
		uint16_t VID;
		uint16_t PID;
		uint16_t DID;
		uint8_t CFG1;
		uint8_t CFG2;
		uint8_t NRD;
		uint8_t PDS;
		uint8_t PDB;
		uint8_t MAXPS;
		uint8_t MAXPB;
		uint8_t HCMCS;
		uint8_t HCMCB;
		uint8_t PWRT;
	};

	enum class Registers : uint8_t {
		STCD = 0x00,
		VIDL = 0x01,
		VIDM = 0x02,
		PIDL = 0x03,
		PIDM = 0x04,
		DIDL = 0x05,
		DIDM = 0x06,
		CFG1 = 0x07,
		CFG2 = 0x08,
		NRD = 0x09,
		PDS = 0x0A,
		PDB = 0x0B,
		MAXPS = 0x0C,
		MAXPB = 0x0D,
		HCMCS = 0x0E,
		HCMCB = 0x0F,
		PWRT = 0x10,
	};

	USB2504A(uint8_t address, IO rst, IO vbus, I2C_Wrapper *i2c) : i2cAddress(address), rst(rst), vbus(vbus), i2c(i2c) {};
	void init();
	void reset();
	void writeConfig(Config config);
	void attach();

private:
	uint8_t i2cAddress;
	IO rst;
	IO vbus;
	I2C_Wrapper *i2c;
	osMutexId_t lock;
	static int _USB2504A_cnt;
	char name[configMAX_TASK_NAME_LEN];

	void Write(Registers reg, uint8_t data);

	static void _i2c_dma_complete(I2C_HandleTypeDef *hi2c);
};

#endif //defined(HAL_I2C_MODULE_ENABLED) && USE_HAL_I2C_REGISTER_CALLBACKS
