#include "./SSD1306.hpp"
#include <cstring>
#include <algorithm>

void SSD1306_base::_init()
{
	Buffer = (uint8_t*)pvPortMalloc((screen_width * screen_height) / 8);

	// Reset OLED
	Reset();

	// Init OLED
	SetDisplayOn(false); //display off

	WriteCommand(0x20); //Set Memory Addressing Mode
	WriteCommand(0x00); // 00b,Horizontal Addressing Mode; 01b,Vertical Addressing Mode;
						// 10b,Page Addressing Mode (RESET); 11b,Invalid

	WriteCommand(0xB0); //Set Page Start Address for Page Addressing Mode,0-7

	#ifdef SSD1306_MIRROR_VERT
	WriteCommand(0xC0); // Mirror vertically
	#else
	WriteCommand(0xC8); //Set COM Output Scan Direction
	#endif

	WriteCommand(0x00); //---set low column address
	WriteCommand(0x10); //---set high column address

	WriteCommand(0x40); //--set start line address - CHECK

	SetContrast(0xFF);

	#ifdef SSD1306_MIRROR_HORIZ
	WriteCommand(0xA0); // Mirror horizontally
	#else
	WriteCommand(0xA1); //--set segment re-map 0 to 127 - CHECK
	#endif

	#ifdef SSD1306_INVERSE_COLOR
	WriteCommand(0xA7); //--set inverse color
	#else
	WriteCommand(0xA6); //--set normal color
	#endif

	// Set multiplex ratio.
	if (screen_height == 128)
		WriteCommand(0xFF); // Found in the Luma Python lib for SH1106.
	else
		WriteCommand(0xA8); //--set multiplex ratio(1 to 64) - CHECK

	if (screen_height == 32)
		WriteCommand(0x1F); //
	else if (screen_height == 64)
		WriteCommand(0x3F); //
	else if (screen_height == 128)
		WriteCommand(0x3F); // Seems to work for 128px high displays too.
	else
		Error_Handler();

	WriteCommand(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content

	WriteCommand(0xD3); //-set display offset - CHECK
	WriteCommand(0x00); //-not offset

	WriteCommand(0xD5); //--set display clock divide ratio/oscillator frequency
	WriteCommand(0xF0); //--set divide ratio

	WriteCommand(0xD9); //--set pre-charge period
	WriteCommand(0x22); //

	WriteCommand(0xDA); //--set com pins hardware configuration - CHECK

	if (screen_height == 32)
		WriteCommand(0x02);
	else if (screen_height == 64)
		WriteCommand(0x12);
	else if (screen_height == 128)
		WriteCommand(0x12);
	else
		Error_Handler();

	WriteCommand(0xDB); //--set vcomh
	WriteCommand(0x20); //0x20,0.77xVcc

	WriteCommand(0x8D); //--set DC-DC enable
	WriteCommand(0x14); //
	SetDisplayOn(true); //--turn on SSD1306 panel

	// Clear screen
	Fill(SSD1306_util::Color::Black);

	// Flush buffer to screen
	UpdateScreen();

	// Set default values for screen object
	curpos = SSD1306_util::Vertex(0, 0);

	Initialized = 1;
}

void SSD1306_base::Reset()
{
	// CS = High (not selected)
	if (CS.port)
		HAL_GPIO_WritePin(CS.port, CS.pin, GPIO_PIN_SET);

    // Reset the OLED
	if (RES.port) {
		HAL_GPIO_WritePin(RES.port, RES.pin, GPIO_PIN_RESET);
		osDelay(10);
		HAL_GPIO_WritePin(RES.port, RES.pin, GPIO_PIN_SET);
	}
	osDelay(100); // Wait for the screen to boot
}

bool SSD1306_base::_FillBuffer(uint8_t* buf, uint32_t len)
{
	bool ret = true;
	if (len <= ((screen_width * screen_height) / 8)) {
		memcpy(Buffer,buf,len);
		ret = false;
	}
	return ret;
}

void SSD1306_base::Fill(SSD1306_util::Color color)
{
    for(uint32_t i = 0; i < ((screen_width * screen_height) / 8); i++) {
        Buffer[i] = (color == SSD1306_util::Color::White) ? 0xFF : 0x00;
    }
}

void SSD1306_base::UpdateScreen(void)
{
    // Write data to each page of RAM. Number of pages
    // depends on the screen height:
    //
    //  * 32px   ==  4 pages
    //  * 64px   ==  8 pages
    //  * 128px  ==  16 pages
    for(uint8_t i = 0; i < screen_height/8; i++) {
        WriteCommand(0xB0 + i); // Set the current RAM page address.
        WriteCommand(0x00);
        WriteCommand(0x10);
        WriteData(Buffer + (screen_width * i), screen_width);
    }
}

void SSD1306_base::DrawPixel(SSD1306_util::Vertex v, SSD1306_util::Color color)
{
    if(v.x >= screen_width || v.y >= screen_height) {
        // Don't write outside the buffer
        return;
    }

    // Draw in the right color
    switch (color) {
    case SSD1306_util::Color::White:
    	Buffer[v.x + (v.y / 8) * screen_width] |= 1 << (v.y % 8);
    	break;
    case SSD1306_util::Color::Black:
    	Buffer[v.x + (v.y / 8) * screen_width] &= ~(1 << (v.y % 8));
    	break;
    default:
    	break;
    }
}

bool SSD1306_base::WriteChar(char ch, SSD1306_fonts::FontDef Font, SSD1306_util::Color color)
{
	if (ch < 32 || ch > 126)
		return true;

	bool ret = WriteIcon(*(const SSD1306_util::Icon*)(Font.data + Font.offsets[ch - 32]), color, ~color);
	if (!ret) {
		for (uint16_t i = curpos.y; i < curpos.y + Font.FontHeight; i++) {
			DrawPixel(SSD1306_util::Vertex(curpos.x, i), ~color);
		}
	}
	curpos.x++;

	return ret;
}

bool SSD1306_base::WriteIcon(const SSD1306_util::Icon &icon, SSD1306_util::Color fg, SSD1306_util::Color bg)
{
	const uint8_t *basePtr = icon.data;

	if (screen_width < (curpos.x + icon.Width) || screen_height < (curpos.y + icon.Height))
		return true;

	for (uint16_t i = 0; i < icon.Width; i++) {
		uint16_t j = icon.Height;
		while (j > 0) {
			uint8_t c = *basePtr++;
			int8_t pixelsUsed = std::min(j, (uint16_t)8);
			for (uint8_t k = 1; k && j; k <<= 1) {
				if (k == 1 << pixelsUsed) {
					break;
				}
					DrawPixel(SSD1306_util::Vertex(curpos.x + i, curpos.y + (icon.Height - j)), (c & k) ? fg : bg);
				j--;
			}
		}
	}
	curpos.x += icon.Width;

	return false;
}

const char *SSD1306_base::WriteString(const char* str, SSD1306_fonts::FontDef Font, SSD1306_util::Color color)
{
    while (*str) {
        if (WriteChar(*str, Font, color))
            return str;
        str++;
    }
    return NULL;
}

void SSD1306_base::SetCursor(SSD1306_util::Vertex v)
{
	curpos = v;
}

void SSD1306_base::Line(SSD1306_util::Vertex v1, SSD1306_util::Vertex v2, SSD1306_util::Color color)
{
	int32_t deltaX = std::abs(v2.x - v1.x);
	int32_t deltaY = std::abs(v2.y - v1.y);
	int32_t signX = ((v1.x < v2.x) ? 1 : -1);
	int32_t signY = ((v1.y < v2.y) ? 1 : -1);
	int32_t error = deltaX - deltaY;
	int32_t error2;

	DrawPixel(v2, color);
	while((v1.x != v2.x) || (v1.y != v2.y)) {
		DrawPixel(v1, color);
		error2 = error * 2;
		if(error2 > -deltaY) {
			error -= deltaY;
			v1.x += signX;
		} else {
			/*nothing to do*/
		}

		if(error2 < deltaX) {
		  error += deltaX;
		  v1.y += signY;
		} else {
			/*nothing to do*/
		}
	}
}

void SSD1306_base::DrawRectangle(SSD1306_util::Vertex v1, SSD1306_util::Vertex v2, SSD1306_util::Color color)
{
	Line(SSD1306_util::Vertex(v1.x, v1.y), SSD1306_util::Vertex(v2.x, v1.y),color);
	Line(SSD1306_util::Vertex(v2.x, v1.y), SSD1306_util::Vertex(v2.x, v2.y),color);
	Line(SSD1306_util::Vertex(v2.x, v2.y), SSD1306_util::Vertex(v1.x, v2.y),color);
	Line(SSD1306_util::Vertex(v1.x, v2.y), SSD1306_util::Vertex(v1.x, v1.y),color);
}

void SSD1306_base::FillRectangle(SSD1306_util::Vertex v1, SSD1306_util::Vertex v2, SSD1306_util::Color color)
{
	if (v1.x > v2.x || v1.y > v2.y)
		return;
	for (uint8_t i = v1.x; i <= v2.x; i++)
		for (uint8_t j = v1.y; j <= v2.y; j++)
			DrawPixel(SSD1306_util::Vertex(i, j), color);
}

void SSD1306_base::SetContrast(const uint8_t value)
{
    WriteCommand(0x81);
    WriteCommand(value);
}

void SSD1306_base::SetDisplayOn(const bool on)
{
	DisplayOn = on;
    WriteCommand(on ? 0xAF : 0xAE);
}



void SSD1306_base::init()
{
	interface->Init();
	_init();
}

#if defined(HAL_I2C_MODULE_ENABLED) && USE_HAL_I2C_REGISTER_CALLBACKS
void SSD1306_i2c::WriteCommand(uint8_t byte)
{
	HAL_I2C_Mem_Write((I2C_HandleTypeDef*)interface->resource, (i2cAddress << 1), 0x00, I2C_MEMADD_SIZE_8BIT, &byte, 1, HAL_MAX_DELAY);
}

void SSD1306_i2c::WriteData(uint8_t* buffer, size_t buff_size)
{
	HAL_I2C_Mem_Write((I2C_HandleTypeDef*)interface->resource, (i2cAddress << 1), 0x40, I2C_MEMADD_SIZE_8BIT, buffer, buff_size, HAL_MAX_DELAY);
}
#endif //defined(HAL_I2C_MODULE_ENABLED) && USE_HAL_I2C_REGISTER_CALLBACKS
