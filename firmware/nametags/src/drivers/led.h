#ifndef __LED_H__
#define __LED_H__
#include <zephyr.h>

void init_led();
void write_led(u32_t value);
void breathe_led(u32_t interval_ms);

#endif // __LED_H__