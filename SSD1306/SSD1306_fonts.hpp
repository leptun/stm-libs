#pragma once

#include <stdint.h>
#include "SSD1306_util.hpp"

namespace SSD1306_fonts {

struct FontDef {
	const uint8_t FontWidth; // Font width in pixels
	const uint8_t FontHeight; // Font height in pixels
	const uint8_t *data; // Pointer to font icon array
	const uint16_t *offsets; // Pointer to font offsets for each char
};

inline SSD1306_util::Vertex centerFont(FontDef font, SSD1306_util::BoundingBox box) {
	return SSD1306_util::Vertex(box.min.x, (box.max.y + box.min.y - font.FontHeight) / 2);
}


extern FontDef Calibri12x12;

}
