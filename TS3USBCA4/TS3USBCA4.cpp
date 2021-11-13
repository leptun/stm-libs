#include "./TS3USBCA4.hpp"
#include <cstdio>

int TS3USBCA4::_TS3USBCA4_cnt = 0;

void TS3USBCA4::init() {
	i2c->Init();
	if (!lock) {
		snprintf(name, sizeof(name), "TS3USBCA4_%i", _TS3USBCA4_cnt++);
		const osMutexAttr_t attrM = {
			.name = name,
		};
		lock = osMutexNew(&attrM);
	}
}

void TS3USBCA4::setChannel(Channels ch, bool flipped) {
	if (ch == Channels::DISABLED) {
		Write(Registers::General_1, 0x00);
	}
	else {
		Write(Registers::General_1, 0x02 | (flipped ? 0x01 : 0x00));
		Write(Registers::General_2, (uint8_t)ch);
	}
}


void TS3USBCA4::Write(Registers reg, uint8_t data) {
	if (osMutexAcquire(lock, 1000) != osOK)
		Error_Handler();
	if (osMutexAcquire(i2c->lock, 1000) != osOK)
		Error_Handler();
	HAL_I2C_RegisterCallback(i2c->hi2c, HAL_I2C_MSPINIT_CB_ID, (pI2C_CallbackTypeDef)this);
	HAL_I2C_RegisterCallback(i2c->hi2c, HAL_I2C_MEM_TX_COMPLETE_CB_ID, _i2c_dma_complete);
	if (HAL_I2C_Mem_Write_DMA(i2c->hi2c, i2cAddress, (uint8_t)reg, I2C_MEMADD_SIZE_8BIT, &data, 1) != HAL_OK)
		Error_Handler();
	if (osEventFlagsWait(i2c->flags, 0x01, osFlagsWaitAny, 1000) != 0x01) {
		Error_Handler();
	}
	osMutexRelease(i2c->lock);
	osMutexRelease(lock);
}

void TS3USBCA4::_i2c_dma_complete(I2C_HandleTypeDef *hi2c) {
	TS3USBCA4* aista = (TS3USBCA4*)((void*)(hi2c->MspInitCallback));
	osEventFlagsSet(aista->i2c->flags, 0x01);
}
