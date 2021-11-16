#include "./USB2504A.hpp"
#include <cstdio>

int USB2504A::_USB2504A_cnt = 0;

void USB2504A::init() {
	i2c->Init();
	if (!lock) {
		snprintf(name, sizeof(name), "USB2504A_%i", _USB2504A_cnt++);
		const osMutexAttr_t attrM = {
			.name = name,
		};
		lock = osMutexNew(&attrM);
	}
}

void USB2504A::reset() {
	HAL_GPIO_WritePin(vbus.port, vbus.pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(rst.port, rst.pin, GPIO_PIN_RESET);
	osDelay(1);
	HAL_GPIO_WritePin(rst.port, rst.pin, GPIO_PIN_SET);
	osDelay(100);
}

void USB2504A::writeConfig(Config config) {
	Write(Registers::VIDL, config.VID & 0xFF);
	Write(Registers::VIDM, (config.VID >> 8) & 0xFF);
	Write(Registers::PIDL, config.PID & 0xFF);
	Write(Registers::PIDM, (config.PID >> 8) & 0xFF);
	Write(Registers::DIDL, config.DID & 0xFF);
	Write(Registers::DIDM, (config.DID >> 8) & 0xFF);
	Write(Registers::CFG1, config.CFG1);
	Write(Registers::CFG2, config.CFG2);
	Write(Registers::NRD, config.NRD);
	Write(Registers::PDS, config.PDS);
	Write(Registers::PDB, config.PDB);
	Write(Registers::MAXPS, config.MAXPS);
	Write(Registers::MAXPB, config.MAXPB);
	Write(Registers::HCMCS, config.HCMCS);
	Write(Registers::HCMCB, config.HCMCB);
	Write(Registers::PWRT, config.PWRT);
}

void USB2504A::attach() {
	Write(Registers::STCD, 0x01);
	HAL_GPIO_WritePin(vbus.port, vbus.pin, GPIO_PIN_SET);
}


void USB2504A::Write(Registers reg, uint8_t data) {
	if (osMutexAcquire(lock, 1000) != osOK)
		Error_Handler();
	if (osMutexAcquire(i2c->lock, 1000) != osOK)
		Error_Handler();
	HAL_I2C_RegisterCallback((I2C_HandleTypeDef*)i2c->resource, HAL_I2C_MSPINIT_CB_ID, (pI2C_CallbackTypeDef)this);
	HAL_I2C_RegisterCallback((I2C_HandleTypeDef*)i2c->resource, HAL_I2C_MEM_TX_COMPLETE_CB_ID, _i2c_dma_complete);
	if (HAL_I2C_Mem_Write_DMA((I2C_HandleTypeDef*)i2c->resource, i2cAddress, (uint8_t)reg, I2C_MEMADD_SIZE_8BIT, &data, 1) != HAL_OK)
		Error_Handler();
	if (osEventFlagsWait(i2c->flags, 0x01, osFlagsWaitAny, 1000) != 0x01) {
		Error_Handler();
	}
	osMutexRelease(i2c->lock);
	osMutexRelease(lock);
}

void USB2504A::_i2c_dma_complete(I2C_HandleTypeDef *hi2c) {
	USB2504A* aista = (USB2504A*)((void*)(hi2c->MspInitCallback));
	osEventFlagsSet(aista->i2c->flags, 0x01);
}
