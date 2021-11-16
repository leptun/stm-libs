#include "./tmc5160.hpp"
#include <cstdio>
#include <math.h>
#include <algorithm>

int TMC5160::_TMC5160_cnt = 0;

HAL_StatusTypeDef TMC5160::init()
{
	spi->Init();

	if (!lock) {
		snprintf(name, sizeof(name), "TMC5160_%i", _TMC5160_cnt++);
		const osMutexAttr_t attrM = {
			.name = name,
		};
		lock = osMutexNew(&attrM);
	}

	uint32_t IOIN = readRegister(Registers::IOIN);
	if (((IOIN >> 24) & 0xFF) != 0x30 || (IOIN & (1 << 6))) // check tmc version
		return HAL_ERROR;
	if (spi_status & 0x01) //clear reset flag
		writeRegister(Registers::GSTAT, 0x01);
	ready = true;
	return HAL_OK;
}

uint32_t TMC5160::readRegister(Registers reg)
{
	uint8_t pData[5] = {(uint8_t)reg};

	if (osMutexAcquire(lock, 1000) != osOK)
		Error_Handler();
	_spi_tx_rx_dma(pData);
	pData[0] = 0;
	_spi_tx_rx_dma(pData);
	osMutexRelease(lock);

	_handleStatus(pData[0]);
	return ((uint32_t)pData[1] << 24 | (uint32_t)pData[2] << 16 | (uint32_t)pData[3] << 8 | (uint32_t)pData[4]);
}

void TMC5160::writeRegister(Registers reg, uint32_t data)
{
	uint8_t pData[5] = {(uint8_t)((uint8_t)(reg) | 0x80), (uint8_t)(data >> 24), (uint8_t)(data >> 16), (uint8_t)(data >> 8), (uint8_t)data};

	if (osMutexAcquire(lock, 1000) != osOK)
		Error_Handler();
	_spi_tx_rx_dma(pData);
	osMutexRelease(lock);

	_handleStatus(pData[0]);
}

void TMC5160::_spi_tx_rx_dma(uint8_t *pData)
{
	if (osMutexAcquire(spi->lock, 1000) != osOK)
		Error_Handler();
	HAL_SPI_RegisterCallback((SPI_HandleTypeDef*)spi->resource, HAL_SPI_MSPINIT_CB_ID, (pSPI_CallbackTypeDef)this);
	HAL_SPI_RegisterCallback((SPI_HandleTypeDef*)spi->resource, HAL_SPI_TX_RX_COMPLETE_CB_ID, _spi_dma_complete);
	HAL_GPIO_WritePin(csn.port, csn.pin, GPIO_PIN_RESET);
	if (HAL_SPI_TransmitReceive_DMA((SPI_HandleTypeDef*)spi->resource, pData, pData, 5) != HAL_OK)
		Error_Handler();
	if (osEventFlagsWait(spi->flags, 0x01, osFlagsWaitAny, 1000) != 0x01) {
		HAL_GPIO_WritePin(csn.port, csn.pin, GPIO_PIN_SET);
		Error_Handler();
	}
	osMutexRelease(spi->lock);
}

void TMC5160::_spi_dma_complete(SPI_HandleTypeDef *hspi)
{
	TMC5160* aista = (TMC5160*)((void*)(hspi->MspInitCallback));
	HAL_GPIO_WritePin(aista->csn.port, aista->csn.pin, GPIO_PIN_SET);
	osEventFlagsSet(aista->spi->flags, 0x01);
}

void TMC5160::_handleStatus(uint8_t status)
{
	spi_status = status;
}

void TMC5160::setSpreadCycleConfig(SpreadCycleConfig config)
{
	uint32_t chopconf = readRegister(Registers::CHOPCONF);
	chopconf &= ~0x00F187FF;
	chopconf |=
		((uint32_t)(config.toff) << 0) |
		((uint32_t)(config.hstrt) << 4) |
		((uint32_t)(config.hend) << 7) |
		((uint32_t)(config.tbl) << 15) |
		((uint32_t)(config.tpfd) << 20);
	writeRegister(Registers::CHOPCONF, chopconf);
}

