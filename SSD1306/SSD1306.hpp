#pragma once
#include <inttypes.h>
#include <stddef.h>
#include "SSD1306_fonts.hpp"
#include "stm++/IO.hpp"
#include "stm++/rtos_wrappers.hpp"

class SSD1306_base {
public:
	struct Vertex {
		Vertex(uint16_t x, uint16_t y) : x(x), y(y) {};
		uint16_t x;
		uint16_t y;
	};

	struct BoundingBox {
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

	SSD1306_base(uint8_t screen_width, uint8_t screen_height, base_Wrapper *interface, IO RES = {0}, IO CS = {0}, IO DC = {0}) : screen_width(screen_width), screen_height(screen_height), interface(interface), RES(RES), CS(CS), DC(DC) {};

	void init();
	void Reset();
	void Fill(Color color);
	void UpdateScreen(void);
	void DrawPixel(Vertex v, Color color);
	bool WriteChar(char ch, SSD1306_fonts::FontDef Font, Color color = Color::White);
	bool WriteIcon(const Icon &icon, Color fg, Color bg);
	const char *WriteString(const char* str, SSD1306_fonts::FontDef Font, Color color = Color::White);
	void SetCursor(Vertex v);
	void Line(Vertex v1, Vertex v2, Color color);
	void DrawRectangle(Vertex v1, Vertex v2, Color color);
	void FillRectangle(Vertex v1, Vertex v2, Color color);
	void SetContrast(const uint8_t value);
	void SetDisplayOn(const bool on);
	inline bool GetDisplayOn() const {
	    return DisplayOn;
	}

	virtual void WriteCommand(uint8_t byte) {};
	virtual void WriteData(uint8_t* buffer, size_t buff_size) {};

protected:
	uint16_t screen_width;
	uint16_t screen_height;
	base_Wrapper *interface;
	IO RES;
	IO CS;
	IO DC;

private:
	uint8_t *Buffer = nullptr;
	Vertex curpos = Vertex(0, 0);
    bool Initialized;
    bool DisplayOn;

	bool _FillBuffer(uint8_t* buf, uint32_t len);
	void _init();
};

#ifdef HAL_HAS_I2C
class SSD1306_i2c : public SSD1306_base {
public:
	SSD1306_i2c(uint8_t screen_width, uint8_t screen_height, I2C_Wrapper *i2c, uint8_t address, IO RES = {0}) :
		SSD1306_base(screen_width, screen_height, i2c, RES),
		i2cAddress(address) {};
	void WriteCommand(uint8_t byte) override;
	void WriteData(uint8_t* buffer, size_t buff_size) override;

private:
	uint8_t i2cAddress;
};
#endif

inline SSD1306_base::Color operator~(SSD1306_base::Color color) {
	return static_cast<SSD1306_base::Color>(-static_cast<int8_t>(color));
}
