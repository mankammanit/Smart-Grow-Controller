#ifndef __PH_CM_H__
#define __PH_CM_H__

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"


//center voltage 7.0
#define CENTER_POINT7 2400.00
//center voltage 4.0
#define CENTER_POINT4 1400.00

//range buffer ph7.0
#define PH_8_VOLTAGE 2400.00
#define PH_6_VOLTAGE 2600.00

//range buffer ph4.0
#define PH_5_VOLTAGE 1300.00
#define PH_3_VOLTAGE 1500.00


float adc_reading_ph,adc_reading_buff;

esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t PH_CH = ADC_CHANNEL_6;     //GPIO34 if ADC1
static const adc_atten_t atten = ADC_ATTEN_DB_11;

void readAnalogpH();
float readPH(float voltage);
void init_ph();
void phCalibration(uint8_t mode);
void calibration(float voltage,uint8_t mode);
void print_char_val_type(esp_adc_cal_value_t val_type);
void check_efuse();
#endif
