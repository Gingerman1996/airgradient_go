# LP5036 LED Driver

Driver for the TI LP5036 36-channel RGB LED controller (I2C).

## I2C Address

- 0x30 to 0x33 based on ADDR1/ADDR0 pins
- 0x1C broadcast address

## Channel Mapping

The device exposes 36 outputs (OUT0..OUT35). RGB LED modules are mapped as:

- LED0 = OUT0 (R), OUT1 (G), OUT2 (B)
- LED1 = OUT3 (R), OUT4 (G), OUT5 (B)
- ...
- LED11 = OUT33 (R), OUT34 (G), OUT35 (B)

## Basic Usage

```cpp
#include "lp5036.h"

// i2c_bus is an existing i2c_master_bus_handle_t
// created by your application.

drivers::LP5036 led(i2c_bus, drivers::LP5036_I2C::ADDR_BASE);
if (led.init() == ESP_OK) {
  led.set_log_scale(false);
  led.set_led_brightness(0, 0xFF);
  led.set_led_color(0, 0xFF, 0x00, 0x00); // LED0 = red
}
```

## Example

See `components/lp5036/example/lp5036_example.cpp`.
