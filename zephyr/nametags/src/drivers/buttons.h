#ifndef __BUTTONS_H__
#define __BUTTONS_H__
#include <zephyr.h>
#define BUTTON_COUNT 3

#define LEFT_BUTTON 2
#define MIDDLE_BUTTON 1
#define RIGHT_BUTTON 0

struct keyboard_event {
    u8_t button;
    bool pressed;
};

extern struct k_msgq keyboard_event_queue;

void init_buttons();

bool button_read(u8_t button_number);

#endif // __BUTTONS_H__