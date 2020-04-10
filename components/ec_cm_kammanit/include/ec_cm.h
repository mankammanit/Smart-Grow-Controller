#ifndef __EC_CM_H__
#define __EC_CM_H__

#include "uart_cm.h"
#include "main.h"
#include "st_profile.h"
#include "nvs_storage.h"

float ec_read(int voltage);
void ec_calibration(int voltage,uint8_t mode);
void ecCalibration(uint8_t mode);
void ec_add_val();
extern float ec_temp;

#define RAWEC_1413_LOW       0.70
#define RAWEC_1413_HIGH      1.80
#define RAWEC_276_LOW        1.95
#define RAWEC_276_HIGH       3.2
#define RAWEC_1288_LOW       8
#define RAWEC_1288_HIGH      16.8




#endif
