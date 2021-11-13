#pragma once

#include <stdint.h>

namespace SSD1306_fonts {

struct FontDef {
	const uint8_t FontWidth; // Font width in pixels
	const uint8_t FontHeight; // Font height in pixels
	const uint8_t *data; // Pointer to font icon array
	const uint16_t *offsets; // Pointer to font offsets for each char
};


extern FontDef Calibri12x12;

}
