#include "./AT24C.hpp"
#include <cstdio>
#include <algorithm>

int AT24C::_AT24C_cnt = 0;

void AT24C::init()
{
	i2c->Init();
	if (!lock) {
		snprintf(name, sizeof(name), "AT24C_%i", _AT24C_cnt++);
		const osMutexAttr_t attrM = {
			.name = name,
		};
		lock = osMutexNew(&attrM);
	}
}

void AT24C::Read(uint16_t address, uint8_t* pData, uint16_t len)
{
	if (osMutexAcquire(lock, 1000) != osOK)
		Error_Handler();
	if (osMutexAcquire(i2c->lock, 1000) != osOK)
		Error_Handler();
	HAL_I2C_RegisterCallback(i2c->hi2c, HAL_I2C_MSPINIT_CB_ID, (pI2C_CallbackTypeDef)this);
	HAL_I2C_RegisterCallback(i2c->hi2c, HAL_I2C_MEM_RX_COMPLETE_CB_ID, _i2c_dma_complete);
	if (HAL_I2C_Mem_Read_DMA(i2c->hi2c, ((i2cAddress | (address >> 8)) << 1), (address & 0xFF), I2C_MEMADD_SIZE_8BIT, pData, len) != HAL_OK)
		Error_Handler();
	if (osEventFlagsWait(i2c->flags, 0x01, osFlagsWaitAny, 1000) != 0x01) {
		Error_Handler();
	}
	osMutexRelease(i2c->lock);
	osMutexRelease(lock);
}

void AT24C::Write(uint16_t address, uint8_t* pData, uint16_t len)
{
	if (osMutexAcquire(lock, 1000) != osOK)
		Error_Handler();
	while (len) {
		uint16_t pageSize = (memSize > 256) ? 16 : 8;
		uint16_t addressEnd = (address & ~(pageSize - 1)) + pageSize;
		uint16_t writeSize = std::min((uint16_t)(addressEnd - address), len);
		if (osMutexAcquire(i2c->lock, 1000) != osOK)
			Error_Handler();
		HAL_I2C_RegisterCallback(i2c->hi2c, HAL_I2C_MSPINIT_CB_ID, (pI2C_CallbackTypeDef)this);
		HAL_I2C_RegisterCallback(i2c->hi2c, HAL_I2C_MEM_TX_COMPLETE_CB_ID, _i2c_dma_complete);
		if (HAL_I2C_Mem_Write_DMA(i2c->hi2c, ((i2cAddress | (address >> 8)) << 1), (address & 0xFF), I2C_MEMADD_SIZE_8BIT, pData, writeSize) != HAL_OK)
			Error_Handler();
		if (osEventFlagsWait(i2c->flags, 0x01, osFlagsWaitAny, 1000) != 0x01) {
			Error_Handler();
		}
		osMutexRelease(i2c->lock);
		pData += writeSize;
		len -= writeSize;
		address = addressEnd;
		osDelay(5); //self timed page write sequence
	}

	osMutexRelease(lock);
}

void AT24C::_i2c_dma_complete(I2C_HandleTypeDef *hi2c)
{
	AT24C* aista = (AT24C*)((void*)(hi2c->MspInitCallback));
	osEventFlagsSet(aista->i2c->flags, 0x01);
}
