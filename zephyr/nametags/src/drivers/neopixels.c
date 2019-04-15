
#include "neopixels.h"

#define NEOPIXEL_COUNT 4
#define NEOPIXEL_STRIP_DEV_NAME DT_WORLDSEMI_WS2812_0_LABEL

static struct led_rgb neopixel_colors[NEOPIXEL_COUNT];
struct device *neopixel_strip;

void init_neopixels()
{
	// Neopixels
	neopixel_strip = device_get_binding(NEOPIXEL_STRIP_DEV_NAME);

	neopixel_colors[0] = purple;
	neopixel_colors[1] = purple;
	neopixel_colors[2] = purple;
	neopixel_colors[3] = purple;
	led_strip_update_rgb(neopixel_strip, neopixel_colors, NEOPIXEL_COUNT);
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