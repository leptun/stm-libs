#pragma once
#include <inttypes.h>
#include <stddef.h>
#include "stm++/IO.hpp"
#include "stm++/rtos_wrappers.hpp"

class L3GD20H_base {
public:
	enum class Registers : uint8_t {
		WHO_AM_I = 0x0f,
		CTRL1 = 0x20,
		CTRL2 = 0x21,
		CTRL3 = 0x22,
		CTRL4 = 0x23,
		CTRL5 = 0x24,
		REFERENCE = 0x25,
		OUT_TEMP = 0x26,
		STATUS = 0x27,
		OUT_X_L = 0x28,
		OUT_X_H = 0x29,
		OUT_Y_L = 0x2a,
		OUT_Y_H = 0x2b,
		OUT_Z_L = 0x2c,
		OUT_Z_H = 0x2d,
		FIFO_CRTL = 0x2e,
		FIFO_SRC = 0x2f,
		IG_CFG = 0x30,
		IG_SRC = 0x31,
		IG_THS_XH = 0x32,
		IG_THS_XL = 0x33,
		IG_THS_YH = 0x34,
		IG_THS_YL = 0x35,
		IG_THS_ZH = 0x36,
		IG_THS_ZL = 0x37,
		IG_DURATION = 0x38,
		LOW_ODR = 0x39,
	};

	L3GD20H_base(base_Wrapper *interface, IO DRDY = {0}) : interface(interface), DRDY(DRDY) {};

	void init();

	virtual void WriteRegs(Registers reg, const uint8_t* buffer, size_t buff_size) = 0;
	virtual void ReadRegs(Registers reg, uint8_t* buffer, size_t buff_size) = 0;

	void WriteReg(Registers reg, uint8_t val) {
		WriteRegs(reg, &val, 1);
	}

	uint8_t ReadReg(Registers reg) {
		uint8_t val = 0;
		ReadRegs(reg, &val, 1);
		return val;
	}

protected:
	base_Wrapper *interface;
	IO DRDY;

private:
};

#if defined(HAL_I2C_MODULE_ENABLED) && USE_HAL_I2C_REGISTER_CALLBACKS
class L3GD20H_i2c : public L3GD20H_base {
public:
	L3GD20H_i2c(I2C_Wrapper *i2c, uint8_t address = 0x6A, IO DRDY = {0}) : L3GD20H_base(i2c, DRDY), i2cAddress(address) {};

	void WriteRegs(Registers reg, const uint8_t* buffer, size_t buff_size) override;
	void ReadRegs(Registers reg, uint8_t* buffer, size_t buff_size) override;
private:
	uint8_t i2cAddress;
};
#endif //defined(HAL_I2C_MODULE_ENABLED) && USE_HAL_I2C_REGISTER_CALLBACKS

#if defined(HAL_SPI_MODULE_ENABLED) && USE_HAL_SPI_REGISTER_CALLBACKS
class L3GD20H_spi : public L3GD20H_base {
public:
	L3GD20H_spi(SPI_Wrapper *spi, IO CS, IO DRDY = {0}) : L3GD20H_base(spi, DRDY), CS(CS) {};

	void WriteRegs(Registers reg, const uint8_t* buffer, size_t buff_size) override;
	void ReadRegs(Registers reg, uint8_t* buffer, size_t buff_size) override;
private:
	IO CS;
};
#endif //defined(HAL_SPI_MODULE_ENABLED) && USE_HAL_SPI_REGISTER_CALLBACKS
