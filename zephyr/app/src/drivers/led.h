#ifndef __LED_H__
#define __LED_H__
#include <zephyr.h>

void init_led();
void write_led(u32_t value);

#endif // __LED_H__