#ifndef __NEOPIXELS_H__
#define __NEOPIXELS_H__
#include <zephyr.h>
#include <led_strip.h>

static const struct led_rgb red = { .r = 0x20, .g = 0x00, .b = 0x00 };
static const struct led_rgb green = { .r = 0x0, .g = 0x20, .b = 0x0 };
static const struct led_rgb blue = { .r = 0x0, .g = 0x0, .b = 0x20 };
static const struct led_rgb purple = { .r = 0x10, .g = 0x0, .b = 0x20 };

void init_neopixels();
void write_neopixel(u32_t neopixel_number, struct led_rgb value);
void update_neopixels();

#endif // __NEOPIXELS_H__