#ifndef __BATTERY_VOLTAGE_H__
#define __BATTERY_VOLTAGE_H__
#include <zephyr.h>

void init_battery_voltage();

void sample_battery_voltage();

float read_battery_voltage();



#endif // __BATTERY_VOLTAGE_H__