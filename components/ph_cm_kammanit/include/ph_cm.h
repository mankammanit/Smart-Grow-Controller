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

//range buffer ph7.0
#define PH_8_VOLTAGE 2400.00
#define PH_6_VOLTAGE 2600.00

//range buffer ph4.0
#define PH_5_VOLTAGE 1300.00
#define PH_3_VOLTAGE 1500.00

//center voltage 7.0
#define CENTER_POINT7 (PH_8_VOLTAGE+PH_6_VOLTAGE)/2
//center voltage 4.0
#define CENTER_POINT4 (PH_5_VOLTAGE+PH_3_VOLTAGE)/2


float adc_reading_ph,adc_reading_buff;

esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t PH_CH = ADC_CHANNEL_6;     //GPIO34 if ADC1
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;
#define ADC_WIDTH ADC_WIDTH_BIT_12
#define ADC_12_BIT_RES         4096
#define VREF                   1090 // Value in mV.  Change this per board.  Use route vref to GPIO function to get this value from a multimeter

static const uint32_t adc1_vref_atten_scale[4] = {57431, 76236, 105481, 196602};
static const uint32_t adc1_vref_atten_offset[4] = {75, 78, 107, 142};
#define NO_OF_SAMPLES   1000          //Multisampling

void readAnalogpH();
float readPH(float voltage);
void init_ph();
void phCalibration(uint8_t mode);
void calibration(float voltage,uint8_t mode);
void print_char_val_type(esp_adc_cal_value_t val_type);
void check_efuse();
#endif
