#include "display.h"

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(display);

#include <display/cfb.h>

#define DISPLAY_DRIVER		"DISPLAY"
static struct device *display;

void init_display()
{
    display = device_get_binding(DISPLAY_DRIVER);
	if (display == NULL) {
		LOG_ERR("Display device not found");
		return;
	}

	if (display_set_pixel_format(display, PIXEL_FORMAT_MONO10) != 0) {
		LOG_ERR("Failed to set required pixel format");
		return;
	}

	if (cfb_framebuffer_init(display)) {
		LOG_ERR("Framebuffer initialization failed!");
		return;
	}

	cfb_framebuffer_clear(display, true);

	display_blanking_off(display);

	u16_t rows;
	u8_t ppt;
	u8_t font_width;
	u8_t font_height;
	rows = cfb_get_display_parameter(display, CFB_DISPLAY_ROWS);
	ppt = cfb_get_display_parameter(display, CFB_DISPLAY_PPT);
	for (int idx = 0; idx < 42; idx++) {
		if (cfb_get_font_size(display, idx, &font_width, &font_height)) {
			break;
		}
		cfb_framebuffer_set_font(display, idx);
		LOG_INF("font width %d, font height %d\n",
		       font_width, font_height);
	}

}

void print_line_to_display(u32_t line_idx, char *str)
{
    cfb_print(display, str, 0, line_idx * 16);
}

void clear_display()
{
    cfb_framebuffer_clear(display, true);
}

void flush_display()
{
    cfb_framebuffer_finalize(display);
}


