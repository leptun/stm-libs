#pragma once
#include "main.h"
#include "cmsis_os.h"

struct ObjectCallback {
	void (*funcWrapper)(void *arg);
	void *arg;
};
