#pragma once

#include "stm++/IO.hpp"
#include "stm++/rtos_wrappers.hpp"

#if defined(HAL_SPI_MODULE_ENABLED) && USE_HAL_SPI_REGISTER_CALLBACKS

class TMC5160 {
public:
	struct CurrentConfig {
		float run_current;
		float hold_current;
		uint8_t iholddelay : 4;
	};

	struct MicrosteppingConfig {
		uint8_t mres : 4;
		uint8_t intpol : 1;
	};

	struct StealthChopConfig {
		uint32_t tpwmthrs : 20;
	};

	struct SpreadCycleConfig {
		uint8_t toff : 4;
		uint8_t hstrt : 3;
		uint8_t hend : 4;
		uint8_t tbl : 2;
		uint8_t tpfd : 4;
	};

	struct HighVelocityConfig {
		uint8_t vhighfs : 1;
		uint8_t vhighchm : 1;
		uint8_t fd3 : 1;
		uint8_t disfdcc : 1;
	};

	struct CoolstepConfig {
		uint32_t tcoolthrs : 20;
		uint32_t thigh : 20;
		int8_t sgt;
		uint8_t semin : 4;
		uint8_t semax : 4;
		uint8_t sedn : 2;
		uint8_t seup : 2;
		bool seimin;
		bool sfilt;
	};

	enum class Registers : uint8_t {
		// General Configuration Registers
		GCONF = 0x00,
		GSTAT = 0x01,
		IOIN = 0x04,

		// Velocity Dependent Driver Feature Control Register Set
		GLOBALSCALER = 0x0B,
		IHOLD_IRUN = 0x10,
		TPOWERDOWN = 0x11,
		TSTEP = 0x12,
		TPWMTHRS = 0x13,
		TCOOLTHRS = 0x14,
		THIGH = 0x15,

		// Ramp Generator Registers
		RAMPMODE = 0x20,
		XACTUAL = 0x21,
		VACTUAL = 0x22,
		VSTART = 0x23,
		A1 = 0x24,
		V1 = 0x25,
		AMAX = 0x26,
		VMAX = 0x27,
		DMAX = 0x28,
		D1 = 0x2A,
		VSTOP = 0x2B,
		TZEROWAIT = 0x2C,
		XTARGET = 0x2D,
		VDCMIN = 0x33,
		SW_MODE = 0x34,
		RAMP_STAT = 0x35,
		XLATCH = 0x36,

		// Encoder Registers
		ENCMODE = 0x38,
		X_ENC = 0x39,
		ENC_CONST = 0x3A,
		ENC_STATUS = 0x3B,
		ENC_LATCH = 0x3C,
		ENC_DEVIATION = 0x3D,

		// Motor Driver Registers
		CHOPCONF = 0x6C,
		COOLCONF = 0x6D,
		DCCTRL = 0x6E,
		DRV_STATUS = 0x6F,
		PWMCONF = 0x70,
	};

	enum class Events : uint32_t {
		DIAG0,
		DIAG1,
		event_pos_reached,
		event_stop_sg,
		event_stop_r,
		event_stop_l,
		deviation_warn,
		N_event,
		PositionCompare,
	};

	TMC5160(IO csn, IO enn, IO diag0, IO diag1, SPI_Wrapper *spi) : motion(*this), endstops(*this), csn(csn), enn(enn), diag0(diag0), diag1(diag1), spi(spi) {};
	HAL_StatusTypeDef init();
	bool Ready() { return ready; };

	uint32_t readRegister(Registers reg);
	void writeRegister(Registers reg, uint32_t data);
	void setSpreadCycleConfig(SpreadCycleConfig config);
	void setMicrosteppingConfig(MicrosteppingConfig config);
	void setCoolstepConfig(CoolstepConfig config);
	void setCurrents(CurrentConfig config);
	void enable();
	void disable();
	void setInversion(bool inverted);
	uint16_t getSG_RESULT();
	uint8_t getCS_ACTUAL();
	uint32_t getTSTEP();

	class Motion {
	public:
		enum class RampModes : uint8_t {
			position = 0,
			velocity_positive = 1,
			velocity_negative = 2,
			hodl = 3,
		};
		struct ProfileConfig {
			uint32_t vstart : 18;
			uint32_t vstop : 18;
			uint32_t tzerowait : 16;
			uint32_t a1 : 16;
			uint32_t v1 : 20;
			uint32_t vmax : 23;
			uint32_t amax : 16;
			uint32_t dmax : 16;
			uint32_t d1 : 16;
//			uint32_t vdcmin : 23;
		};
		Motion(TMC5160 &tmc) : _tmc(tmc) {};
		void setProfileConfig(ProfileConfig config);
		void setMode(RampModes mode);
		void setVelocity(uint32_t vmax);
		void resetPosition(int32_t pos);
		void setTargetPosition(int32_t pos);
		void setTargetPositionRelative(int32_t pos);
		void resetTargetToActual();
		int32_t getLatchedPosition();
		bool targetPositionReached();
		bool targetVelocityReached();
		bool vzero();
		void waitForTargetPosition();
		void waitForTargetVelocity();
		void waitForVzero();
		void softStop();
		inline uint32_t getAbsoluteVMAX() {return _vmax;};
	private:
		TMC5160 &_tmc;
		uint32_t _vmax;
		int32_t _target_position;
	} motion;

	class Endstops {
	public:
		struct EndstopConfig {
			bool pol_stop_l;
			bool pol_stop_r;
			bool swap_lr;
		};

		struct LatchConfig {
			bool latch_l_active;
			bool latch_l_inactive;
			bool latch_r_active;
			bool latch_r_inactive;
		};

		struct SwitchStatus {
			bool status_stop_l;
			bool status_stop_r;
			bool status_latch_l;
			bool status_latch_r;
			bool event_stop_l;
			bool event_stop_r;
		};

		struct LimitingConfig {
			bool stop_l_enable;
			bool stop_r_enable;
		};

		Endstops(TMC5160 &tmc) : _tmc(tmc) {};
		void setEndstopConfig(EndstopConfig config);
		void setSoftstop(bool en_softstop);
		void setLimitingConfig(LimitingConfig config);
		void setLatchConfig(LatchConfig config);
		LatchConfig getLatchConfig();
		void clearLatchEvents();
		SwitchStatus getSwitchStatus();
	private:
		TMC5160 &_tmc;
		uint32_t _sw_mode = 0;
		inline void _pushConfig() {_tmc.writeRegister(Registers::SW_MODE, _sw_mode);};
	} endstops;

	osEventFlagsId_t evFlags;

private:
	static constexpr float VREF = 0.325;
	static constexpr float SENSE_RESISTOR = 0.075;
	IO csn;
	IO enn;
	IO diag0;
	IO diag1;
	SPI_Wrapper *spi;
	osMutexId_t lock;
	static int _TMC5160_cnt;
	char name[configMAX_TASK_NAME_LEN];
	uint8_t spi_status = 0;
	bool ready = false;

	osThreadId_t TMC5160_TaskHandle;
	static void threadWrapper(void *argument);
	void thread();

	void _spi_tx_rx_dma(uint8_t *pData);
	static void _spi_dma_complete(SPI_HandleTypeDef *hspi);
	void _handleStatus(uint8_t status);

	static void diag0_callback(TMC5160 *tmc);
	static void diag1_callback(TMC5160 *tmc);

	uint8_t _calc_globalscaler(float current);
	uint8_t _calc_current_bits(float current, unsigned int globalscaler);
};

#endif //defined(HAL_SPI_MODULE_ENABLED) && USE_HAL_SPI_REGISTER_CALLBACKS
