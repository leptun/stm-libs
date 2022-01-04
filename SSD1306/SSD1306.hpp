#pragma once
#include <inttypes.h>
#include <stddef.h>
#include "SSD1306_fonts.hpp"
#include "SSD1306_util.hpp"
#include "stm++/IO.hpp"
#include "stm++/rtos_wrappers.hpp"

class SSD1306_base {
public:
	SSD1306_base(uint8_t screen_width, uint8_t screen_height, base_Wrapper *interface, IO RES = {0}, IO CS = {0}, IO DC = {0}) : box(SSD1306_util::Vertex(0,0), SSD1306_util::Vertex(screen_width - 1, screen_height - 1)), screen_width(screen_width), screen_height(screen_height), interface(interface), RES(RES), CS(CS), DC(DC) {};

	void init();
	void Reset();
	void Fill(SSD1306_util::Color color);
	void UpdateScreen(void);
	void DrawPixel(SSD1306_util::Vertex v, SSD1306_util::Color color);
	bool WriteChar(char ch, SSD1306_fonts::FontDef Font, SSD1306_util::Color color = SSD1306_util::Color::White);
	bool WriteIcon(const SSD1306_util::Icon &icon, SSD1306_util::Color fg, SSD1306_util::Color bg);
	const char *WriteString(const char* str, SSD1306_fonts::FontDef Font, SSD1306_util::Color color = SSD1306_util::Color::White);
	void SetCursor(SSD1306_util::Vertex v);
	void Line(SSD1306_util::Vertex v1, SSD1306_util::Vertex v2, SSD1306_util::Color color);
	void DrawRectangle(SSD1306_util::Vertex v1, SSD1306_util::Vertex v2, SSD1306_util::Color color);
	void FillRectangle(SSD1306_util::Vertex v1, SSD1306_util::Vertex v2, SSD1306_util::Color color);
	void SetContrast(const uint8_t value);
	void SetDisplayOn(const bool on);
	inline bool GetDisplayOn() const {
	    return DisplayOn;
	}

	virtual void WriteCommand(uint8_t byte) {};
	virtual void WriteData(uint8_t* buffer, size_t buff_size) {};

	SSD1306_util::BoundingBox box;
	SSD1306_util::Vertex curpos = SSD1306_util::Vertex(0, 0);

protected:
	uint16_t screen_width;
	uint16_t screen_height;
	base_Wrapper *interface;
	IO RES;
	IO CS;
	IO DC;

private:
	uint8_t *Buffer = nullptr;
    bool Initialized;
    bool DisplayOn;

	bool _FillBuffer(uint8_t* buf, uint32_t len);
	void _init();
};

#if defined(HAL_I2C_MODULE_ENABLED) && USE_HAL_I2C_REGISTER_CALLBACKS
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
#endif //defined(HAL_I2C_MODULE_ENABLED) && USE_HAL_I2C_REGISTER_CALLBACKS

inline SSD1306_util::Color operator~(SSD1306_util::Color color) {
	return static_cast<SSD1306_util::Color>(-static_cast<int8_t>(color));
}
