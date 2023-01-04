#pragma once
#include "stm++.hpp"
#include "IO.hpp"

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
extern void EXTI_ex_RegisterCallback(const IO &io, const ObjectCallback &cb);
