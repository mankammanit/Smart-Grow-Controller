#ifndef __nvs_storage_H__
#define __nvs_storage_H__

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "time.h"
#include "sdkconfig.h"
#include "st_profile.h"
#include "string.h"
#include "pca9685.h"
#include "main.h"
#include "feeding_cm.h"

////////////////////////////////////

void save_ferti(ferti_set ptr);
bool read_ferti(ferti_set *ptr);
void save_ratio(ratio_val ptr);
bool read_ratio(ratio_val *ptr);
////////////////////////////////////
void save_ph_kvalue(ph_str_val ptr);
bool read_ph_kvalue(ph_str_val *ptr);
void save_ec_kvalue(ec_str_val ptr);
bool read_ec_kvalue(ec_str_val *ptr);
/////////////////////////////////////
void save_program(timepg ptr);
bool read_program(timepg *ptr);
//////////////////////////////////////
void save_statuspg(statuspg ptr);
bool read_statuspg(statuspg *ptr);
void save_time(struct tm *start_time);
bool read_time(struct tm *start_time);
///////////////////////////////////////
void save_environment(environment ptr);
bool read_environment(environment *ptr);
///////////////////////////////////////
esp_err_t resetfactory();
void load_default_nvs();
void save_working(str_working ptr);
bool read_working(str_working *ptr);

#endif
