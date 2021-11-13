#include "rtos_wrappers.hpp"

void base_Wrapper::Init() {
	if (!lock) {
		const osMutexAttr_t attrM = {
			.name = name,
		};
		lock = osMutexNew(&attrM);

		const osEventFlagsAttr_t attrF = {
			.name = name,
		};
		flags = osEventFlagsNew(&attrF);
	}
}
