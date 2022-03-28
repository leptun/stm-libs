#pragma once
#include "main.h"

#if defined(HAL_I2C_MODULE_ENABLED) && defined(I2C_OA2_MASK07)

class I2C_slave_base {
public:
	virtual void init() = 0;
	virtual bool event_address_match(uint8_t addr, bool dir) = 0;
	virtual void event_repeated_start(uint8_t addr, bool dir) = 0;
	virtual void event_stop(uint8_t addr, bool dir) = 0;
	virtual bool event_rx(uint8_t val) = 0;
	virtual uint8_t event_tx() = 0;

	I2C_HandleTypeDef *hi2c;
	I2C_slave_base *nextSlave = NULL;
};

class I2C_slave_manager {
public:
	I2C_slave_manager(I2C_HandleTypeDef *hi2c) : hi2c(hi2c) {};

	void registerSlave(I2C_slave_base *slv) {
		if (slavesBase == NULL)
			slavesBase = slv;
		else
			slavesBase->nextSlave = slv;
		slv->hi2c = hi2c;
	}

	void init() {
		for (I2C_slave_base *slv = slavesBase; slv; slv = slv->nextSlave) {
			slv->init();
		}

		hi2c->State = HAL_I2C_STATE_BUSY;
		hi2c->Mode = HAL_I2C_MODE_SLAVE;
		hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
		hi2c->pBuffPtr = (uint8_t *)this;

		__HAL_I2C_ENABLE_IT(hi2c, I2C_IT_ADDRI | I2C_IT_STOPI);
	}

	void event_isr() {
		if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_ADDR)) {
			uint8_t addr = (hi2c->Instance->ISR & I2C_ISR_ADDCODE_Msk) >> I2C_ISR_ADDCODE_Pos;
			uint8_t dir = (hi2c->Instance->ISR & I2C_ISR_DIR_Msk) >> I2C_ISR_DIR_Pos;
			if (!dir) {//if read
				hi2c->Instance->CR1 |= I2C_CR1_SBC;
				hi2c->Instance->CR2 |= I2C_CR2_RELOAD;
				setNBYTES(1);
				__HAL_I2C_ENABLE_IT(hi2c, I2C_IT_RXI);
				__HAL_I2C_DISABLE_IT(hi2c, I2C_IT_TXI);
			}
			else {
				hi2c->Instance->CR1 &= ~I2C_CR1_SBC;
				hi2c->Instance->CR2 &= ~I2C_CR2_RELOAD;
				setNBYTES(0);
				__HAL_I2C_ENABLE_IT(hi2c, I2C_IT_TXI);
				__HAL_I2C_DISABLE_IT(hi2c, I2C_IT_RXI);
			}

			if (activeSlave) {
				activeSlave->event_repeated_start(addr, dir);
			}

			activeSlave = NULL;
			for (I2C_slave_base *slv = slavesBase; slv; slv = slv->nextSlave) {
				if (slv->event_address_match(addr, dir)) {
					activeSlave = slv;
					break;
				}
			}

			__HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_TXE);
			__HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_ADDR);
		}
		else if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_STOPF)) {
			__HAL_I2C_DISABLE_IT(hi2c, I2C_IT_RXI | I2C_IT_TXI);
			uint8_t addr = (hi2c->Instance->ISR & I2C_ISR_ADDCODE_Msk) >> I2C_ISR_ADDCODE_Pos;
			uint8_t dir = (hi2c->Instance->ISR & I2C_ISR_DIR_Msk) >> I2C_ISR_DIR_Pos;

			if (activeSlave) {
				activeSlave->event_stop(addr, dir);
			}

			__HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF);
		}
		else if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_RXNE)) {
			uint8_t val = (uint8_t)hi2c->Instance->RXDR;
			bool ack = false;
			if (activeSlave) {
				ack |= activeSlave->event_rx(val);
			}
			if (!ack)
				hi2c->Instance->CR2 |= I2C_CR2_NACK;
			setNBYTES(1);
		}
		else if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_TXIS)) {
			uint8_t val = 0;
			if (activeSlave) {
				val = activeSlave->event_tx();
			}
			hi2c->Instance->TXDR = val;
		}
	}

	void error_isr() {

	}

private:
	I2C_HandleTypeDef *hi2c;
	I2C_slave_base *slavesBase;
	I2C_slave_base *activeSlave;

	void setNBYTES(uint8_t b) {
		hi2c->Instance->CR2 = (hi2c->Instance->CR2 & ~I2C_CR2_NBYTES_Msk) | (b << I2C_CR2_NBYTES_Pos);
	}
};

#endif //#defined(HAL_I2C_MODULE_ENABLED) && defined(I2C_OA2_MASK07)
