#pragma once

#include "main.h"

struct IO {
	GPIO_TypeDef *port;
	uint16_t pin;
};
