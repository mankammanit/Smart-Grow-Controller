#ifndef __FEEDING_CM_H__
#define __FEEDING_CM_H__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <driver/i2c.h>
#include <driver/gpio.h>
#include "string.h"
#include "uart_cm.h"
#include "st_profile.h"
#include "rtc_ds1307.h"
#include "nvs_storage.h"
#include "pca9685.h"
#include "main.h"
#include "wifi.h"

uint8_t call_water_lv();
// void SETFILL(uint8_t pin, bool val);
void SETFILL(uint8_t pin, bool val,char* info);
uint8_t call_status_pump();
extern float ec_value;
extern float ph_value;


void task_feeding_all();

extern bool start_dosing_ec;
extern bool start_dosing_ph;

extern float ec_setpoint;
extern float ph_setpoint;

extern uint8_t ratio_time[4];

extern bool send_after_connect;

extern time_t current_stamp;
extern time_t feed_stamp;

typedef enum {
        fer_ok=1,
        dosing_ec=2,
        dosing_ph=3
} expression_t;
extern expression_t expression_task;


#endif
