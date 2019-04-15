
#include "neopixels.h"

#define NEOPIXEL_COUNT 4
#define NEOPIXEL_STRIP_DEV_NAME DT_WORLDSEMI_WS2812_0_LABEL

static struct led_rgb neopixel_colors[NEOPIXEL_COUNT];
struct device *neopixel_strip;

void init_neopixels()
{
	neopixel_strip = device_get_binding(NEOPIXEL_STRIP_DEV_NAME);
}

void write_neopixel(u32_t neopixel_number, struct led_rgb value)
{
	if (neopixel_number < NEOPIXEL_COUNT)
	{
		neopixel_colors[neopixel_number] = value;
	}
}

void flush_neopixels()
{
	led_strip_update_rgb(neopixel_strip, neopixel_colors, NEOPIXEL_COUNT);
}

void write_neopixels_all(struct led_rgb value, bool flush)
{
	for (u8_t i = 0; i < NEOPIXEL_COUNT; i++)
	{
		neopixel_colors[i] = value;
	}
	if (flush)
	{
		flush_neopixels();
	}
}
