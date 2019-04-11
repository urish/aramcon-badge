#ifndef __DISPLAY_H__
#define __DISPLAY_H__
#include <zephyr.h>

void init_display();
void clear_display();
void print_line_to_display(u32_t line_idx, char *str);
void flush_display();

#endif // __DISPLAY_H__