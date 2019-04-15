#ifndef __VIBRATION_MOTOR_H__
#define __VIBRATION_MOTOR_H__
#include <zephyr.h>

void init_vibration_motor();
void write_vibration_motor(u32_t value);

#endif // __VIBRATION_MOTOR_H__