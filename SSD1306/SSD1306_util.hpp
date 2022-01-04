#pragma once

#include <stdint.h>

namespace SSD1306_util {
	struct Vertex {
		Vertex() :x(0), y(0) {};
		Vertex(uint16_t x, uint16_t y) : x(x), y(y) {};
		uint16_t x;
		uint16_t y;
	};

	struct BoundingBox {
		BoundingBox() {};
		BoundingBox(Vertex min, Vertex max) : min(min), max(max) {};
		Vertex min;
		Vertex max;
	};

	struct Icon {
		const uint8_t Width;
		const uint8_t Height;
		const uint8_t data[0];
	};

	enum class Color {
		Black = -1,
		None = 0, //no change
		White = 1,
	};
};