void TMC5160::setMicrosteppingConfig(MicrosteppingConfig config)
{
	uint32_t chopconf = readRegister(Registers::CHOPCONF);
	chopconf &= ~0x1F000000;
	chopconf |=
		((uint32_t)(config.mres) << 24) |
		((uint32_t)(config.intpol) << 28);
	writeRegister(Registers::CHOPCONF, chopconf);
}

void TMC5160::setCoolstepConfig(CoolstepConfig config)
{
	writeRegister(Registers::TCOOLTHRS, config.tcoolthrs);
	writeRegister(Registers::THIGH, config.thigh);
	uint32_t coolconf =
		((uint32_t)(config.semin) << 0) |
		((uint32_t)(config.seup) << 5) |
		((uint32_t)(config.semax) << 8) |
		((uint32_t)(config.sedn) << 13) |
		((uint32_t)(config.seimin) << 15) |
		(((uint32_t)(config.sgt) & 0x7F) << 16) |
		((uint32_t)(config.sfilt) << 24);
	writeRegister(Registers::COOLCONF, coolconf);
}

uint8_t TMC5160::_calc_globalscaler(float current)
{
	unsigned int globalscaler = (current * 256. * sqrt(2) * SENSE_RESISTOR / VREF) + 0.5;
	if (globalscaler < 32)
		globalscaler = 32;
	else if (globalscaler >= 256)
		globalscaler = 0;
	return globalscaler;
}

uint8_t TMC5160::_calc_current_bits(float current, unsigned int globalscaler)
{
	if (globalscaler == 0)
		globalscaler = 256;
	uint8_t cs = (current * 256. * 32. * sqrt(2) * SENSE_RESISTOR) / (globalscaler * VREF) - 0.5;
	if (cs > 31)
		cs = 31;
	return cs;
}

void TMC5160::setCurrents(CurrentConfig config)
{
	if (config.hold_current > config.run_current)
		config.hold_current = config.run_current;
	uint8_t globalscaler = _calc_globalscaler(config.run_current);
	uint8_t ihold = _calc_current_bits(config.hold_current, globalscaler);
	uint8_t irun = _calc_current_bits(config.run_current, globalscaler);
	uint32_t ihold_irun =
			((uint32_t)(ihold & 0x1F) << 0) |
			((uint32_t)(irun & 0x1F) << 8) |
			((uint32_t)(config.iholddelay) << 16);
	writeRegister(Registers::GLOBALSCALER, globalscaler);
	writeRegister(Registers::IHOLD_IRUN, ihold_irun);
}

void TMC5160::enable()
{
	HAL_GPIO_WritePin(enn.port, enn.pin, GPIO_PIN_RESET);
}

void TMC5160::disable()
{
	HAL_GPIO_WritePin(enn.port, enn.pin, GPIO_PIN_SET);
}

void TMC5160::setInversion(bool inverted)
{
	uint32_t reg = readRegister(Registers::GCONF);
	writeRegister(TMC5160::Registers::GCONF, reg | (1 << 4));
}

uint16_t TMC5160::getSG_RESULT()
{
	uint32_t reg = readRegister(Registers::DRV_STATUS);
	return reg & 0x3FF;
}

uint8_t TMC5160::getCS_ACTUAL()
{
	uint32_t reg = readRegister(Registers::DRV_STATUS);
	return (reg >> 16) & 0x1F;
}

uint32_t TMC5160::getTSTEP()
{
	uint32_t reg = readRegister(Registers::TSTEP);
	return reg & 0xFFFFF;
}

void TMC5160::Motion::setProfileConfig(ProfileConfig config)
{
	_tmc.writeRegister(Registers::VSTART, config.vstart);
	_tmc.writeRegister(Registers::VSTOP, config.vstop);
	_tmc.writeRegister(Registers::TZEROWAIT, config.tzerowait);
	_tmc.writeRegister(Registers::A1, config.a1);
	_tmc.writeRegister(Registers::V1, config.v1);
	_vmax = config.vmax;
	_tmc.writeRegister(Registers::AMAX, config.amax);
	_tmc.writeRegister(Registers::DMAX, config.dmax);
	_tmc.writeRegister(Registers::D1, config.d1);
}

