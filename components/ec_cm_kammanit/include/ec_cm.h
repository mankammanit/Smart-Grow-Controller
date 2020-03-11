#ifndef __EC_CM_H__
#define __EC_CM_H__

#include "uart_cm.h"
#include "main.h"
#include "st_profile.h"
#include "nvs_storage.h"

float ec_read(int averageVoltage);
uint8_t calibrat_ec(int volt);
extern float ec_temp;
#endif
