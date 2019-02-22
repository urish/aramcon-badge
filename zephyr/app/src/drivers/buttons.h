#ifndef __BUTTONS_H__
#define __BUTTONS_H__
#include <zephyr.h>
#define BUTTON_COUNT 3

void init_buttons();
void button_read(u32_t button_number, u32_t *value);

#endif // __BUTTONS_H__