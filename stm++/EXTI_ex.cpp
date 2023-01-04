#include "EXTI_ex.hpp"

ObjectCallback EXTI_ex_registry[16];

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	ObjectCallback *obj = &EXTI_ex_registry[POSITION_VAL(GPIO_Pin)];
	if (obj->funcWrapper) {
		obj->funcWrapper(obj->arg);
	}
}

void EXTI_ex_RegisterCallback(const IO &io, const ObjectCallback &cb) {
	if (io.port && IS_GPIO_PIN(io.pin)) {
		EXTI_ex_registry[POSITION_VAL(io.pin)] = cb;
	}
}
