#pragma once
#include "I2C_slave/I2C_slave.hpp"
#include <string.h>
#include "circle_buffer.hpp"

#if defined(HAL_I2C_MODULE_ENABLED)

class LIDAR07_100W_slave : public I2C_slave_base {
public:
	virtual void init() {
		static const osThreadAttr_t LIDAR07_attributes = {
		  .name = "LIDAR07",
		  .stack_size = 256 * 4,
		  .priority = (osPriority_t) osPriorityNormal,
		};
		thread = osThreadNew(task, this, &LIDAR07_attributes);
	}

	virtual bool event_address_match(uint8_t addr, bool dir) {
		if (addr != 0x70)
			return false;
		cmdBufIdx = 0;
		return true;
	}

	virtual void event_repeated_start(uint8_t addr, bool dir) {
		event_stop(addr, dir);
	}

	virtual void event_stop(uint8_t addr, bool dir) {
		if (!dir) { //slave receiver
			processCommand();
		}
		else { //slave transmitter
			if (!__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_TXE) && !txDummy) {
				responseBuf.RewindFirst();
				HAL_GPIO_WritePin(TOF_INT_GPIO_Port, TOF_INT_Pin, GPIO_PIN_SET);
			}
		}

	}

	virtual bool event_rx(uint8_t val) {
		if (cmdBufIdx >= sizeof(cmdBuf))
			return false;

		cmdBuf[cmdBufIdx++] = val;
		return true;
	}

	virtual uint8_t event_tx() {
		uint8_t val = 0;
		if (responseBuf.ConsumeFirst(val)) {
			txDummy = false;
		}
		else {
			txDummy = true;
			if (responseBuf.IsEmpty()) {
				HAL_GPIO_WritePin(TOF_INT_GPIO_Port, TOF_INT_Pin, GPIO_PIN_RESET);
			}
		}
		return val;
	}

	static void task(void *argument) {
		LIDAR07_100W_slave *aista = (LIDAR07_100W_slave *)argument;
		for (;;) {
			if (aista->measurement_active) {
				if (!aista->rangingMode) {
					aista->measurement_active = false;
				}
				uint8_t response[24] = {0xFA, (uint8_t)Commands::MeasurementStartStop | 0x80};
				uint16_t responseLen = 16;

				uint16_t distance = 420;
				response[4] = distance & 0xFF;
				response[5] = (distance >> 8) & 0xFF;


				taskENTER_CRITICAL();
				aista->pushResponse(response, responseLen);
				taskEXIT_CRITICAL();
			}
			osDelay(aista->framerate);
		}
	}

private:
	osThreadId_t thread;

	uint8_t cmdBuf[9];
	uint8_t cmdBufIdx;
	CircleBuffer<uint8_t, 64> responseBuf;
	bool txDummy;

	bool rangingMode;
	bool filtering;
	uint32_t framerate = 10;
	bool measurement_active;

	enum class Commands : uint8_t {
		ReadVersion = 0x43,
		FilteringControl = 0x59,
		MeasurementStartStop = 0x60,
		RangingModeControl = 0x61,
		FrameRateSet = 0x62,
		ReadSystemErrorStatus = 0x65,
	};

	void processCommand() {
		if (cmdBufIdx < sizeof(cmdBuf))
			return; //command too short

		uint8_t cmd = cmdBuf[0];
		uint32_t arg = cmdBuf[1] | ((uint32_t)cmdBuf[2] << 8) | ((uint32_t)cmdBuf[3] << 16) | ((uint32_t)cmdBuf[4] << 24);
		uint32_t crc = cmdBuf[5] | ((uint32_t)cmdBuf[6] << 8) | ((uint32_t)cmdBuf[7] << 16) | ((uint32_t)cmdBuf[8] << 24);

		bool flag_write = cmd & 0x80;
		cmd &= 0x7F;

		LL_CRC_ResetCRCCalculationUnit(CRC);
		for (uint8_t i = 0; i < sizeof(cmdBuf) - 4; i++) {
			LL_CRC_FeedData8(CRC, cmdBuf[i]);
		}
		uint32_t computedCRC = LL_CRC_ReadData32(CRC);
		if (crc != computedCRC)
			return; //crc error

		uint8_t response[12] = {0xFA, cmdBuf[0]};
		uint16_t responseLen = 0;
		switch ((Commands)cmd) {
		case Commands::ReadVersion: {
			if (arg != 0 || flag_write)
				return;
			responseLen = 4;
			response[4] = 0x07;
			response[5] = 0x01;
			response[6] = 0x02;
			response[7] = 0x00;
		} break;
		case Commands::RangingModeControl: {
			if (arg & ~0x01ul)
				return;
			responseLen = 4;
			if (flag_write) {
				rangingMode = arg;
			}
			else {
				response[4] = rangingMode;
			}
		} break;
		case Commands::FilteringControl: {
			if (arg & ~0x01ul)
				return;
			responseLen = 4;
			if (flag_write) {
				filtering = arg;
			}
			else {
				response[4] = filtering;
			}
		} break;
		case Commands::FrameRateSet: {
			responseLen = 4;
			if (flag_write) {
				framerate = arg;
			}
			else {
				response[4] = framerate & 0xFF;
				response[5] = (framerate >> 8) & 0xFF;
				response[6] = (framerate >> 16) & 0xFF;
				response[7] = (framerate >> 24) & 0xFF;
			}
		} break;
		case Commands::MeasurementStartStop: {
			if (!flag_write)
				return;

			if (arg == 2) {
				measurement_active = false;
			}
			else if (arg == 1) {
				measurement_active = true;
			}
			return;
		} break;
		case Commands::ReadSystemErrorStatus: {
			responseLen = 4;
			if (flag_write) { // software reset
				if (arg != 0x55aa55aa) {
					return;
				}
				measurement_active = false;
				framerate = 10;
				rangingMode = false;
				filtering = false;
				txDummy = false;
				responseBuf.Flush();
			}
			else {
			}
		} break;
		default:
			return;
		}

		pushResponse(response, responseLen);
	}

	void pushResponse(uint8_t *response, uint16_t responseLen) {
		response[2] = responseLen & 0xFF;
		response[3] = (responseLen >> 8) & 0xFF;

		LL_CRC_ResetCRCCalculationUnit(CRC);
		for (uint16_t i = 0; i < responseLen + 4; i++) {
			LL_CRC_FeedData8(CRC, response[i]);
		}
		uint32_t newCRC = LL_CRC_ReadData32(CRC);
		response[responseLen + 4] = newCRC & 0xFF;
		response[responseLen + 4 + 1] = (newCRC >> 8) & 0xFF;
		response[responseLen + 4 + 2] = (newCRC >> 16) & 0xFF;
		response[responseLen + 4 + 3] = (newCRC >> 24) & 0xFF;

		for (uint16_t i = 1; i < responseLen + 8; i++) {
			responseBuf.push_back_DontRewrite(response[i]);
		}

		HAL_GPIO_WritePin(TOF_INT_GPIO_Port, TOF_INT_Pin, GPIO_PIN_SET);
	}
};

#endif //#defined(HAL_I2C_MODULE_ENABLED)