void TMC5160::Motion::setMode(RampModes mode)
{
	_tmc.writeRegister(Registers::RAMPMODE, (uint32_t)mode);
}

void TMC5160::Motion::setVelocity(uint32_t vmax)
{
	vmax = std::min(vmax, _vmax);
	_tmc.writeRegister(Registers::VMAX, vmax);
}

void TMC5160::Motion::resetPosition(int32_t pos)
{
	_target_position = pos;
	_tmc.writeRegister(Registers::XACTUAL, (uint32_t)(pos));
	_tmc.writeRegister(Registers::XTARGET, (uint32_t)(pos));
}

void TMC5160::Motion::setTargetPosition(int32_t pos)
{
	_target_position = pos;
	_tmc.writeRegister(Registers::XTARGET, (uint32_t)(pos));
}

void TMC5160::Motion::setTargetPositionRelative(int32_t pos)
{
	_tmc.writeRegister(Registers::XTARGET, _target_position + pos);
}

void TMC5160::Motion::resetTargetToActual()
{
	setTargetPosition(_tmc.readRegister(Registers::XACTUAL));
}

int32_t TMC5160::Motion::getLatchedPosition()
{
	return _tmc.readRegister(Registers::XLATCH);
}

bool TMC5160::Motion::targetPositionReached()
{
	return (_tmc.readRegister(Registers::RAMP_STAT) & (1 << 9));
}

bool TMC5160::Motion::targetVelocityReached()
{
	return (_tmc.readRegister(Registers::RAMP_STAT) & (1 << 8));
}

void TMC5160::Motion::waitForTargetPosition()
{
	while (!targetPositionReached())
		osDelay(1);
}

void TMC5160::Motion::waitForTargetVelocity()
{
	while (!targetVelocityReached())
		osDelay(1);
}

void TMC5160::Motion::softStop()
{
	setVelocity(0);
	setMode(RampModes::velocity_positive);
	waitForTargetVelocity();
	setMode(RampModes::hodl);
}

void TMC5160::Endstops::setEndstopConfig(EndstopConfig config)
{
	_sw_mode &= ~0x1C;
	_sw_mode |= config.pol_stop_l << 2;
	_sw_mode |= config.pol_stop_r << 3;
	_sw_mode |= config.swap_lr << 4;
	_pushConfig();
}

void TMC5160::Endstops::setSoftstop(bool en_softstop)
{
	_sw_mode &= ~(1 << 11);
	_sw_mode |= en_softstop << 11;
	_pushConfig();
}

void TMC5160::Endstops::setLimitingConfig(LimitingConfig config)
{
	_sw_mode &= ~0x03;
	_sw_mode |= (config.stop_l_enable << 0);
	_sw_mode |= (config.stop_r_enable << 1);
	_pushConfig();
}

void TMC5160::Endstops::setLatchConfig(LatchConfig config)
{
	_sw_mode &= ~0x1E0;
	_sw_mode |= config.latch_l_active << 5;
	_sw_mode |= config.latch_l_inactive << 6;
	_sw_mode |= config.latch_r_active << 7;
	_sw_mode |= config.latch_r_inactive << 8;
	_pushConfig();
}

void TMC5160::Endstops::clearLatchEvents()
{
	_tmc.writeRegister(Registers::RAMP_STAT, 0x0C);
}

TMC5160::Endstops::SwitchStatus TMC5160::Endstops::getSwitchStatus()
{
	uint32_t reg = _tmc.readRegister(Registers::RAMP_STAT);
	SwitchStatus status = {
		.status_stop_l = (bool)(reg & (1 << 0)),
		.status_stop_r = (bool)(reg & (1 << 1)),
		.status_latch_l = (bool)(reg & (1 << 2)),
		.status_latch_r = (bool)(reg & (1 << 3)),
		.event_stop_l = (bool)(reg & (1 << 4)),
		.event_stop_r = (bool)(reg & (1 << 5)),
	};
	return status;
}
