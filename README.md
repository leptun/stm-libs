# stm-libs
This is just a collection of STM32 HAL libraries that I use throughout my projects as a submodule. All of them use freeRTOS functions and heavily rely on HAL callbacks.

# Usage
1. Add this repo as a submodule in an existing project (must be C++, a C project can be converted to c++ in stm32cubeide)
2. Edit the stm32cubeMX file so that HAL callbacks, DMA and interrupts are enabled for the peripherals that are used and make sure that freeRTOS is used.
3. Configure the submodule directory to not be excluded from the build in all configurations.
4. Configure the project c++ include paths to include the submodule directory.
5. Parts of the library can now be used in the final code.

# Example code
```
#include "cmsis_os.h"
#include "AT24C/AT24C.hpp"
#include "SSD1306/SSD1306.hpp"
#include "tmc5160/tmc5160.hpp"

SPI_Wrapper tmc_spi(&hspi3, "tmc_spi_bus");
I2C_Wrapper i2c_bus(&hi2c1, "i2c_bus");

AT24C eeprom(0x50, 1024, &i2c_bus);
SSD1306_i2c lcd(128, 64, &i2c_bus, 0x3C, pins::Joystick.reset);
TMC5160 tmc(pins::TMC1.CSN, pins::TMC1.ENN, &tmc_spi)
```
