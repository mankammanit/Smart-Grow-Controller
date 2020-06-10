#include <driver/i2c.h>
#include <driver/gpio.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>
#include <esp_task_wdt.h>

//cm include lib
#include "pca9685.h"
#include "buzzer.h"
#include "main.h"
#include "uart_cm.h"
#include "ph_cm.h"
#include "st_profile.h"
#include "nvs_storage.h"
#include "rtc_ds1307.h"
#include "hdc1080.h"
#include "ds18b20.h"
#include "wifi.h"
#include "ec_cm.h"
#include "feeding_cm.h"
#include "function.h"
//nextion ota
#include "nex_ota.h"
//esp32 ota
#include "OTAServer.h"
#include "cJSON.h"

#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_spiffs.h"



static void task_timer_sch(void *pvParameters)
{
        //Subscribe this task to TWDT, then check if it is subscribed
        CHECK_ERROR_CODE(esp_task_wdt_add(task_3), ESP_OK);
        CHECK_ERROR_CODE(esp_task_wdt_status(task_3), ESP_OK);

        struct tm time;
        readValue(&time);
        read_time(&start_time);
        read_time_pump(&start_time2);

        while (1)
        {
                CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK); //Comment this line to trigger a TWDT timeout
                // printf("Task running on core %d\n",xPortGetCoreID());
                ESP_LOGI("ESP32", "[APP] Free memory: %d bytes", esp_get_free_heap_size());


                sprintf(tft_val, "My memory is %d", esp_get_free_heap_size());
                sprintf(str_name,DEBUG_TFT,tft_val);
                send_tft(str_name);

                ///clock
                Timestamp     = difftime(mktime(&time)+time_since_boot, mktime(&start_time));
                Pumpstamp     = difftime(mktime(&time)+time_since_boot, mktime(&start_time2));
                current_stamp = mktime(&time)+time_since_boot;
                feed_stamp    = mktime(&time)+time_since_boot;

                // printf("current_stamp %s", asctime(localtime(&current_stamp)));
                // printf("feed_stamp %s", asctime(localtime(&feed_stamp)));
                // printf("Timestamp %ld\n", Timestamp/86400);

                ///read last nvs schedule
                read_working(&working_timer);
                read_ferti(&ferti_set_val);
                read_program(&time_pg);
                read_statuspg(&status_pg);
                read_environment(&_environment);

                //function i2c pca9685_1
                call_time_light();

                //function i2c pca9685_2
                call_time_waterpump();
                task_feeding_all();
                waterlv();
                call_fill_mode_time();

                vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
}

static void read_sensor_task(void *pvParameters)
{
        //Subscribe this task to TWDT, then check if it is subscribed
        CHECK_ERROR_CODE(esp_task_wdt_add(task_4), ESP_OK);
        CHECK_ERROR_CODE(esp_task_wdt_status(task_4), ESP_OK);

        while (1)
        {

                CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK); //Comment this line to trigger a TWDT timeout

                read_sensor_all();
                update_mqtt_cm();
                if (network_is_alive())
                {
                        call_rssi();
                }
                vTaskDelay(10000 / portTICK_PERIOD_MS);
        }
}

static void RECV_CALL_EC(void *pvParameters)
{

        //Subscribe this task to TWDT, then check if it is subscribed
        CHECK_ERROR_CODE(esp_task_wdt_add(task_1), ESP_OK);
        CHECK_ERROR_CODE(esp_task_wdt_status(task_1), ESP_OK);

        while (1)
        {

                CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK); //Comment this line to trigger a TWDT timeout

                strcpy(ec_uart, readec());
                cJSON *attributes = cJSON_Parse(ec_uart);
                if (attributes != NULL)
                {
                        int ec_val = cJSON_GetObjectItem(attributes,"EC")->valueint;
                        double temp_val = cJSON_GetObjectItem(attributes,"TEMP")->valuedouble;

                        ec_val_plot = ec_val;
                        if(temp_val<0.00) ec_temp = 25.00;
                        else ec_temp = temp_val;
                }
                cJSON_Delete(attributes);

                vTaskDelay(100 / portTICK_PERIOD_MS);
        }
}

static void RECV_CALL_TFT(void *pvParameter)
{
        //Subscribe this task to TWDT, then check if it is subscribed
        CHECK_ERROR_CODE(esp_task_wdt_add(task_2), ESP_OK);
        CHECK_ERROR_CODE(esp_task_wdt_status(task_2), ESP_OK);

        while (1)
        {

                CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK); //Comment this line to trigger a TWDT timeout

                strcpy(dtmp, readtft());
                uint8_t dtmp_length = strlen(dtmp);
                for (uint8_t i = 0; i < dtmp_length; i++)
                {
                        // printf("recv[%d] = %02x,%d\n", i, dtmp[i],dtmp[i]);
                        dtmp[i] -= 1;
                        ESP_LOGI("RX_TASK2", "RECV[%d]:%x,%d,%c", i, dtmp[i], dtmp[i], dtmp[i]);
                }

                switch (dtmp[0])
                {
                case PAGE_LOADING:
                        printf("case PAGE_LOADING\n");

                        //1st load nvs default
                        if (read_ratio(&ratio_led))
                                ;
                        else
                        {
                                for (size_t i = 0; i < 4; i++)
                                {

                                        ratio_led.bright_1[i] = 100;
                                        ratio_led.bright_2[i] = 100;
                                        ratio_led.bright_3[i] = 100;
                                        ratio_led.bright_4[i] = 100;
                                }
                                save_ratio(ratio_led);
                        }

                        if (read_program(&time_pg))
                                ;
                        else
                        {
                                for (uint8_t i = 0; i < 4; i++)
                                {
                                        for(uint8_t j=0; j<4; j++ )
                                        {
                                                time_pg.dayon[i] = 1;
                                                time_pg.dayoff[i] = 2;

                                                time_pg.pg_hour_on[i][j] = 18;
                                                time_pg.pg_min_on[i][j] = 0;


                                                time_pg.pg_hour_off[i][j] = 8;
                                                time_pg.pg_min_off[i][j] = 0;


                                                time_pg.bright_1[i][j] = 100;
                                                time_pg.bright_2[i][j] = 100;
                                                time_pg.bright_3[i][j] = 100;
                                                time_pg.bright_4[i][j] = 100;
                                        }
                                }

                                save_program(time_pg);
                        }

                        if (read_environment(&_environment))
                                ;
                        else
                        {
                                //mode off default
                                _environment.fill1_state=MODE_OFF;
                                _environment.fill2_state=MODE_OFF;
                                _environment.fill3_state=MODE_OFF;
                                _environment.fill4_state=MODE_OFF;
                                _environment.pump_ph_state=MODE_OFF;
                                _environment.solenoide_state=MODE_OFF;
                                _environment.pump_water_state=MODE_OFF;

                                //dutycycle high default
                                _environment.fill1_duty=DUTY_HIGH;
                                _environment.fill2_duty=DUTY_HIGH;
                                _environment.fill3_duty=DUTY_HIGH;
                                _environment.fill4_duty=DUTY_HIGH;
                                _environment.pump_ph_duty=DUTY_HIGH;

                                //calibration fill1-4 (EC)
                                _environment.fill1_caribrate=1;
                                _environment.fill2_caribrate=1;
                                _environment.fill3_caribrate=1;
                                _environment.fill4_caribrate=1;

                                save_environment(_environment);
                        }

                        if (read_ferti(&ferti_set_val))
                                ;
                        else
                        {
                                ferti_set_val.ec_set_point = 12;
                                ferti_set_val.ph_set_point = 68;
                                for (uint8_t i = 0; i < 4; i++)
                                {
                                        ferti_set_val.ratio_fer[i] = 1;
                                }
                                ferti_set_val.ph_working_on_h[0] = 8;
                                ferti_set_val.ph_working_on_m[0] = 0;
                                ferti_set_val.ph_working_off_h[0] = 8;
                                ferti_set_val.ph_working_off_m[0] = 10;

                                ferti_set_val.ph_working_on_h[1] = 9;
                                ferti_set_val.ph_working_on_m[1] = 0;
                                ferti_set_val.ph_working_off_h[1] = 9;
                                ferti_set_val.ph_working_off_m[1] = 10;

                                ferti_set_val.ph_working_on_h[2] = 10;
                                ferti_set_val.ph_working_on_m[2] = 0;
                                ferti_set_val.ph_working_off_h[2] = 10;
                                ferti_set_val.ph_working_off_m[2] = 10;

                                ferti_set_val.ph_working_on_h[3] = 11;
                                ferti_set_val.ph_working_on_m[3] = 0;
                                ferti_set_val.ph_working_off_h[3] = 11;
                                ferti_set_val.ph_working_off_m[3] = 10;

                                ferti_set_val.ph_status_timer[0] = 0;
                                ferti_set_val.ph_status_timer[1] = 0;
                                ferti_set_val.ph_status_timer[2] = 0;
                                ferti_set_val.ph_status_timer[3] = 0;

                                ferti_set_val.ratio_ph = 1;
                                ferti_set_val.wait_ph = 10;
                                ferti_set_val.wait_ec = 10;
                                save_ferti(ferti_set_val);
                        }



                        if (read_working(&working_timer))
                                ;
                        else
                        {
                                working_timer.working_on_h[0] = 8;
                                working_timer.working_on_m[0] = 0;
                                working_timer.working_off_h[0] = 8;
                                working_timer.working_off_m[0] = 10;

                                working_timer.working_on_h[1] = 9;
                                working_timer.working_on_m[1] = 0;
                                working_timer.working_off_h[1] = 9;
                                working_timer.working_off_m[1] = 10;

                                working_timer.working_on_h[2] = 10;
                                working_timer.working_on_m[2] = 0;
                                working_timer.working_off_h[2] = 10;
                                working_timer.working_off_m[2] = 10;

                                working_timer.working_on_h[3] = 11;
                                working_timer.working_on_m[3] = 0;
                                working_timer.working_off_h[3] = 11;
                                working_timer.working_off_m[3] = 10;

                                working_timer.status_timer[0] = 0;
                                working_timer.status_timer[1] = 0;
                                working_timer.status_timer[2] = 0;
                                working_timer.status_timer[3] = 0;

                                working_timer.working_day = 0;
                                working_timer.working_nextday = 0;
                                working_timer.working_lastday = 0;

                                working_timer.fill_working_on_h[0] = 8;
                                working_timer.fill_working_on_m[0] = 0;
                                working_timer.fill_working_off_h[0] = 8;
                                working_timer.fill_working_off_m[0] = 10;

                                working_timer.fill_working_on_h[1] = 9;
                                working_timer.fill_working_on_m[1] = 0;
                                working_timer.fill_working_off_h[1] = 9;
                                working_timer.fill_working_off_m[1] = 10;

                                working_timer.fill_working_on_h[2] = 10;
                                working_timer.fill_working_on_m[2] = 0;
                                working_timer.fill_working_off_h[2] = 10;
                                working_timer.fill_working_off_m[2] = 10;

                                working_timer.fill_working_on_h[3] = 11;
                                working_timer.fill_working_on_m[3] = 0;
                                working_timer.fill_working_off_h[3] = 11;
                                working_timer.fill_working_off_m[3] = 10;

                                working_timer.fill_status_timer[0] = 0;
                                working_timer.fill_status_timer[1] = 0;
                                working_timer.fill_status_timer[2] = 0;
                                working_timer.fill_status_timer[3] = 0;

                                save_working(working_timer);
                        }

                        read_statuspg(&status_pg);

                        sprintf(str_name, DAY_PLANTED, status_pg.start_day);
                        send_tft(str_name);
                        //wepo to status
                        sprintf(str_name, SAVE_EEPROM_TFT, status_pg.switch_mode, 30);
                        send_tft(str_name);
                        sprintf(str_name, DEVICE_NO, status_pg.mydevice_no);
                        send_tft(str_name);

                        set_bright(ZONE1_LED1, ZONE1_LED2, ZONE1_LED3, ZONE1_LED4,
                                   ratio_led.bright_1[0],
                                   ratio_led.bright_2[0],
                                   ratio_led.bright_3[0],
                                   ratio_led.bright_4[0],
                                   "LOAD_MANUAL_ZONE1");

                        set_bright(ZONE2_LED1, ZONE2_LED2, ZONE2_LED3, ZONE2_LED4,
                                   ratio_led.bright_1[1],
                                   ratio_led.bright_2[1],
                                   ratio_led.bright_3[1],
                                   ratio_led.bright_4[1],
                                   "LOAD_MANUAL_ZONE2");

                        set_bright(ZONE3_LED1, ZONE3_LED2, ZONE3_LED3, ZONE3_LED4,
                                   ratio_led.bright_1[2],
                                   ratio_led.bright_2[2],
                                   ratio_led.bright_3[2],
                                   ratio_led.bright_4[2],
                                   "LOAD_MANUAL_ZONE3");

                        set_bright(ZONE4_LED1, ZONE4_LED2, ZONE4_LED3, ZONE4_LED4,
                                   ratio_led.bright_1[3],
                                   ratio_led.bright_2[3],
                                   ratio_led.bright_3[3],
                                   ratio_led.bright_4[3],
                                   "LOAD_MANUAL_ZONE4");

                        switch (_environment.fill1_state) {
                        case MODE_OFF:
                                printf("PUMP_FILL_1 (A) to mode OFF\n");
                                SETFILL(PUMP_A,false,"PUMP_A_OFF");
                                break;
                        case MODE_ON:
                                printf("PUMP_FILL_1 (A) to mode ON\n");
                                SETFILL(PUMP_A,true,"PUMP_A_ON");
                                break;
                        case MODE_AUTO:
                                printf("PUMP_FILL_1 (A) to mode AUTO\n");
                                SETFILL(PUMP_A,false,"PUMP_A_OFF");
                                break;
                        }
                        switch (_environment.fill2_state) {
                        case MODE_OFF:
                                printf("PUMP_FILL_2 (B) to mode OFF\n");
                                SETFILL(PUMP_B,false,"PUMP_B_OFF");
                                break;
                        case MODE_ON:
                                printf("PUMP_FILL_2 (B) to mode ON\n");
                                SETFILL(PUMP_B,true,"PUMP_B_ON");
                                break;
                        case MODE_AUTO:
                                printf("PUMP_FILL_2 (B) to mode AUTO\n");
                                SETFILL(PUMP_B,false,"PUMP_B_OFF");
                                break;
                        }

                        switch (_environment.fill3_state) {
                        case MODE_OFF:
                                printf("PUMP_FILL_3 (C) to mode OFF\n");
                                SETFILL(PUMP_C,false,"PUMP_C_OFF");
                                break;
                        case MODE_ON:
                                printf("PUMP_FILL_3 (C) to mode ON\n");
                                SETFILL(PUMP_C,true,"PUMP_C_ON");
                                break;
                        case MODE_AUTO:
                                printf("PUMP_FILL_3 (C) to mode AUTO\n");
                                SETFILL(PUMP_C,false,"PUMP_C_OFF");
                                break;
                        }

                        switch (_environment.fill4_state) {
                        case MODE_OFF:
                                printf("PUMP_FILL_4 (D) to mode OFF\n");
                                SETFILL(PUMP_D,false,"PUMP_D_OFF");
                                break;
                        case MODE_ON:
                                printf("PUMP_FILL_4 (D) to mode ON\n");
                                SETFILL(PUMP_D,true,"PUMP_D_ON");
                                break;
                        case MODE_AUTO:
                                printf("PUMP_FILL_4 (D) to mode AUTO\n");
                                SETFILL(PUMP_D,false,"PUMP_D_OFF");
                                break;
                        }

                        switch (_environment.pump_ph_state) {
                        case MODE_OFF:
                                printf("PUMP_FILL_PH to mode OFF\n");
                                SETFILL(PUMP_PH,false,"PUMP_PH_OFF");
                                break;
                        case MODE_ON:
                                printf("PUMP_FILL_PH to mode ON\n");
                                SETFILL(PUMP_PH,true,"PUMP_PH_ON");
                                break;
                        case MODE_AUTO:
                                printf("PUMP_FILL_PH to mode AUTO\n");
                                SETFILL(PUMP_PH,false,"PUMP_PH_OFF");
                                break;
                        case MODE_TIME:
                                printf("PUMP_FILL_PH to mode TIME\n");
                                SETFILL(PUMP_PH,false,"PUMP_PH_OFF");
                                break;
                        }

                        switch (_environment.solenoide_state)
                        {
                        case MODE_OFF:
                                printf("SOLENOIDE to mode OFF\n");
                                SETFILL(SOLENOIDE, false,"SOLENOIDE_OFF");
                                break;
                        case MODE_ON:
                                printf("SOLENOIDE to mode ON\n");
                                SETFILL(SOLENOIDE, true,"SOLENOIDE_ON");
                                break;
                        case MODE_AUTO:
                                printf("SOLENOIDE to mode AUTO\n");
                                SETFILL(SOLENOIDE, false,"SOLENOIDE_OFF");
                                break;
                        case MODE_TIME:
                                printf("PUMP_FILL_PH to mode TIME\n");
                                SETFILL(SOLENOIDE,false,"SOLENOIDE_OFF");
                                break;
                        }

                        switch (_environment.pump_water_state)
                        {
                        case MODE_OFF:
                                printf("PUMP_WATER to mode OFF\n");
                                SETFILL(ON_OFF_P1, false,"PUMP_WATER_OFF");
                                break;
                        case MODE_ON:
                                printf("PUMP_WATER to mode ON\n");
                                SETFILL(ON_OFF_P1, true,"PUMP_WATER_ON");
                                break;
                        case MODE_AUTO:
                                printf("PUMP_WATER to mode AUTO\n");
                                SETFILL(ON_OFF_P1, false,"PUMP_WATER_OFF");
                                break;
                        }

                        check_light_on_off();

                        start_dosing_ec = true;
                        start_dosing_ph = true;

                        break;
                case PAGE_HOME:
                        printf("case PAGE_HOME\n");
                        call_time(1);
                        break;

                case DASH_BOARD:
                        printf("case DASH_BOARD\n");
                        call_time(1);
                        update_dashboard();
                        sprintf(tft_val,"%02d/%02d %02d:%02d",start_time2.tm_mday,start_time2.tm_mon+1,
                                start_time2.tm_hour,start_time2.tm_min);
                        sprintf(str_name, PLOT_TXT,457,316,85,20,tft_val);
                        send_tft(str_name);

                        check_light_on_off();

                        break;

                case PAGE_RATIO:
                        // printf("case PAGE_RATIO_MAN\n");
                        read_ratio(&ratio_led);
                        switch (dtmp[1])
                        {

                        case 1:
                                printf("case PAGE_RATIO_MAN ZONE1\n");
                                sprintf(str_name, SET_ZONE_LED1, ratio_led.bright_1[0]);
                                send_tft(str_name);
                                sprintf(str_name, SET_ZONE_LED2, ratio_led.bright_2[0]);
                                send_tft(str_name);
                                sprintf(str_name, SET_ZONE_LED3, ratio_led.bright_3[0]);
                                send_tft(str_name);
                                sprintf(str_name, SET_ZONE_LED4, ratio_led.bright_4[0]);
                                send_tft(str_name);


                                break;

                        case 2:
                                printf("case PAGE_RATIO_MAN ZONE2\n");
                                sprintf(str_name, SET_ZONE_LED1, ratio_led.bright_1[1]);
                                send_tft(str_name);
                                sprintf(str_name, SET_ZONE_LED2, ratio_led.bright_2[1]);
                                send_tft(str_name);
                                sprintf(str_name, SET_ZONE_LED3, ratio_led.bright_3[1]);
                                send_tft(str_name);
                                sprintf(str_name, SET_ZONE_LED4, ratio_led.bright_4[1]);
                                send_tft(str_name);

                                break;

                        case 3:
                                printf("case PAGE_RATIO_MAN ZONE3\n");
                                sprintf(str_name, SET_ZONE_LED1, ratio_led.bright_1[2]);
                                send_tft(str_name);
                                sprintf(str_name, SET_ZONE_LED2, ratio_led.bright_2[2]);
                                send_tft(str_name);
                                sprintf(str_name, SET_ZONE_LED3, ratio_led.bright_3[2]);
                                send_tft(str_name);
                                sprintf(str_name, SET_ZONE_LED4, ratio_led.bright_4[2]);
                                send_tft(str_name);


                                break;

                        case 4:
                                printf("case PAGE_RATIO_MAN ZONE4\n");
                                sprintf(str_name, SET_ZONE_LED1, ratio_led.bright_1[3]);
                                send_tft(str_name);
                                sprintf(str_name, SET_ZONE_LED2, ratio_led.bright_2[3]);
                                send_tft(str_name);
                                sprintf(str_name, SET_ZONE_LED3, ratio_led.bright_3[3]);
                                send_tft(str_name);
                                sprintf(str_name, SET_ZONE_LED4, ratio_led.bright_4[3]);
                                send_tft(str_name);


                                break;
                        }


                        break;

                case RATIO:
                        // printf("case RATIO_MAN\n");

                        switch (dtmp[1])
                        {
                        case 1:
                                printf("case RATIO_MAN ZONE1\n");
                                ratio_led.bright_1[0] = dtmp[2];
                                ratio_led.bright_2[0] = dtmp[3];
                                ratio_led.bright_3[0] = dtmp[4];
                                ratio_led.bright_4[0] = dtmp[5];

                                set_bright(ZONE1_LED1, ZONE1_LED2, ZONE1_LED3, ZONE1_LED4,
                                           dtmp[2],
                                           dtmp[3],
                                           dtmp[4],
                                           dtmp[5],
                                           "SET_MANUAL_ZONE1");

                                break;

                        case 2:
                                printf("case RATIO_MAN ZONE2\n");
                                ratio_led.bright_1[1] = dtmp[2];
                                ratio_led.bright_2[1] = dtmp[3];
                                ratio_led.bright_3[1] = dtmp[4];
                                ratio_led.bright_4[1] = dtmp[5];

                                set_bright(ZONE2_LED1, ZONE2_LED2, ZONE2_LED3, ZONE2_LED4,
                                           dtmp[2],
                                           dtmp[3],
                                           dtmp[4],
                                           dtmp[5],
                                           "SET_MANUAL_ZONE2");
                                break;

                        case 3:
                                printf("case RATIO_MAN ZONE3\n");
                                ratio_led.bright_1[2] = dtmp[2];
                                ratio_led.bright_2[2] = dtmp[3];
                                ratio_led.bright_3[2] = dtmp[4];
                                ratio_led.bright_4[2] = dtmp[5];

                                set_bright(ZONE3_LED1, ZONE3_LED2, ZONE3_LED3, ZONE3_LED4,
                                           dtmp[2],
                                           dtmp[3],
                                           dtmp[4],
                                           dtmp[5],
                                           "SET_MANUAL_ZONE3");
                                break;

                        case 4:
                                printf("case RATIO_MAN ZONE4\n");
                                ratio_led.bright_1[3] = dtmp[2];
                                ratio_led.bright_2[3] = dtmp[3];
                                ratio_led.bright_3[3] = dtmp[4];
                                ratio_led.bright_4[3] = dtmp[5];

                                set_bright(ZONE4_LED1, ZONE4_LED2, ZONE4_LED3, ZONE4_LED4,
                                           dtmp[2],
                                           dtmp[3],
                                           dtmp[4],
                                           dtmp[5],
                                           "SET_MANUAL_ZONE4");
                                break;
                        }
                        save_ratio(ratio_led);

                        break;

                case PAGE_PROGRAM:

                        printf("case PAGE_PROGRAM\n");

                        sprintf(str_name, DAY_PLANTED, Timestamp_day);
                        send_tft(str_name);

                        //protect
                        if(time_pg.dayon[1]<=time_pg.dayoff[0])
                        {
                                time_pg.dayon[1]=time_pg.dayoff[0]+1;
                                time_pg.dayoff[1]=time_pg.dayon[1]+1;
                                time_pg.dayon[2]=time_pg.dayoff[1]+1;
                                save_program(time_pg);
                                sprintf(str_name,PAGE_TFT,"L_auto");
                                send_tft(str_name);
                        }
                        if(time_pg.dayon[2]<=time_pg.dayoff[1])
                        {
                                time_pg.dayon[2]=time_pg.dayoff[1]+1;
                                save_program(time_pg);
                                sprintf(str_name,PAGE_TFT,"L_auto");
                                send_tft(str_name);
                        }

                        //STAGE1
                        sprintf(str_name, STAGE_1_ON,time_pg.dayon[0]);
                        send_tft(str_name);
                        sprintf(str_name, STAGE_1_OFF,time_pg.dayoff[0]);
                        send_tft(str_name);

                        //STAGE2
                        sprintf(str_name, STAGE_2_ON,time_pg.dayon[1]);
                        send_tft(str_name);
                        sprintf(str_name, STAGE_2_OFF,time_pg.dayoff[1]);
                        send_tft(str_name);

                        //STAGE1
                        sprintf(str_name, STAGE_3_ON,time_pg.dayon[2]);
                        send_tft(str_name);
                        sprintf(str_name, STAGE_3_OFF,time_pg.dayoff[2]);
                        send_tft(str_name);



                        break;

                case PAGE_ZONE:
                        // printf("case PAGE_ZONE\n");
                        switch (dtmp[1]) {
                        case 1:
                                printf("case PAGE_ZONE STAGE1\n");

                                sprintf(str_name, ZONE1_led1, time_pg.bright_1[0][0]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE1_led2, time_pg.bright_2[0][0]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE1_led3, time_pg.bright_3[0][0]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE1_led4, time_pg.bright_4[0][0]);
                                send_tft(str_name);

                                sprintf(str_name, ZONE2_led1, time_pg.bright_1[0][1]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE2_led2, time_pg.bright_2[0][1]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE2_led3, time_pg.bright_3[0][1]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE2_led4, time_pg.bright_4[0][1]);
                                send_tft(str_name);

                                sprintf(str_name, ZONE3_led1, time_pg.bright_1[0][2]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE3_led2, time_pg.bright_2[0][2]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE3_led3, time_pg.bright_3[0][2]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE3_led4, time_pg.bright_4[0][2]);
                                send_tft(str_name);

                                sprintf(str_name, ZONE4_led1, time_pg.bright_1[0][3]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE4_led2, time_pg.bright_2[0][3]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE4_led3, time_pg.bright_3[0][3]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE4_led4, time_pg.bright_4[0][3]);
                                send_tft(str_name);

                                sprintf(tft_val, "%02d", time_pg.pg_hour_on[0][0]);
                                sprintf(tft_val2, "%02d", time_pg.pg_min_on[0][0]);
                                sprintf(tft_val3, "%02d", time_pg.pg_hour_off[0][0]);
                                sprintf(tft_val4, "%02d", time_pg.pg_min_off[0][0]);
                                sprintf(str_name, ON_OFF_1, tft_val, tft_val2, tft_val3, tft_val4);
                                send_tft(str_name);

                                sprintf(tft_val, "%02d", time_pg.pg_hour_on[0][1]);
                                sprintf(tft_val2, "%02d", time_pg.pg_min_on[0][1]);
                                sprintf(tft_val3, "%02d", time_pg.pg_hour_off[0][1]);
                                sprintf(tft_val4, "%02d", time_pg.pg_min_off[0][1]);
                                sprintf(str_name, ON_OFF_2, tft_val, tft_val2, tft_val3, tft_val4);
                                send_tft(str_name);

                                sprintf(tft_val, "%02d", time_pg.pg_hour_on[0][2]);
                                sprintf(tft_val2, "%02d", time_pg.pg_min_on[0][2]);
                                sprintf(tft_val3, "%02d", time_pg.pg_hour_off[0][2]);
                                sprintf(tft_val4, "%02d", time_pg.pg_min_off[0][2]);
                                sprintf(str_name, ON_OFF_3, tft_val, tft_val2, tft_val3, tft_val4);
                                send_tft(str_name);

                                sprintf(tft_val, "%02d", time_pg.pg_hour_on[0][3]);
                                sprintf(tft_val2, "%02d", time_pg.pg_min_on[0][3]);
                                sprintf(tft_val3, "%02d", time_pg.pg_hour_off[0][3]);
                                sprintf(tft_val4, "%02d", time_pg.pg_min_off[0][3]);
                                sprintf(str_name, ON_OFF_4, tft_val, tft_val2, tft_val3, tft_val4);
                                send_tft(str_name);

                                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[0][0], time_pg.pg_hour_off[0][0],
                                                                 time_pg.pg_min_on[0][0], time_pg.pg_min_off[0][0]));
                                sprintf(str_name, DUARATION_1, tft_val);
                                send_tft(str_name);

                                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[0][1], time_pg.pg_hour_off[0][1],
                                                                 time_pg.pg_min_on[0][1], time_pg.pg_min_off[0][1]));
                                sprintf(str_name, DUARATION_2, tft_val);
                                send_tft(str_name);

                                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[0][2], time_pg.pg_hour_off[0][2],
                                                                 time_pg.pg_min_on[0][2], time_pg.pg_min_off[0][2]));
                                sprintf(str_name, DUARATION_3, tft_val);
                                send_tft(str_name);

                                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[0][3], time_pg.pg_hour_off[0][3],
                                                                 time_pg.pg_min_on[0][3], time_pg.pg_min_off[0][3]));
                                sprintf(str_name, DUARATION_4, tft_val);
                                send_tft(str_name);

                                break;
                        case 2:
                                printf("case PAGE_ZONE STAGE2\n");

                                sprintf(str_name, ZONE1_led1, time_pg.bright_1[1][0]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE1_led2, time_pg.bright_2[1][0]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE1_led3, time_pg.bright_3[1][0]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE1_led4, time_pg.bright_4[1][0]);
                                send_tft(str_name);

                                sprintf(str_name, ZONE2_led1, time_pg.bright_1[1][1]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE2_led2, time_pg.bright_2[1][1]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE2_led3, time_pg.bright_3[1][1]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE2_led4, time_pg.bright_4[1][1]);
                                send_tft(str_name);

                                sprintf(str_name, ZONE3_led1, time_pg.bright_1[1][2]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE3_led2, time_pg.bright_2[1][2]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE3_led3, time_pg.bright_3[1][2]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE3_led4, time_pg.bright_4[1][2]);
                                send_tft(str_name);

                                sprintf(str_name, ZONE4_led1, time_pg.bright_1[1][3]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE4_led2, time_pg.bright_2[1][3]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE4_led3, time_pg.bright_3[1][3]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE4_led4, time_pg.bright_4[1][3]);
                                send_tft(str_name);

                                sprintf(tft_val, "%02d", time_pg.pg_hour_on[1][0]);
                                sprintf(tft_val2, "%02d", time_pg.pg_min_on[1][0]);
                                sprintf(tft_val3, "%02d", time_pg.pg_hour_off[1][0]);
                                sprintf(tft_val4, "%02d", time_pg.pg_min_off[1][0]);
                                sprintf(str_name, ON_OFF_1, tft_val, tft_val2, tft_val3, tft_val4);
                                send_tft(str_name);

                                sprintf(tft_val, "%02d", time_pg.pg_hour_on[1][1]);
                                sprintf(tft_val2, "%02d", time_pg.pg_min_on[1][1]);
                                sprintf(tft_val3, "%02d", time_pg.pg_hour_off[1][1]);
                                sprintf(tft_val4, "%02d", time_pg.pg_min_off[1][1]);
                                sprintf(str_name, ON_OFF_2, tft_val, tft_val2, tft_val3, tft_val4);
                                send_tft(str_name);

                                sprintf(tft_val, "%02d", time_pg.pg_hour_on[1][2]);
                                sprintf(tft_val2, "%02d", time_pg.pg_min_on[1][2]);
                                sprintf(tft_val3, "%02d", time_pg.pg_hour_off[1][2]);
                                sprintf(tft_val4, "%02d", time_pg.pg_min_off[1][2]);
                                sprintf(str_name, ON_OFF_3, tft_val, tft_val2, tft_val3, tft_val4);
                                send_tft(str_name);

                                sprintf(tft_val, "%02d", time_pg.pg_hour_on[1][3]);
                                sprintf(tft_val2, "%02d", time_pg.pg_min_on[1][3]);
                                sprintf(tft_val3, "%02d", time_pg.pg_hour_off[1][3]);
                                sprintf(tft_val4, "%02d", time_pg.pg_min_off[1][3]);
                                sprintf(str_name, ON_OFF_4, tft_val, tft_val2, tft_val3, tft_val4);
                                send_tft(str_name);

                                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[1][0], time_pg.pg_hour_off[1][0],
                                                                 time_pg.pg_min_on[1][0], time_pg.pg_min_off[1][0]));
                                sprintf(str_name, DUARATION_1, tft_val);
                                send_tft(str_name);

                                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[1][1], time_pg.pg_hour_off[1][1],
                                                                 time_pg.pg_min_on[1][1], time_pg.pg_min_off[1][1]));
                                sprintf(str_name, DUARATION_2, tft_val);
                                send_tft(str_name);

                                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[1][2], time_pg.pg_hour_off[1][2],
                                                                 time_pg.pg_min_on[1][2], time_pg.pg_min_off[1][2]));
                                sprintf(str_name, DUARATION_3, tft_val);
                                send_tft(str_name);

                                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[1][3], time_pg.pg_hour_off[1][3],
                                                                 time_pg.pg_min_on[1][3], time_pg.pg_min_off[1][3]));
                                sprintf(str_name, DUARATION_4, tft_val);
                                send_tft(str_name);

                                break;
                        case 3:
                                printf("case PAGE_ZONE STAGE3\n");

                                sprintf(str_name, ZONE1_led1, time_pg.bright_1[2][0]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE1_led2, time_pg.bright_2[2][0]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE1_led3, time_pg.bright_3[2][0]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE1_led4, time_pg.bright_4[2][0]);
                                send_tft(str_name);

                                sprintf(str_name, ZONE2_led1, time_pg.bright_1[2][1]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE2_led2, time_pg.bright_2[2][1]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE2_led3, time_pg.bright_3[2][1]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE2_led4, time_pg.bright_4[2][1]);
                                send_tft(str_name);

                                sprintf(str_name, ZONE3_led1, time_pg.bright_1[2][2]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE3_led2, time_pg.bright_2[2][2]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE3_led3, time_pg.bright_3[2][2]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE3_led4, time_pg.bright_4[2][2]);
                                send_tft(str_name);

                                sprintf(str_name, ZONE4_led1, time_pg.bright_1[2][3]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE4_led2, time_pg.bright_2[2][3]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE4_led3, time_pg.bright_3[2][3]);
                                send_tft(str_name);
                                sprintf(str_name, ZONE4_led4, time_pg.bright_4[2][3]);
                                send_tft(str_name);

                                sprintf(tft_val, "%02d", time_pg.pg_hour_on[2][0]);
                                sprintf(tft_val2, "%02d", time_pg.pg_min_on[2][0]);
                                sprintf(tft_val3, "%02d", time_pg.pg_hour_off[2][0]);
                                sprintf(tft_val4, "%02d", time_pg.pg_min_off[2][0]);
                                sprintf(str_name, ON_OFF_1, tft_val, tft_val2, tft_val3, tft_val4);
                                send_tft(str_name);

                                sprintf(tft_val, "%02d", time_pg.pg_hour_on[2][1]);
                                sprintf(tft_val2, "%02d", time_pg.pg_min_on[2][1]);
                                sprintf(tft_val3, "%02d", time_pg.pg_hour_off[2][1]);
                                sprintf(tft_val4, "%02d", time_pg.pg_min_off[2][1]);
                                sprintf(str_name, ON_OFF_2, tft_val, tft_val2, tft_val3, tft_val4);
                                send_tft(str_name);

                                sprintf(tft_val, "%02d", time_pg.pg_hour_on[2][2]);
                                sprintf(tft_val2, "%02d", time_pg.pg_min_on[2][2]);
                                sprintf(tft_val3, "%02d", time_pg.pg_hour_off[2][2]);
                                sprintf(tft_val4, "%02d", time_pg.pg_min_off[2][2]);
                                sprintf(str_name, ON_OFF_3, tft_val, tft_val2, tft_val3, tft_val4);
                                send_tft(str_name);

                                sprintf(tft_val, "%02d", time_pg.pg_hour_on[2][3]);
                                sprintf(tft_val2, "%02d", time_pg.pg_min_on[2][3]);
                                sprintf(tft_val3, "%02d", time_pg.pg_hour_off[2][3]);
                                sprintf(tft_val4, "%02d", time_pg.pg_min_off[2][3]);
                                sprintf(str_name, ON_OFF_4, tft_val, tft_val2, tft_val3, tft_val4);
                                send_tft(str_name);

                                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[2][0], time_pg.pg_hour_off[2][0],
                                                                 time_pg.pg_min_on[2][0], time_pg.pg_min_off[2][0]));
                                sprintf(str_name, DUARATION_1, tft_val);
                                send_tft(str_name);

                                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[2][1], time_pg.pg_hour_off[2][1],
                                                                 time_pg.pg_min_on[2][1], time_pg.pg_min_off[2][1]));
                                sprintf(str_name, DUARATION_2, tft_val);
                                send_tft(str_name);

                                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[2][2], time_pg.pg_hour_off[2][2],
                                                                 time_pg.pg_min_on[2][2], time_pg.pg_min_off[2][2]));
                                sprintf(str_name, DUARATION_3, tft_val);
                                send_tft(str_name);

                                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[2][3], time_pg.pg_hour_off[2][3],
                                                                 time_pg.pg_min_on[2][3], time_pg.pg_min_off[2][3]));
                                sprintf(str_name, DUARATION_4, tft_val);
                                send_tft(str_name);

                                break;
                        }
                        break;

                case SAVE_DAY_ZONE:
                        // printf("case SAVE_DAY_ZONE\n");
                        switch (dtmp[1]) {
                        case 1:
                                printf("case SAVE_DAY_ZONE STAGE1\n");
                                if (dtmp[3] > 0)
                                {

                                        uint16_t val = dtmp[3] * 256 + dtmp[2];
                                        time_pg.dayon[0] = val;
                                }
                                else time_pg.dayon[0] = dtmp[2];

                                if (dtmp[7] > 0)
                                {

                                        uint16_t val = dtmp[7] * 256 + dtmp[6];
                                        time_pg.dayoff[0] = val;
                                }
                                else time_pg.dayoff[0] = dtmp[6];

                                time_pg.dayon[1]=time_pg.dayoff[0]+1;
                                time_pg.dayoff[1]=time_pg.dayon[1]+1;
                                time_pg.dayon[2]=time_pg.dayoff[1]+1;
                                time_pg.dayoff[2]=time_pg.dayon[2]+1;

                                break;
                        case 2:
                                printf("case SAVE_DAY_ZONE STAGE2\n");
                                if (dtmp[3] > 0)
                                {

                                        uint16_t val = dtmp[3] * 256 + dtmp[2];
                                        time_pg.dayon[1] = val;
                                }
                                else time_pg.dayon[1] = dtmp[2];

                                if (dtmp[7] > 0)
                                {

                                        uint16_t val = dtmp[7] * 256 + dtmp[6];
                                        time_pg.dayoff[1] = val;
                                }
                                else time_pg.dayoff[1] = dtmp[6];

                                time_pg.dayon[2]=time_pg.dayoff[1]+1;
                                time_pg.dayoff[2]=time_pg.dayon[2]+1;

                                break;
                        case 3:
                                printf("case SAVE_DAY_ZONE STAGE3\n");
                                if (dtmp[3] > 0)
                                {

                                        uint16_t val = dtmp[3] * 256 + dtmp[2];
                                        time_pg.dayon[2] = val;
                                }
                                else time_pg.dayon[2] = dtmp[2];

                                if (dtmp[7] > 0)
                                {

                                        uint16_t val = dtmp[7] * 256 + dtmp[6];
                                        time_pg.dayoff[2] = val;
                                }
                                else time_pg.dayoff[2] = dtmp[6];
                                break;
                        }
                        save_program(time_pg);
                        sprintf(str_name,PAGE_TFT,"L_auto");
                        send_tft(str_name);
                        break;

                case PAGE_RATIO_MENU:
                        printf("case PAGE_RATIO_MENU\n");

                        read_program(&time_pg);

                        sprintf(str_name, TXT_ZONE1_LED1, ratio_led.bright_1[0]);
                        send_tft(str_name);
                        sprintf(str_name, TXT_ZONE1_LED2, ratio_led.bright_2[0]);
                        send_tft(str_name);
                        sprintf(str_name, TXT_ZONE1_LED3, ratio_led.bright_3[0]);
                        send_tft(str_name);
                        sprintf(str_name, TXT_ZONE1_LED4, ratio_led.bright_4[0]);
                        send_tft(str_name);

                        sprintf(str_name, TXT_ZONE2_LED1, ratio_led.bright_1[1]);
                        send_tft(str_name);
                        sprintf(str_name, TXT_ZONE2_LED2, ratio_led.bright_2[1]);
                        send_tft(str_name);
                        sprintf(str_name, TXT_ZONE2_LED3, ratio_led.bright_3[1]);
                        send_tft(str_name);
                        sprintf(str_name, TXT_ZONE2_LED4, ratio_led.bright_4[1]);
                        send_tft(str_name);

                        sprintf(str_name, TXT_ZONE3_LED1, ratio_led.bright_1[2]);
                        send_tft(str_name);
                        sprintf(str_name, TXT_ZONE3_LED2, ratio_led.bright_2[2]);
                        send_tft(str_name);
                        sprintf(str_name, TXT_ZONE3_LED3, ratio_led.bright_3[2]);
                        send_tft(str_name);
                        sprintf(str_name, TXT_ZONE3_LED4, ratio_led.bright_4[2]);
                        send_tft(str_name);

                        sprintf(str_name, TXT_ZONE4_LED1, ratio_led.bright_1[3]);
                        send_tft(str_name);
                        sprintf(str_name, TXT_ZONE4_LED2, ratio_led.bright_2[3]);
                        send_tft(str_name);
                        sprintf(str_name, TXT_ZONE4_LED3, ratio_led.bright_3[3]);
                        send_tft(str_name);
                        sprintf(str_name, TXT_ZONE4_LED4, ratio_led.bright_4[3]);
                        send_tft(str_name);

                        break;

                case PAGE_RATIO_PG:
                        // printf("case PAGE_RATIO_PG\n");
                        read_program(&time_pg);
                        switch (dtmp[1]) {
                        case 1:
                                switch (dtmp[2]) {
                                case 1:
                                        printf("case PAGE_RATIO_PG1 ZONE1\n");
                                        sprintf(str_name, SET_ZONE_LED1, time_pg.bright_1[0][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED2, time_pg.bright_2[0][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED3, time_pg.bright_3[0][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED4, time_pg.bright_4[0][0]);
                                        send_tft(str_name);
                                        break;
                                case 2:
                                        printf("case PAGE_RATIO_PG1 ZONE2\n");
                                        sprintf(str_name, SET_ZONE_LED1, time_pg.bright_1[0][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED2, time_pg.bright_2[0][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED3, time_pg.bright_3[0][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED4, time_pg.bright_4[0][1]);
                                        send_tft(str_name);
                                        break;
                                case 3:
                                        printf("case PAGE_RATIO_PG1 ZONE3\n");
                                        sprintf(str_name, SET_ZONE_LED1, time_pg.bright_1[0][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED2, time_pg.bright_2[0][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED3, time_pg.bright_3[0][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED4, time_pg.bright_4[0][2]);
                                        send_tft(str_name);
                                        break;
                                case 4:
                                        printf("case PAGE_RATIO_PG1 ZONE4\n");
                                        sprintf(str_name, SET_ZONE_LED1, time_pg.bright_1[0][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED2, time_pg.bright_2[0][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED3, time_pg.bright_3[0][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED4, time_pg.bright_4[0][3]);
                                        send_tft(str_name);
                                        break;
                                }
                                break;

                        case 2:
                                switch (dtmp[2]) {
                                case 1:
                                        printf("case PAGE_RATIO_PG2 ZONE1\n");
                                        sprintf(str_name, SET_ZONE_LED1, time_pg.bright_1[1][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED2, time_pg.bright_2[1][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED3, time_pg.bright_3[1][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED4, time_pg.bright_4[1][0]);
                                        send_tft(str_name);
                                        break;
                                case 2:
                                        printf("case PAGE_RATIO_PG2 ZONE2\n");
                                        sprintf(str_name, SET_ZONE_LED1, time_pg.bright_1[1][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED2, time_pg.bright_2[1][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED3, time_pg.bright_3[1][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED4, time_pg.bright_4[1][1]);
                                        send_tft(str_name);
                                        break;
                                case 3:
                                        printf("case PAGE_RATIO_PG2 ZONE3\n");
                                        sprintf(str_name, SET_ZONE_LED1, time_pg.bright_1[1][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED2, time_pg.bright_2[1][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED3, time_pg.bright_3[1][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED4, time_pg.bright_4[1][2]);
                                        send_tft(str_name);
                                        break;
                                case 4:
                                        printf("case PAGE_RATIO_PG2 ZONE4\n");
                                        sprintf(str_name, SET_ZONE_LED1, time_pg.bright_1[1][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED2, time_pg.bright_2[1][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED3, time_pg.bright_3[1][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED4, time_pg.bright_4[1][3]);
                                        send_tft(str_name);
                                        break;
                                }
                                break;

                        case 3:
                                switch (dtmp[2]) {
                                case 1:
                                        printf("case PAGE_RATIO_PG3 ZONE1\n");
                                        sprintf(str_name, SET_ZONE_LED1, time_pg.bright_1[2][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED2, time_pg.bright_2[2][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED3, time_pg.bright_3[2][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED4, time_pg.bright_4[2][0]);
                                        send_tft(str_name);
                                        break;
                                case 2:
                                        printf("case PAGE_RATIO_PG3 ZONE2\n");
                                        sprintf(str_name, SET_ZONE_LED1, time_pg.bright_1[2][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED2, time_pg.bright_2[2][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED3, time_pg.bright_3[2][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED4, time_pg.bright_4[2][1]);
                                        send_tft(str_name);
                                        break;
                                case 3:
                                        printf("case PAGE_RATIO_PG3 ZONE3\n");
                                        sprintf(str_name, SET_ZONE_LED1, time_pg.bright_1[2][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED2, time_pg.bright_2[2][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED3, time_pg.bright_3[2][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED4, time_pg.bright_4[2][2]);
                                        send_tft(str_name);
                                        break;
                                case 4:
                                        printf("case PAGE_RATIO_PG3 ZONE4\n");
                                        sprintf(str_name, SET_ZONE_LED1, time_pg.bright_1[2][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED2, time_pg.bright_2[2][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED3, time_pg.bright_3[2][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, SET_ZONE_LED4, time_pg.bright_4[2][3]);
                                        send_tft(str_name);
                                        break;
                                }
                                break;
                        }

                        break;

                case SET_RATIO_PG:
                        // printf("case SET_RATIO_PG\n");

                        switch (dtmp[2])
                        {
                        case 1:
                                printf("case SET_RATIO_ZONE1\n");
                                set_bright(ZONE1_LED1, ZONE1_LED2, ZONE1_LED3, ZONE1_LED4,
                                           dtmp[3], dtmp[4], dtmp[5], dtmp[6],
                                           "SET_AUTO_ZONE1");
                                break;
                        case 2:
                                printf("case SET_RATIO_ZONE2\n");
                                set_bright(ZONE2_LED1, ZONE2_LED2, ZONE2_LED3, ZONE2_LED4,
                                           dtmp[3], dtmp[4], dtmp[5], dtmp[6],
                                           "SET_AUTO_ZONE2");
                                break;
                        case 3:
                                printf("case SET_RATIO_ZONE3\n");
                                set_bright(ZONE3_LED1, ZONE3_LED2, ZONE3_LED3, ZONE3_LED4,
                                           dtmp[3], dtmp[4], dtmp[5], dtmp[6],
                                           "SET_AUTO_ZONE3");
                                break;
                        case 4:
                                printf("case SET_RATIO_ZONE4\n");
                                set_bright(ZONE4_LED1, ZONE4_LED2, ZONE4_LED3, ZONE4_LED4,
                                           dtmp[3], dtmp[4], dtmp[5], dtmp[6],
                                           "SET_AUTO_ZONE4");
                                break;
                        }


                        break;
                case SAVE_RATIO_PG:
                        // printf("case SAVE_RATIO_PG\n");
                        switch (dtmp[1]) {
                        case 1:
                                switch (dtmp[2]) {
                                case 1:
                                        printf("case SAVE_RATIO_PG1 ZONE1\n");
                                        time_pg.bright_1[0][0]=dtmp[3];
                                        time_pg.bright_2[0][0]=dtmp[4];
                                        time_pg.bright_3[0][0]=dtmp[5];
                                        time_pg.bright_4[0][0]=dtmp[6];
                                        break;
                                case 2:
                                        printf("case SAVE_RATIO_PG1 ZONE2\n");
                                        time_pg.bright_1[0][1]=dtmp[3];
                                        time_pg.bright_2[0][1]=dtmp[4];
                                        time_pg.bright_3[0][1]=dtmp[5];
                                        time_pg.bright_4[0][1]=dtmp[6];
                                        break;
                                case 3:
                                        printf("case SAVE_RATIO_PG1 ZONE3\n");
                                        time_pg.bright_1[0][2]=dtmp[3];
                                        time_pg.bright_2[0][2]=dtmp[4];
                                        time_pg.bright_3[0][2]=dtmp[5];
                                        time_pg.bright_4[0][2]=dtmp[6];
                                        break;
                                case 4:
                                        printf("case SAVE_RATIO_PG1 ZONE4\n");
                                        time_pg.bright_1[0][3]=dtmp[3];
                                        time_pg.bright_2[0][3]=dtmp[4];
                                        time_pg.bright_3[0][3]=dtmp[5];
                                        time_pg.bright_4[0][3]=dtmp[6];
                                        break;
                                }
                                break;

                        case 2:
                                switch (dtmp[2]) {
                                case 1:
                                        printf("case SAVE_RATIO_PG2 ZONE1\n");
                                        time_pg.bright_1[1][0]=dtmp[3];
                                        time_pg.bright_2[1][0]=dtmp[4];
                                        time_pg.bright_3[1][0]=dtmp[5];
                                        time_pg.bright_4[1][0]=dtmp[6];
                                        break;
                                case 2:
                                        printf("case SAVE_RATIO_PG2 ZONE2\n");
                                        time_pg.bright_1[1][1]=dtmp[3];
                                        time_pg.bright_2[1][1]=dtmp[4];
                                        time_pg.bright_3[1][1]=dtmp[5];
                                        time_pg.bright_4[1][1]=dtmp[6];
                                        break;
                                case 3:
                                        printf("case SAVE_RATIO_PG2 ZONE3\n");
                                        time_pg.bright_1[1][2]=dtmp[3];
                                        time_pg.bright_2[1][2]=dtmp[4];
                                        time_pg.bright_3[1][2]=dtmp[5];
                                        time_pg.bright_4[1][2]=dtmp[6];
                                        break;
                                case 4:
                                        printf("case SAVE_RATIO_PG2 ZONE4\n");
                                        time_pg.bright_1[1][3]=dtmp[3];
                                        time_pg.bright_2[1][3]=dtmp[4];
                                        time_pg.bright_3[1][3]=dtmp[5];
                                        time_pg.bright_4[1][3]=dtmp[6];
                                        break;
                                }
                                break;

                        case 3:
                                switch (dtmp[2]) {
                                case 1:
                                        printf("case SAVE_RATIO_PG3 ZONE1\n");
                                        time_pg.bright_1[2][0]=dtmp[3];
                                        time_pg.bright_2[2][0]=dtmp[4];
                                        time_pg.bright_3[2][0]=dtmp[5];
                                        time_pg.bright_4[2][0]=dtmp[6];
                                        break;
                                case 2:
                                        printf("case SAVE_RATIO_PG3 ZONE2\n");
                                        time_pg.bright_1[2][1]=dtmp[3];
                                        time_pg.bright_2[2][1]=dtmp[4];
                                        time_pg.bright_3[2][1]=dtmp[5];
                                        time_pg.bright_4[2][1]=dtmp[6];
                                        break;
                                case 3:
                                        printf("case SAVE_RATIO_PG3 ZONE3\n");
                                        time_pg.bright_1[2][2]=dtmp[3];
                                        time_pg.bright_2[2][2]=dtmp[4];
                                        time_pg.bright_3[2][2]=dtmp[5];
                                        time_pg.bright_4[2][2]=dtmp[6];
                                        break;
                                case 4:
                                        printf("case SAVE_RATIO_PG3 ZONE4\n");
                                        time_pg.bright_1[2][3]=dtmp[3];
                                        time_pg.bright_2[2][3]=dtmp[4];
                                        time_pg.bright_3[2][3]=dtmp[5];
                                        time_pg.bright_4[2][3]=dtmp[6];
                                        break;
                                }
                                break;
                        }
                        save_program(time_pg);

                        check_light_on_off();

                        break;

                case PAGE_TIMG_PG:
                        printf("case PAGE_TIMG_PG\n");
                        read_program(&time_pg);
                        switch (dtmp[1]) {
                        case 1:
                                switch (dtmp[2]) {
                                case 1:
                                        printf("case PAGE_TIMG_PG1 ZONE1\n");
                                        sprintf(str_name, ON_HR, time_pg.pg_hour_on[0][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, ON_MIN, time_pg.pg_min_on[0][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_HR, time_pg.pg_hour_off[0][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_MIN, time_pg.pg_min_off[0][0]);
                                        send_tft(str_name);
                                        break;
                                case 2:
                                        printf("case PAGE_TIMG_PG1 ZONE2\n");
                                        sprintf(str_name, ON_HR, time_pg.pg_hour_on[0][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, ON_MIN, time_pg.pg_min_on[0][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_HR, time_pg.pg_hour_off[0][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_MIN, time_pg.pg_min_off[0][1]);
                                        send_tft(str_name);
                                        break;
                                case 3:
                                        printf("case PAGE_TIMG_PG1 ZONE3\n");
                                        sprintf(str_name, ON_HR, time_pg.pg_hour_on[0][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, ON_MIN, time_pg.pg_min_on[0][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_HR, time_pg.pg_hour_off[0][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_MIN, time_pg.pg_min_off[0][2]);
                                        send_tft(str_name);
                                        break;
                                case 4:
                                        printf("case PAGE_TIMG_PG1 ZONE4\n");
                                        sprintf(str_name, ON_HR, time_pg.pg_hour_on[0][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, ON_MIN, time_pg.pg_min_on[0][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_HR, time_pg.pg_hour_off[0][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_MIN, time_pg.pg_min_off[0][3]);
                                        send_tft(str_name);
                                        break;
                                }
                                break;

                        case 2:
                                switch (dtmp[2]) {
                                case 1:
                                        printf("case PAGE_TIMG_PG2 ZONE1\n");
                                        sprintf(str_name, ON_HR, time_pg.pg_hour_on[1][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, ON_MIN, time_pg.pg_min_on[1][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_HR, time_pg.pg_hour_off[1][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_MIN, time_pg.pg_min_off[1][0]);
                                        send_tft(str_name);
                                        break;
                                case 2:
                                        printf("case PAGE_TIMG_PG2 ZONE2\n");
                                        sprintf(str_name, ON_HR, time_pg.pg_hour_on[1][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, ON_MIN, time_pg.pg_min_on[1][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_HR, time_pg.pg_hour_off[1][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_MIN, time_pg.pg_min_off[1][1]);
                                        send_tft(str_name);
                                        break;
                                case 3:
                                        printf("case PAGE_TIMG_PG2 ZONE3\n");
                                        sprintf(str_name, ON_HR, time_pg.pg_hour_on[1][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, ON_MIN, time_pg.pg_min_on[1][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_HR, time_pg.pg_hour_off[1][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_MIN, time_pg.pg_min_off[1][2]);
                                        send_tft(str_name);

                                        break;
                                case 4:
                                        printf("case PAGE_TIMG_PG2 ZONE4\n");
                                        sprintf(str_name, ON_HR, time_pg.pg_hour_on[1][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, ON_MIN, time_pg.pg_min_on[1][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_HR, time_pg.pg_hour_off[1][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_MIN, time_pg.pg_min_off[1][3]);
                                        send_tft(str_name);
                                        break;
                                }
                                break;

                        case 3:
                                switch (dtmp[2]) {
                                case 1:
                                        printf("case PAGE_TIMG_PG3 ZONE1\n");
                                        sprintf(str_name, ON_HR, time_pg.pg_hour_on[2][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, ON_MIN, time_pg.pg_min_on[2][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_HR, time_pg.pg_hour_off[2][0]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_MIN, time_pg.pg_min_off[2][0]);
                                        send_tft(str_name);
                                        break;
                                case 2:
                                        printf("case PAGE_TIMG_PG3 ZONE2\n");
                                        sprintf(str_name, ON_HR, time_pg.pg_hour_on[2][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, ON_MIN, time_pg.pg_min_on[2][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_HR, time_pg.pg_hour_off[2][1]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_MIN, time_pg.pg_min_off[2][1]);
                                        send_tft(str_name);
                                        break;
                                case 3:
                                        printf("case PAGE_TIMG_PG3 ZONE3\n");
                                        sprintf(str_name, ON_HR, time_pg.pg_hour_on[2][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, ON_MIN, time_pg.pg_min_on[2][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_HR, time_pg.pg_hour_off[2][2]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_MIN, time_pg.pg_min_off[2][2]);
                                        send_tft(str_name);
                                        break;
                                case 4:
                                        printf("case PAGE_TIMG_PG3 ZONE4\n");
                                        sprintf(str_name, ON_HR, time_pg.pg_hour_on[2][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, ON_MIN, time_pg.pg_min_on[2][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_HR, time_pg.pg_hour_off[2][3]);
                                        send_tft(str_name);
                                        sprintf(str_name, OFF_MIN, time_pg.pg_min_off[2][3]);
                                        send_tft(str_name);
                                        break;
                                }
                                break;
                        }
                        break;

                case SAVE_TIMG_PG:

                        switch (dtmp[1]) {
                        case 1:
                                switch (dtmp[2]) {
                                case 1:
                                        printf("case SAVE_TIMG_PG1 ZONE1\n");
                                        time_pg.pg_hour_on[0][0]=dtmp[3];
                                        time_pg.pg_min_on[0][0]=dtmp[4];
                                        time_pg.pg_hour_off[0][0]=dtmp[5];
                                        time_pg.pg_min_off[0][0]=dtmp[6];
                                        break;
                                case 2:
                                        printf("case SAVE_TIMG_PG1 ZONE2\n");
                                        time_pg.pg_hour_on[0][1]=dtmp[3];
                                        time_pg.pg_min_on[0][1]=dtmp[4];
                                        time_pg.pg_hour_off[0][1]=dtmp[5];
                                        time_pg.pg_min_off[0][1]=dtmp[6];
                                        break;
                                case 3:
                                        printf("case SAVE_TIMG_PG1 ZONE3\n");
                                        time_pg.pg_hour_on[0][2]=dtmp[3];
                                        time_pg.pg_min_on[0][2]=dtmp[4];
                                        time_pg.pg_hour_off[0][2]=dtmp[5];
                                        time_pg.pg_min_off[0][2]=dtmp[6];
                                        break;
                                case 4:
                                        printf("case SAVE_TIMG_PG1 ZONE4\n");
                                        time_pg.pg_hour_on[0][3]=dtmp[3];
                                        time_pg.pg_min_on[0][3]=dtmp[4];
                                        time_pg.pg_hour_off[0][3]=dtmp[5];
                                        time_pg.pg_min_off[0][3]=dtmp[6];
                                        break;
                                }
                                break;

                        case 2:
                                switch (dtmp[2]) {
                                case 1:
                                        printf("case SAVE_TIMG_PG2 ZONE1\n");
                                        time_pg.pg_hour_on[1][0]=dtmp[3];
                                        time_pg.pg_min_on[1][0]=dtmp[4];
                                        time_pg.pg_hour_off[1][0]=dtmp[5];
                                        time_pg.pg_min_off[1][0]=dtmp[6];
                                        break;
                                case 2:
                                        printf("case SAVE_TIMG_PG2 ZONE2\n");
                                        time_pg.pg_hour_on[1][1]=dtmp[3];
                                        time_pg.pg_min_on[1][1]=dtmp[4];
                                        time_pg.pg_hour_off[1][1]=dtmp[5];
                                        time_pg.pg_min_off[1][1]=dtmp[6];
                                        break;
                                case 3:
                                        printf("case SAVE_TIMG_PG2 ZONE3\n");
                                        time_pg.pg_hour_on[1][2]=dtmp[3];
                                        time_pg.pg_min_on[1][2]=dtmp[4];
                                        time_pg.pg_hour_off[1][2]=dtmp[5];
                                        time_pg.pg_min_off[1][2]=dtmp[6];

                                        break;
                                case 4:
                                        printf("case SAVE_TIMG_PG2 ZONE4\n");
                                        time_pg.pg_hour_on[1][3]=dtmp[3];
                                        time_pg.pg_min_on[1][3]=dtmp[4];
                                        time_pg.pg_hour_off[1][3]=dtmp[5];
                                        time_pg.pg_min_off[1][3]=dtmp[6];
                                        break;
                                }
                                break;

                        case 3:
                                switch (dtmp[2]) {
                                case 1:
                                        printf("case SAVE_TIMG_PG3 ZONE1\n");
                                        time_pg.pg_hour_on[2][0]=dtmp[3];
                                        time_pg.pg_min_on[2][0]=dtmp[4];
                                        time_pg.pg_hour_off[2][0]=dtmp[5];
                                        time_pg.pg_min_off[2][0]=dtmp[6];
                                        break;
                                case 2:
                                        printf("case SAVE_TIMG_PG3 ZONE2\n");
                                        time_pg.pg_hour_on[2][1]=dtmp[3];
                                        time_pg.pg_min_on[2][1]=dtmp[4];
                                        time_pg.pg_hour_off[2][1]=dtmp[5];
                                        time_pg.pg_min_off[2][1]=dtmp[6];
                                        break;
                                case 3:
                                        printf("case SAVE_TIMG_PG3 ZONE3\n");
                                        time_pg.pg_hour_on[2][2]=dtmp[3];
                                        time_pg.pg_min_on[2][2]=dtmp[4];
                                        time_pg.pg_hour_off[2][2]=dtmp[5];
                                        time_pg.pg_min_off[2][2]=dtmp[6];
                                        break;
                                case 4:
                                        printf("case SAVE_TIMG_PG3 ZONE4\n");
                                        time_pg.pg_hour_on[2][3]=dtmp[3];
                                        time_pg.pg_min_on[2][3]=dtmp[4];
                                        time_pg.pg_hour_off[2][3]=dtmp[5];
                                        time_pg.pg_min_off[2][3]=dtmp[6];
                                        break;
                                }
                                break;
                        }
                        save_program(time_pg);

                        check_light_on_off();

                        break;

                case ONOFF_PROGRAM:
                        printf("case ONOFF_PROGRAM\n");

                        printf("Program STAGE ALL ON\n");
                        readValue(&start_time);
                        save_time(&start_time);
                        if (dtmp[2] > 0)
                        {

                                uint16_t val = dtmp[2] * 256 + dtmp[1];
                                status_pg.start_day = val;
                        }
                        else
                                status_pg.start_day = dtmp[1];
                        save_statuspg(status_pg);

                        check_light_on_off();

                        break;

                case STATUS_PROGRAM:
                        printf("case STATUS_PROGRAM\n");
                        switch (dtmp[1])
                        {
                        case 0:

                                printf("Light is Manual\n");
                                status_pg.switch_mode = dtmp[1];
                                save_statuspg(status_pg);

                                check_light_on_off();

                                set_bright(ZONE1_LED1, ZONE1_LED2, ZONE1_LED3, ZONE1_LED4,
                                           ratio_led.bright_1[0],
                                           ratio_led.bright_2[0],
                                           ratio_led.bright_3[0],
                                           ratio_led.bright_4[0],
                                           "LOAD_MANUAL_ZONE1");

                                set_bright(ZONE2_LED1, ZONE2_LED2, ZONE2_LED3, ZONE2_LED4,
                                           ratio_led.bright_1[1],
                                           ratio_led.bright_2[1],
                                           ratio_led.bright_3[1],
                                           ratio_led.bright_4[1],
                                           "LOAD_MANUAL_ZONE2");

                                set_bright(ZONE3_LED1, ZONE3_LED2, ZONE3_LED3, ZONE3_LED4,
                                           ratio_led.bright_1[2],
                                           ratio_led.bright_2[2],
                                           ratio_led.bright_3[2],
                                           ratio_led.bright_4[2],
                                           "LOAD_MANUAL_ZONE3");

                                set_bright(ZONE4_LED1, ZONE4_LED2, ZONE4_LED3, ZONE4_LED4,
                                           ratio_led.bright_1[3],
                                           ratio_led.bright_2[3],
                                           ratio_led.bright_3[3],
                                           ratio_led.bright_4[3],
                                           "LOAD_MANUAL_ZONE4");

                                break;

                        case 1:

                                printf("Light is Auto\n");
                                status_pg.switch_mode = dtmp[1];
                                save_statuspg(status_pg);

                                check_light_on_off();

                                break;
                        }

                        break;

                case PAGE_FERTILIZER:
                        printf("case PAGE_FERTILIZER\n");

                        sprintf(str_name, fer_LOOPTIME_, working_timer.working_day);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMERON_H, working_timer.working_on_h[0]);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMERON_M, working_timer.working_on_m[0]);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMEROFF_H, working_timer.working_off_h[0]);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMEROFF_M, working_timer.working_off_m[0]);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMERON_H2, working_timer.working_on_h[1]);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMERON_M2, working_timer.working_on_m[1]);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMEROFF_H2, working_timer.working_off_h[1]);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMEROFF_M2, working_timer.working_off_m[1]);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMERON_H3, working_timer.working_on_h[2]);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMERON_M3, working_timer.working_on_m[2]);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMEROFF_H3, working_timer.working_off_h[2]);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMEROFF_M3, working_timer.working_off_m[2]);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMERON_H4, working_timer.working_on_h[3]);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMERON_M4, working_timer.working_on_m[3]);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMEROFF_H4, working_timer.working_off_h[3]);
                        send_tft(str_name);
                        sprintf(str_name, fer_TIMEROFF_M4, working_timer.working_off_m[3]);
                        send_tft(str_name);

                        if(working_timer.status_timer[0] == 1)
                        {
                                sprintf(str_name, FER_TPUMP1,18);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPUMP1,1);
                                send_tft(str_name);
                        }
                        else
                        {
                                sprintf(str_name, FER_TPUMP1,17);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPUMP1,0);
                                send_tft(str_name);
                        }
                        if(working_timer.status_timer[1] == 1)
                        {
                                sprintf(str_name, FER_TPUMP2,18);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPUMP2,1);
                                send_tft(str_name);
                        }
                        else
                        {
                                sprintf(str_name, FER_TPUMP2,17);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPUMP2,0);
                                send_tft(str_name);
                        }
                        if(working_timer.status_timer[2] == 1)
                        {
                                sprintf(str_name, FER_TPUMP3,18);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPUMP3,1);
                                send_tft(str_name);
                        }
                        else
                        {
                                sprintf(str_name, FER_TPUMP3,17);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPUMP3,0);
                                send_tft(str_name);
                        }
                        if(working_timer.status_timer[3] == 1)
                        {
                                sprintf(str_name, FER_TPUMP4,18);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPUMP4,1);
                                send_tft(str_name);
                        }
                        else
                        {
                                sprintf(str_name, FER_TPUMP4,17);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPUMP4,0);
                                send_tft(str_name);
                        }

                        sprintf(str_name, fill_TIMERON_H, working_timer.fill_working_on_h[0]);
                        send_tft(str_name);
                        sprintf(str_name, fill_TIMERON_M, working_timer.fill_working_on_m[0]);
                        send_tft(str_name);
                        sprintf(str_name, fill_TIMEROFF_H, working_timer.fill_working_off_h[0]);
                        send_tft(str_name);
                        sprintf(str_name, fill_TIMEROFF_M, working_timer.fill_working_off_m[0]);
                        send_tft(str_name);
                        sprintf(str_name, fill_TIMERON_H2, working_timer.fill_working_on_h[1]);
                        send_tft(str_name);
                        sprintf(str_name, fill_TIMERON_M2, working_timer.fill_working_on_m[1]);
                        send_tft(str_name);
                        sprintf(str_name, fill_TIMEROFF_H2, working_timer.fill_working_off_h[1]);
                        send_tft(str_name);
                        sprintf(str_name, fill_TIMEROFF_M2, working_timer.fill_working_off_m[1]);
                        send_tft(str_name);
                        sprintf(str_name, fill_TIMERON_H3, working_timer.fill_working_on_h[2]);
                        send_tft(str_name);
                        sprintf(str_name, fill_TIMERON_M3, working_timer.fill_working_on_m[2]);
                        send_tft(str_name);
                        sprintf(str_name, fill_TIMEROFF_H3, working_timer.fill_working_off_h[2]);
                        send_tft(str_name);
                        sprintf(str_name, fill_TIMEROFF_M3, working_timer.fill_working_off_m[2]);
                        send_tft(str_name);
                        sprintf(str_name, fill_TIMERON_H4, working_timer.fill_working_on_h[3]);
                        send_tft(str_name);
                        sprintf(str_name, fill_TIMERON_M4, working_timer.fill_working_on_m[3]);
                        send_tft(str_name);
                        sprintf(str_name, fill_TIMEROFF_H4, working_timer.fill_working_off_h[3]);
                        send_tft(str_name);
                        sprintf(str_name, fill_TIMEROFF_M4, working_timer.fill_working_off_m[3]);
                        send_tft(str_name);

                        if(working_timer.fill_status_timer[0] == 1)
                        {
                                sprintf(str_name, FER_TFILL1,18);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TFILL1,1);
                                send_tft(str_name);
                        }
                        else
                        {
                                sprintf(str_name, FER_TPUMP1,17);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TFILL1,0);
                                send_tft(str_name);
                        }
                        if(working_timer.fill_status_timer[1] == 1)
                        {
                                sprintf(str_name, FER_TFILL2,18);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TFILL2,1);
                                send_tft(str_name);
                        }
                        else
                        {
                                sprintf(str_name, FER_TFILL2,17);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TFILL2,0);
                                send_tft(str_name);
                        }
                        if(working_timer.fill_status_timer[2] == 1)
                        {
                                sprintf(str_name, FER_TFILL3,18);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TFILL3,1);
                                send_tft(str_name);
                        }
                        else
                        {
                                sprintf(str_name, FER_TFILL3,17);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TFILL3,0);
                                send_tft(str_name);
                        }
                        if(working_timer.fill_status_timer[3] == 1)
                        {
                                sprintf(str_name, FER_TFILL4,18);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TFILL4,1);
                                send_tft(str_name);
                        }
                        else
                        {
                                sprintf(str_name, FER_TFILL4,17);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TFILL4,0);
                                send_tft(str_name);
                        }

                        sprintf(str_name, SET_EC_POINT, ferti_set_val.ec_set_point);
                        send_tft(str_name);
                        sprintf(str_name, SET_PH_POINT, ferti_set_val.ph_set_point);
                        send_tft(str_name);
                        sprintf(str_name, SET_RATIO_A, ferti_set_val.ratio_fer[0]);
                        send_tft(str_name);
                        sprintf(str_name, SET_RATIO_B, ferti_set_val.ratio_fer[1]);
                        send_tft(str_name);
                        sprintf(str_name, SET_RATIO_C, ferti_set_val.ratio_fer[2]);
                        send_tft(str_name);
                        sprintf(str_name, SET_RATIO_D, ferti_set_val.ratio_fer[3]);
                        send_tft(str_name);

                        sprintf(str_name, SET_RATIO_PH, ferti_set_val.ratio_ph);
                        send_tft(str_name);
                        sprintf(str_name, WAIT_EC_, ferti_set_val.wait_ec);
                        send_tft(str_name);
                        sprintf(str_name, WAIT_PH_, ferti_set_val.wait_ph);
                        send_tft(str_name);

                        sprintf(str_name, ph_TIMERON_H, ferti_set_val.ph_working_on_h[0]);
                        send_tft(str_name);
                        sprintf(str_name, ph_TIMERON_M, ferti_set_val.ph_working_on_m[0]);
                        send_tft(str_name);
                        sprintf(str_name, ph_TIMEROFF_H, ferti_set_val.ph_working_off_h[0]);
                        send_tft(str_name);
                        sprintf(str_name, ph_TIMEROFF_M, ferti_set_val.ph_working_off_m[0]);
                        send_tft(str_name);
                        sprintf(str_name, ph_TIMERON_H2, ferti_set_val.ph_working_on_h[1]);
                        send_tft(str_name);
                        sprintf(str_name, ph_TIMERON_M2, ferti_set_val.ph_working_on_m[1]);
                        send_tft(str_name);
                        sprintf(str_name, ph_TIMEROFF_H2, ferti_set_val.ph_working_off_h[1]);
                        send_tft(str_name);
                        sprintf(str_name, ph_TIMEROFF_M2, ferti_set_val.ph_working_off_m[1]);
                        send_tft(str_name);
                        sprintf(str_name, ph_TIMERON_H3, ferti_set_val.ph_working_on_h[2]);
                        send_tft(str_name);
                        sprintf(str_name, ph_TIMERON_M3, ferti_set_val.ph_working_on_m[2]);
                        send_tft(str_name);
                        sprintf(str_name, ph_TIMEROFF_H3, ferti_set_val.ph_working_off_h[2]);
                        send_tft(str_name);
                        sprintf(str_name, ph_TIMEROFF_M3, ferti_set_val.ph_working_off_m[2]);
                        send_tft(str_name);
                        sprintf(str_name, ph_TIMERON_H4, ferti_set_val.ph_working_on_h[3]);
                        send_tft(str_name);
                        sprintf(str_name, ph_TIMERON_M4, ferti_set_val.ph_working_on_m[3]);
                        send_tft(str_name);
                        sprintf(str_name, ph_TIMEROFF_H4, ferti_set_val.ph_working_off_h[3]);
                        send_tft(str_name);
                        sprintf(str_name, ph_TIMEROFF_M4, ferti_set_val.ph_working_off_m[3]);
                        send_tft(str_name);

                        if(ferti_set_val.ph_status_timer[0] == 1)
                        {
                                sprintf(str_name, FER_TPH1,18);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPH1,1);
                                send_tft(str_name);
                        }
                        else
                        {
                                sprintf(str_name, FER_TPH1,17);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPH1,0);
                                send_tft(str_name);
                        }
                        if(ferti_set_val.ph_status_timer[1] == 1)
                        {
                                sprintf(str_name, FER_TPH2,18);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPH2,1);
                                send_tft(str_name);
                        }
                        else
                        {
                                sprintf(str_name, FER_TPH2,17);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPH2,0);
                                send_tft(str_name);
                        }
                        if(ferti_set_val.ph_status_timer[2] == 1)
                        {
                                sprintf(str_name, FER_TPH3,18);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPH3,1);
                                send_tft(str_name);
                        }
                        else
                        {
                                sprintf(str_name, FER_TPH3,17);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPH3,0);
                                send_tft(str_name);
                        }
                        if(ferti_set_val.ph_status_timer[3] == 1)
                        {
                                sprintf(str_name, FER_TPH4,18);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPH4,1);
                                send_tft(str_name);
                        }
                        else
                        {
                                sprintf(str_name, FER_TPH4,17);
                                send_tft(str_name);
                                sprintf(str_name, SET_STATUS_TPH4,0);
                                send_tft(str_name);
                        }

                        break;

                case SET_FERTILIZER:
                        printf("case SET_FERTILIZER\n");

                        ferti_set_val.ec_set_point = dtmp[2];
                        ferti_set_val.ph_set_point = dtmp[6];

                        ferti_set_val.ratio_fer[0] = dtmp[10];
                        ferti_set_val.ratio_fer[1] = dtmp[11];
                        ferti_set_val.ratio_fer[2] = dtmp[12];
                        ferti_set_val.ratio_fer[3] = dtmp[13];

                        ferti_set_val.ratio_ph = dtmp[14];
                        ferti_set_val.wait_ec = dtmp[15];
                        ferti_set_val.wait_ph = dtmp[16];

                        ferti_set_val.ph_working_on_h[0] = dtmp[17];
                        ferti_set_val.ph_working_on_m[0] = dtmp[18];
                        ferti_set_val.ph_working_off_h[0] = dtmp[19];
                        ferti_set_val.ph_working_off_m[0] = dtmp[20];

                        ferti_set_val.ph_working_on_h[1] = dtmp[21];
                        ferti_set_val.ph_working_on_m[1] = dtmp[22];
                        ferti_set_val.ph_working_off_h[1] = dtmp[23];
                        ferti_set_val.ph_working_off_m[1] = dtmp[24];

                        ferti_set_val.ph_working_on_h[2] = dtmp[25];
                        ferti_set_val.ph_working_on_m[2] = dtmp[26];
                        ferti_set_val.ph_working_off_h[2] = dtmp[27];
                        ferti_set_val.ph_working_off_m[2] = dtmp[28];

                        ferti_set_val.ph_working_on_h[3] = dtmp[29];
                        ferti_set_val.ph_working_on_m[3] = dtmp[30];
                        ferti_set_val.ph_working_off_h[3] = dtmp[31];
                        ferti_set_val.ph_working_off_m[3] = dtmp[32];

                        ferti_set_val.ph_status_timer[0] = dtmp[33];
                        ferti_set_val.ph_status_timer[1] = dtmp[34];
                        ferti_set_val.ph_status_timer[2] = dtmp[35];
                        ferti_set_val.ph_status_timer[3] = dtmp[36];

                        save_ferti(ferti_set_val);
                        start_dosing_ec = true;
                        start_dosing_ph = true;
                        expression_task = fer_ok;

                        break;

                case SET_PUMPCONTROL:
                        printf("case SET_PUMPCONTROL\n");
                        working_timer.working_day = dtmp[1];

                        working_timer.working_on_h[0] = dtmp[2];
                        working_timer.working_on_m[0] = dtmp[3];
                        working_timer.working_off_h[0] = dtmp[4];
                        working_timer.working_off_m[0] = dtmp[5];

                        working_timer.working_on_h[1] = dtmp[6];
                        working_timer.working_on_m[1] = dtmp[7];
                        working_timer.working_off_h[1] = dtmp[8];
                        working_timer.working_off_m[1] = dtmp[9];

                        working_timer.working_on_h[2] = dtmp[10];
                        working_timer.working_on_m[2] = dtmp[11];
                        working_timer.working_off_h[2] = dtmp[12];
                        working_timer.working_off_m[2] = dtmp[13];

                        working_timer.working_on_h[3] = dtmp[14];
                        working_timer.working_on_m[3] = dtmp[15];
                        working_timer.working_off_h[3] = dtmp[16];
                        working_timer.working_off_m[3] = dtmp[17];

                        working_timer.status_timer[0] = dtmp[18];
                        working_timer.status_timer[1] = dtmp[19];
                        working_timer.status_timer[2] = dtmp[20];
                        working_timer.status_timer[3] = dtmp[21];

                        working_timer.working_nextday = 0;
                        working_timer.working_lastday = 0;
                        save_working(working_timer);

                        readValue(&start_time2);
                        save_time_pump(&start_time2);

                        break;

                case SET_FILL_TIME:
                        printf("case SET_FILL_TIME\n");

                        working_timer.fill_working_on_h[0] = dtmp[1];
                        working_timer.fill_working_on_m[0] = dtmp[2];
                        working_timer.fill_working_off_h[0] = dtmp[3];
                        working_timer.fill_working_off_m[0] = dtmp[4];

                        working_timer.fill_working_on_h[1] = dtmp[5];
                        working_timer.fill_working_on_m[1] = dtmp[6];
                        working_timer.fill_working_off_h[1] = dtmp[7];
                        working_timer.fill_working_off_m[1] = dtmp[8];

                        working_timer.fill_working_on_h[2] = dtmp[9];
                        working_timer.fill_working_on_m[2] = dtmp[10];
                        working_timer.fill_working_off_h[2] = dtmp[11];
                        working_timer.fill_working_off_m[2] = dtmp[12];

                        working_timer.fill_working_on_h[3] = dtmp[13];
                        working_timer.fill_working_on_m[3] = dtmp[14];
                        working_timer.fill_working_off_h[3] = dtmp[15];
                        working_timer.fill_working_off_m[3] = dtmp[16];

                        working_timer.fill_status_timer[0] = dtmp[17];
                        working_timer.fill_status_timer[1] = dtmp[18];
                        working_timer.fill_status_timer[2] = dtmp[19];
                        working_timer.fill_status_timer[3] = dtmp[20];

                        save_working(working_timer);

                        break;

                case PAGE_ENVIRONMENT:
                        printf("case PAGE_ENVIRONMENT\n");
                        read_environment(&_environment);

                        switch (_environment.fill1_state) {
                        case MODE_OFF:
                                sprintf(str_name,FILL_STATE_A,41);
                                send_tft(str_name);
                                break;
                        case MODE_ON:
                                sprintf(str_name,FILL_STATE_A,40);
                                send_tft(str_name);
                                break;
                        case MODE_AUTO:
                                sprintf(str_name,FILL_STATE_A,42);
                                send_tft(str_name);
                                break;
                        }

                        switch (_environment.fill2_state) {
                        case MODE_OFF:
                                sprintf(str_name,FILL_STATE_B,41);
                                send_tft(str_name);
                                break;
                        case MODE_ON:
                                sprintf(str_name,FILL_STATE_B,40);
                                send_tft(str_name);
                                break;
                        case MODE_AUTO:
                                sprintf(str_name,FILL_STATE_B,42);
                                send_tft(str_name);
                                break;
                        }

                        switch (_environment.fill3_state) {
                        case MODE_OFF:
                                sprintf(str_name,FILL_STATE_C,41);
                                send_tft(str_name);
                                break;
                        case MODE_ON:
                                sprintf(str_name,FILL_STATE_C,40);
                                send_tft(str_name);
                                break;
                        case MODE_AUTO:
                                sprintf(str_name,FILL_STATE_C,42);
                                send_tft(str_name);
                                break;
                        }

                        switch (_environment.fill4_state) {
                        case MODE_OFF:
                                sprintf(str_name,FILL_STATE_D,41);
                                send_tft(str_name);
                                break;
                        case MODE_ON:
                                sprintf(str_name,FILL_STATE_D,40);
                                send_tft(str_name);
                                break;
                        case MODE_AUTO:
                                sprintf(str_name,FILL_STATE_D,42);
                                send_tft(str_name);
                                break;
                        }

                        switch (_environment.pump_ph_state) {
                        case MODE_OFF:
                                sprintf(str_name,PUMP_STATE_PH,68);
                                send_tft(str_name);
                                break;
                        case MODE_ON:
                                sprintf(str_name,PUMP_STATE_PH,65);
                                send_tft(str_name);
                                break;
                        case MODE_AUTO:
                                sprintf(str_name,PUMP_STATE_PH,66);
                                send_tft(str_name);
                                break;
                        case MODE_TIME:
                                sprintf(str_name,PUMP_STATE_PH,67);
                                send_tft(str_name);
                                break;
                        }

                        switch (_environment.solenoide_state) {
                        case MODE_OFF:
                                sprintf(str_name,SOLENOIDE_STATE,68);
                                send_tft(str_name);
                                break;
                        case MODE_ON:
                                sprintf(str_name,SOLENOIDE_STATE,65);
                                send_tft(str_name);
                                break;
                        case MODE_AUTO:
                                sprintf(str_name,SOLENOIDE_STATE,66);
                                send_tft(str_name);
                                break;
                        case MODE_TIME:
                                sprintf(str_name,SOLENOIDE_STATE,67);
                                send_tft(str_name);
                                break;
                        }

                        switch (_environment.pump_water_state) {
                        case MODE_OFF:
                                sprintf(str_name,PUMP_WATER_STATE,41);
                                send_tft(str_name);
                                break;
                        case MODE_ON:
                                sprintf(str_name,PUMP_WATER_STATE,40);
                                send_tft(str_name);
                                break;
                        case MODE_AUTO:
                                sprintf(str_name,PUMP_WATER_STATE,42);
                                send_tft(str_name);
                                break;
                        }

                        break;

                case SET_ENVIRONMENT:
                        printf("case SET_ENVIRONMENT\n");
                        switch (dtmp[1]) {
                        case 1:
                                printf("PUMP_FILL_1 (A)\n");
                                switch (dtmp[2]) {
                                case MODE_OFF:
                                        printf("PUMP_FILL_1 (A) to mode OFF\n");
                                        SETFILL(PUMP_A,false,"PUMP_A_OFF");
                                        break;
                                case MODE_ON:
                                        printf("PUMP_FILL_1 (A) to mode ON\n");
                                        SETFILL(PUMP_A,true,"PUMP_A_ON");
                                        break;
                                case MODE_AUTO:
                                        printf("PUMP_FILL_1 (A) to mode AUTO\n");
                                        SETFILL(PUMP_A,false,"PUMP_A_OFF");
                                        expression_task = fer_ok;
                                        start_dosing_ec = true;
                                        break;
                                }
                                _environment.fill1_state=dtmp[2];
                                break;
                        case 2:
                                switch (dtmp[2]) {
                                case MODE_OFF:
                                        printf("PUMP_FILL_2 (B) to mode OFF\n");
                                        SETFILL(PUMP_B,false,"PUMP_B_OFF");
                                        break;
                                case MODE_ON:
                                        printf("PUMP_FILL_2 (B) to mode ON\n");
                                        SETFILL(PUMP_B,true,"PUMP_B_ON");
                                        break;
                                case MODE_AUTO:
                                        printf("PUMP_FILL_2 (B) to mode AUTO\n");
                                        SETFILL(PUMP_B,false,"PUMP_B_OFF");
                                        expression_task = fer_ok;
                                        start_dosing_ec = true;
                                        break;
                                }
                                _environment.fill2_state=dtmp[2];
                                break;
                        case 3:
                                switch (dtmp[2]) {
                                case MODE_OFF:
                                        printf("PUMP_FILL_3 (C) to mode OFF\n");
                                        SETFILL(PUMP_C,false,"PUMP_C_OFF");
                                        break;
                                case MODE_ON:
                                        printf("PUMP_FILL_3 (C) to mode ON\n");
                                        SETFILL(PUMP_C,true,"PUMP_C_ON");
                                        break;
                                case MODE_AUTO:
                                        printf("PUMP_FILL_3 (C) to mode AUTO\n");
                                        SETFILL(PUMP_C,false,"PUMP_C_OFF");
                                        expression_task = fer_ok;
                                        start_dosing_ec = true;
                                        break;
                                }
                                _environment.fill3_state=dtmp[2];
                                break;
                        case 4:
                                switch (dtmp[2]) {
                                case MODE_OFF:
                                        printf("PUMP_FILL_4 (D) to mode OFF\n");
                                        SETFILL(PUMP_D,false,"PUMP_D_OFF");
                                        break;
                                case MODE_ON:
                                        printf("PUMP_FILL_4 (D) to mode ON\n");
                                        SETFILL(PUMP_D,true,"PUMP_D_ON");
                                        break;
                                case MODE_AUTO:
                                        printf("PUMP_FILL_4 (D) to mode AUTO\n");
                                        SETFILL(PUMP_D,false,"PUMP_D_OFF");
                                        expression_task = fer_ok;
                                        start_dosing_ec = true;
                                        break;
                                }
                                _environment.fill4_state=dtmp[2];
                                break;
                        case 5:
                                switch (dtmp[2]) {
                                case MODE_OFF:
                                        printf("PUMP_FILL_PH to mode OFF\n");
                                        SETFILL(PUMP_PH,false,"PUMP_PH_OFF");
                                        break;
                                case MODE_ON:
                                        printf("PUMP_FILL_PH to mode ON\n");
                                        SETFILL(PUMP_PH,true,"PUMP_PH_ON");
                                        break;
                                case MODE_AUTO:
                                        printf("PUMP_FILL_PH to mode AUTO\n");
                                        SETFILL(PUMP_PH,false,"PUMP_PH_OFF");
                                        expression_task = fer_ok;
                                        start_dosing_ph = true;
                                        break;
                                case MODE_TIME:
                                        printf("PUMP_FILL_PH to mode TIME\n");
                                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                                        expression_task = fer_ok;
                                        start_dosing_ph = true;
                                        break;
                                }
                                _environment.pump_ph_state=dtmp[2];
                                break;
                        case 6:
                                switch (dtmp[2])
                                {
                                case MODE_OFF:
                                        printf("SOLENOIDE to mode OFF\n");
                                        SETFILL(SOLENOIDE, false,"SOLENOIDE_OFF");
                                        break;
                                case MODE_ON:
                                        printf("SOLENOIDE to mode ON\n");
                                        SETFILL(SOLENOIDE, true,"SOLENOIDE_ON");
                                        break;
                                case MODE_AUTO:
                                        printf("SOLENOIDE to mode AUTO\n");
                                        SETFILL(SOLENOIDE, false,"SOLENOIDE_OFF");
                                        break;
                                case MODE_TIME:
                                        printf("SOLENOIDE to mode TIME\n");
                                        SETFILL(SOLENOIDE, false,"SOLENOIDE_OFF");
                                        break;
                                }
                                _environment.solenoide_state = dtmp[2];
                                break;
                        case 7:
                                switch (dtmp[2])
                                {
                                case MODE_OFF:
                                        printf("PUMP_WATER to mode OFF\n");
                                        SETFILL(ON_OFF_P1, false,"PUMP_WATER_OFF");
                                        break;
                                case MODE_ON:
                                        printf("PUMP_WATER to mode ON\n");
                                        SETFILL(ON_OFF_P1, true,"PUMP_WATER_ON");
                                        break;
                                case MODE_AUTO:
                                        printf("PUMP_WATER to mode AUTO\n");
                                        SETFILL(ON_OFF_P1, false,"PUMP_WATER_OFF");
                                        break;
                                }
                                _environment.pump_water_state = dtmp[2];
                                break;
                        }
                        save_environment(_environment);
                        break;

                case PAGE_DUTYCYCLE:
                        // printf("case PAGE_DUTYCYCLE\n");
                        switch (dtmp[1]) {
                        case 1:
                                printf("case PAGE_DUTYCYCLE ferA\n");
                                sprintf(str_name,TFT_SET_DUTY,_environment.fill1_duty);
                                send_tft(str_name);
                                break;
                        case 2:
                                printf("case PAGE_DUTYCYCLE ferB\n");
                                sprintf(str_name,TFT_SET_DUTY,_environment.fill2_duty);
                                send_tft(str_name);
                                break;
                        case 3:
                                printf("case PAGE_DUTYCYCLE ferC\n");
                                sprintf(str_name,TFT_SET_DUTY,_environment.fill3_duty);
                                send_tft(str_name);
                                break;
                        case 4:
                                printf("case PAGE_DUTYCYCLE ferD\n");
                                sprintf(str_name,TFT_SET_DUTY,_environment.fill4_duty);
                                send_tft(str_name);
                                break;
                        case 5:
                                printf("case PAGE_DUTYCYCLE pH\n");
                                sprintf(str_name,TFT_SET_DUTY,_environment.pump_ph_duty);
                                send_tft(str_name);
                                break;
                        }
                        break;

                case SET_DUTYCYCLE:
                        // printf("case SET_DUTYCYCLE\n");
                        switch (dtmp[1]) {
                        case 1:
                                printf("case SET_DUTYCYCLE ferA\n");
                                _environment.fill1_duty = dtmp[2];

                                switch (_environment.fill1_state) {
                                case MODE_OFF:
                                        printf("PUMP_FILL_A to mode OFF\n");
                                        SETFILL(PUMP_A,false,"PUMP_A_OFF");
                                        break;
                                case MODE_ON:
                                        printf("PUMP_FILL_A to mode ON\n");
                                        SETFILL(PUMP_A,true,"PUMP_A_ON");
                                        break;
                                case MODE_AUTO:
                                        printf("PUMP_FILL_A to mode AUTO\n");
                                        SETFILL(PUMP_A,false,"PUMP_A_OFF");
                                        expression_task = fer_ok;
                                        start_dosing_ec = true;
                                        break;
                                }

                                break;
                        case 2:
                                printf("case SET_DUTYCYCLE ferB\n");
                                _environment.fill2_duty = dtmp[2];

                                switch (_environment.fill2_state) {
                                case MODE_OFF:
                                        printf("PUMP_FILL_B to mode OFF\n");
                                        SETFILL(PUMP_B,false,"PUMP_B_OFF");
                                        break;
                                case MODE_ON:
                                        printf("PUMP_FILL_B to mode ON\n");
                                        SETFILL(PUMP_B,true,"PUMP_B_ON");
                                        break;
                                case MODE_AUTO:
                                        printf("PUMP_FILL_B to mode AUTO\n");
                                        SETFILL(PUMP_B,false,"PUMP_B_OFF");
                                        expression_task = fer_ok;
                                        start_dosing_ec = true;
                                        break;
                                }

                                break;
                        case 3:
                                printf("case SET_DUTYCYCLE ferC\n");
                                _environment.fill3_duty = dtmp[2];

                                switch (_environment.fill3_state) {
                                case MODE_OFF:
                                        printf("PUMP_FILL_C to mode OFF\n");
                                        SETFILL(PUMP_C,false,"PUMP_C_OFF");
                                        break;
                                case MODE_ON:
                                        printf("PUMP_FILL_C to mode ON\n");
                                        SETFILL(PUMP_C,true,"PUMP_C_ON");
                                        break;
                                case MODE_AUTO:
                                        printf("PUMP_FILL_C to mode AUTO\n");
                                        SETFILL(PUMP_C,false,"PUMP_C_OFF");
                                        expression_task = fer_ok;
                                        start_dosing_ec = true;
                                        break;
                                }

                                break;
                        case 4:
                                printf("case SET_DUTYCYCLE ferD\n");
                                _environment.fill4_duty = dtmp[2];

                                switch (_environment.fill4_state) {
                                case MODE_OFF:
                                        printf("PUMP_FILL_D to mode OFF\n");
                                        SETFILL(PUMP_D,false,"PUMP_D_OFF");
                                        break;
                                case MODE_ON:
                                        printf("PUMP_FILL_D to mode ON\n");
                                        SETFILL(PUMP_D,true,"PUMP_D_ON");
                                        break;
                                case MODE_AUTO:
                                        printf("PUMP_FILL_D to mode AUTO\n");
                                        SETFILL(PUMP_D,false,"PUMP_D_OFF");
                                        expression_task = fer_ok;
                                        start_dosing_ec = true;
                                        break;
                                }

                                break;
                        case 5:
                                printf("case SET_DUTYCYCLE pH\n");
                                _environment.pump_ph_duty = dtmp[2];

                                switch (_environment.pump_ph_state) {
                                case MODE_OFF:
                                        printf("PUMP_FILL_PH to mode OFF\n");
                                        SETFILL(PUMP_PH,false,"PUMP_PH_OFF");
                                        break;
                                case MODE_ON:
                                        printf("PUMP_FILL_PH to mode ON\n");
                                        SETFILL(PUMP_PH,true,"PUMP_PH_ON");
                                        break;
                                case MODE_AUTO:
                                        printf("PUMP_FILL_PH to mode AUTO\n");
                                        SETFILL(PUMP_PH,false,"PUMP_PH_OFF");
                                        expression_task = fer_ok;
                                        start_dosing_ph = true;
                                        break;
                                case MODE_TIME:
                                        printf("PUMP_FILL_PH to mode TIME\n");
                                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                                        expression_task = fer_ok;
                                        start_dosing_ph = true;
                                        break;
                                }

                                break;
                        }
                        save_environment(_environment);
                        break;

                case PAGE_CALIBRATE_EC:
                        printf("case PAGE_CALIBRATE_EC\n");
                        sprintf(str_name,PER_BUFF,0);
                        send_tft(str_name);
                        sprintf(tft_val, "EC %.2f", ec_read(ec_val_plot));
                        sprintf(str_name, INFO_BUFF_EC, tft_val);
                        send_tft(str_name);
                        vTaskDelay(500 / portTICK_RATE_MS);
                        sprintf(tft_val, "Analog %d", ec_val_plot);
                        sprintf(str_name, INFO_BUFF_EC, tft_val);
                        send_tft(str_name);
                        vTaskDelay(500 / portTICK_RATE_MS);
                        ec_calibration(ec_val_plot, 1);
                        vTaskDelay(500 / portTICK_RATE_MS);
                        ec_calibration(ec_val_plot, 2);

                        break;

                case PAGE_CALIBRATE_PH:
                        printf("case PAGE_CALIBRATE_PH\n");
                        sprintf(str_name,PER_BUFF,0);
                        send_tft(str_name);
                        sprintf(tft_val, "pH %.2f", ph_value);
                        sprintf(str_name, INFO_BUFF_PH, tft_val);
                        send_tft(str_name);
                        vTaskDelay(500 / portTICK_RATE_MS);
                        sprintf(tft_val, "Analog %.2f", adc_reading_buff);
                        sprintf(str_name, INFO_BUFF_PH, tft_val);
                        send_tft(str_name);
                        vTaskDelay(500 / portTICK_RATE_MS);
                        calibration(adc_reading_buff, 1);
                        vTaskDelay(500 / portTICK_RATE_MS);
                        calibration(adc_reading_buff, 2);
                        break;

                case SET_CALIBRATE_EC:
                        printf("case SET_CALIBRATE_EC\n");
                        ec_calibration(ec_val_plot, 3);
                        break;

                case SET_CALIBRATE_PH:
                        printf("case SET_CALIBRATE_PH\n");
                        calibration(adc_reading_buff, 3);
                        break;

                case PAGE_VOLTAGE_PH:
                        printf("case PAGE_VOLTAGE_PH\n");
                        int ph__ =  (float)adc_reading_buff*100;
                        sprintf(str_name, ANALOG_PH,ph__);
                        send_tft(str_name);
                        break;

                case SET_VOLTAGE_PH:
                        switch (dtmp[1]) {
                        case 1:   // PH_4
                                printf("case SET_VOLTAGE_PH 4\n");
                                if (dtmp[3] > 0 && dtmp[4]==0)
                                {
                                        uint16_t val = dtmp[3] * 256 + dtmp[2];
                                        ph_val.PH_4_VOL = ((float)val/10);
                                        ph_val.PH_5_VOL = ph_val.PH_4_VOL-25.00;
                                        ph_val.PH_3_VOL = ph_val.PH_4_VOL+25.00;
                                        printf("case SET_VOLTAGE_PH 4 [>256] --> %.2f,%.2f,%.2f\n",ph_val.PH_5_VOL,
                                               ph_val.PH_4_VOL,ph_val.PH_3_VOL);
                                }
                                else if (dtmp[4]>0)
                                {
                                        uint32_t val = ((dtmp[4]*65536) + (dtmp[3]* 256)) + dtmp[2];
                                        ph_val.PH_4_VOL = ((float)val/100);
                                        ph_val.PH_5_VOL = ph_val.PH_4_VOL-25.00;
                                        ph_val.PH_3_VOL = ph_val.PH_4_VOL+25.00;
                                        printf("case SET_VOLTAGE_PH 4 [>256] --> %.2f,%.2f,%.2f\n",ph_val.PH_5_VOL,
                                               ph_val.PH_4_VOL,ph_val.PH_3_VOL);
                                }

                                break;
                        case 2:   // PH_7
                                printf("case SET_VOLTAGE_PH 7\n");
                                if (dtmp[3] > 0 && dtmp[4]==0)
                                {
                                        uint16_t val = dtmp[3] * 256 + dtmp[2];
                                        ph_val.PH_7_VOL = ((float)val/10);
                                        ph_val.PH_8_VOL = ph_val.PH_7_VOL-25.00;
                                        ph_val.PH_6_VOL = ph_val.PH_7_VOL+25.00;
                                        printf("case SET_VOLTAGE_PH 7 [>256] --> %.2f,%.2f,%.2f\n",ph_val.PH_8_VOL,
                                               ph_val.PH_7_VOL,ph_val.PH_6_VOL);
                                }
                                else if (dtmp[4]>0)
                                {
                                        uint32_t val = ((dtmp[4]*65536) + (dtmp[3]* 256)) + dtmp[2];
                                        ph_val.PH_7_VOL = ((float)val/100);
                                        ph_val.PH_8_VOL = ph_val.PH_7_VOL-25.00;
                                        ph_val.PH_6_VOL = ph_val.PH_7_VOL+25.00;
                                        printf("case SET_VOLTAGE_PH 7 [>256] --> %.2f,%.2f,%.2f\n",ph_val.PH_8_VOL,
                                               ph_val.PH_7_VOL,ph_val.PH_6_VOL);
                                }

                                save_ph_kvalue(ph_val);
                                break;
                        }
                        break;

                case PAGE_DATE:
                        printf("case PAGE_DATE\n");
                        call_time(2);
                        break;

                case SET_DATE:
                        printf("case SET_DATE\n");
                        //settime yy/mm/dd hh-mm-ss
                        uint16_t convert_year = dtmp[7] * 256 + dtmp[6];
                        tft_set_time(convert_year, dtmp[5], dtmp[4], dtmp[1], dtmp[2], dtmp[3]);
                        break;

                case PAGE_NETWORK:
TO_PAGE_NETWORK:
                        printf("case PAGE_NETWORK\n");
                        tcip_info();
                        call_rssi();
                        break;

                case SCAN_NETWORK:
TO_SCAN_NETWORK:
                        printf("case SCAN_NETWORK\n");
                        scan_wifi();
                        break;

                case CONNECT_NETWORK:
                        printf("case CONNECT_NETWORK\n");
                        switch (dtmp[1])
                        {
                        case 1:
                                printf("case SAVE_SSID\n");
                                char *ssid;
                                ssid = malloc(sizeof(dtmp) + 1);
                                memset(&ssid[0], 0x00, sizeof(dtmp) + 1);
                                memcpy(&ssid[0], &dtmp[2], sizeof(dtmp));
                                set_network.ssid = ssid;

                                break;
                        case 2:
                                printf("case SAVE_PASS\n");
                                char *pass;
                                pass = malloc(sizeof(dtmp) + 1);
                                memset(&pass[0], 0x00, sizeof(dtmp) + 1);
                                memcpy(&pass[0], &dtmp[2], sizeof(dtmp));
                                set_network.pass = pass;
                                printf("WIFI CONFIG %s:%s\n", set_network.ssid, set_network.pass);
                                reconnect_wifi(set_network.ssid, set_network.pass);

                                sprintf(str_name, PAGE_TFT, "Network");
                                send_tft(str_name);
                                goto TO_PAGE_NETWORK;
                                break;
                        }
                        break;

                case ERASE_FLASH:
                        printf("case ERASE_FLASH\n");
                        bz_tone(Eb8, 600);
                        load_default_nvs();
                        esp_restart();
                        break;

                case ESP32_TFT_RESTART:
                        printf("case ERASE_FLASH\n");
                        esp_restart();
                        break;

                case PAGE_FW:
                        printf("case PAGE_FW\n");

                        switch (dtmp[1])
                        {
                        case 1:
                                printf("case PAGE_FW_ESP32\n");
                                if (network_is_alive())
                                {
                                        //version new in http
                                        sprintf(str_name, WAIT_OTA, 1);
                                        send_tft(str_name);
                                        char ver_esp_[5];
                                        uint8_t version_esp_n = (float)read_ver_bin() * 10;
                                        uint8_t version_esp_o = (float)FIRMWARE_ESP32 * 10;
                                        sprintf(str_name, WAIT_OTA, 0);
                                        send_tft(str_name);
                                        printf("read ver esp %d\n", version_esp_n);
                                        sprintf(ver_esp_, "%.1f", (float)version_esp_n / 10);

                                        //send ver in program
                                        sprintf(str_name, VERSION_FW, version_esp_o);
                                        send_tft(str_name);

                                        if (version_esp_n > version_esp_o)
                                        {
                                                sprintf(str_name, INSTALL_FW, 1);
                                                send_tft(str_name);
                                                sprintf(str_name, INFO_FW, "There are new firmware updates", ver_esp_);
                                                send_tft(str_name);
                                        }
                                        else
                                        {
                                                sprintf(str_name, INSTALL_FW, 0);
                                                send_tft(str_name);
                                                sprintf(str_name, INFO_FW, "Firmware is Lasted", ver_esp_);
                                                send_tft(str_name);
                                        }
                                }
                                else
                                {
                                        ESP_LOGI("INFO_WIFI", "Wi-Fi network connection missing");
                                        sprintf(str_name, PAGE_TFT, "Scan");
                                        send_tft(str_name);
                                        goto TO_SCAN_NETWORK;
                                }
                                break;
                        case 2:
                                printf("case PAGE_FW_TFT\n");
                                //version new in http
                                sprintf(str_name, WAIT_OTA, 1);
                                send_tft(str_name);
                                char ver_tft_[5];
                                uint8_t version_tft_n = (float)read_ver_tft() * 10;
                                sprintf(str_name, WAIT_OTA, 0);
                                send_tft(str_name);
                                printf("read ver tft %d\n", version_tft_n);
                                sprintf(ver_tft_, "%.1f", (float)version_tft_n / 10);
                                if (network_is_alive())
                                {
                                        if (version_tft_n > dtmp[2])
                                        {
                                                sprintf(str_name, INSTALL_FW, 1);
                                                send_tft(str_name);
                                                sprintf(str_name, INFO_FW, "There are new firmware updates", ver_tft_);
                                                send_tft(str_name);
                                        }
                                        else
                                        {
                                                sprintf(str_name, INSTALL_FW, 0);
                                                send_tft(str_name);
                                                sprintf(str_name, INFO_FW, "Firmware is Lasted", ver_tft_);
                                                send_tft(str_name);
                                        }
                                }
                                else
                                {
                                        ESP_LOGI("INFO_WIFI", "Wi-Fi network connection missing");
                                        sprintf(str_name, PAGE_TFT, "Scan");
                                        send_tft(str_name);
                                        goto TO_SCAN_NETWORK;
                                }
                                break;
                        }

                        break;

                case UPDATE_ESP32:
                        printf("case UPDATE_ESP32\n");
                        ota_http();
                        break;

                case UPDATE_TFT:
                        printf("case UPDATE_TFT\n");
                        sprintf(str_name, CONNECT_TFT);
                        send_tft(str_name);
                        esp_task_wdt_deinit();
                        esp_task_wdt_status(task_1);
                        esp_task_wdt_status(task_3);
                        esp_task_wdt_status(task_4);
                        vTaskSuspend(task_1);
                        vTaskSuspend(task_3);
                        vTaskSuspend(task_4);
                        vTaskDelay(1000 / portTICK_RATE_MS);
                        // connect to webserver and upload Nextion firmware
                        if (!nex_ota_upload())
                        {
                                ESP_LOGE("Nextion OTA", "Error uploading firmware!");
                                status_pg.OTA_NEXTION = 0;
                                save_statuspg(status_pg);
                                esp_restart();
                        }
                        else
                        {
                                ESP_LOGI("Nextion OTA", "Done!");
                                vTaskResume(task_1);
                                vTaskResume(task_3);
                                vTaskResume(task_4);
                                status_pg.OTA_NEXTION = 1;
                                save_statuspg(status_pg);
                                esp_restart();
                        }

                        break;

                case PAGE_COMMUNITY:
                        printf("case PAGE_COMMUNITY\n");
                        sprintf(str_name, QR_CODE, CONTAIN_DASHBOARD);
                        send_tft(str_name);

                        break;

                case PAGE_DEBUG:
                        printf("case PAGE_DEBUG\n");
                        read_statuspg(&status_pg);
                        sprintf(str_name, DEVICE_NO, status_pg.mydevice_no);
                        send_tft(str_name);
                        break;

                case DEBUG_SETMQTT:
                        printf("case DEBUG_SETMQTT\n");
                        status_pg.mydevice_no = dtmp[1];
                        save_statuspg(status_pg);
                        sprintf(str_name, DEVICE_NO, status_pg.mydevice_no);
                        send_tft(str_name);
                        esp_restart();
                        break;
                }
        }
        vTaskDelete(NULL);
}

static void periodic_timer_callback(void* arg)
{
        time_since_boot = esp_timer_get_time() / 1e6;

        ESP_LOGI("periodic", "Periodic timer called, time since boot: %lld sec", time_since_boot);

}


void app_main()
{

        ESP_LOGI("ESP32", "[APP] Startup..");
        ESP_LOGI("ESP32", "[APP] Free memory: %d bytes", esp_get_free_heap_size());
        ESP_LOGI("ESP32", "[APP] IDF version: %s", esp_get_idf_version());

        //Initializing the task watchdog
        //ห้ามค้างเกิน 90 secs
        esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);

        //Initialize NVS
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
        {
                ESP_ERROR_CHECK(nvs_flash_erase());
                ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);

        //UART MODE
        init_ec();  /* init ec */
        init_tft(); /* init lcd nextion */

        //WIFI MODE
        initialise_wifi();    /* init wifi */
        initialise_mqtt_cm(); /* init mqtt_cm */

        //check version file ota nextion & mqtt device
        if (read_statuspg(&status_pg))
                ;
        else
        {
                status_pg.mydevice_no = 0;
                status_pg.switch_mode = 0;
                status_pg.OTA_NEXTION = 1;
                save_statuspg(status_pg);
        }
        if (status_pg.OTA_NEXTION == 0)
        {
                wifi_wait_connected();
                if (!nex_ota_upload())
                {
                        ESP_LOGE("Nextion OTA", "Error uploading firmware!");
                        status_pg.OTA_NEXTION = 0;
                        save_statuspg(status_pg);
                        esp_restart();
                }
                else
                {
                        ESP_LOGI("Nextion OTA", "Done!");
                        status_pg.OTA_NEXTION = 1;
                        save_statuspg(status_pg);
                        esp_restart();
                }
        }

        //I2C MODE
        pca9685_init();               /* init i2c pca9685 */
        PCA9685_ADDR = 0x41;
        setFrequencyPCA9685(1000);
        turnAllOff();
        PCA9685_ADDR = 0x42;
        setFrequencyPCA9685(1000);
        turnAllOff();

        ds1307_init();                /* init i2c ds1307 */
        temp_init();                  /* init i2c hdc1080 */

        scan_i2c();                  /* scan i2c */

        //ANALOG MODE
        init_ph();                    /* init ph */
        ec_add_val();                  /* init ec */

        //DIGITAL MODE
        app_buzzer_cfg(); /* init buzzer */
        ds18b20_init();   /* init water temp */

        // init waterlv
        gpio_config_t io_conf;
        io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
        io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_down_en = 0;
        io_conf.pull_up_en = 1;
        gpio_config(&io_conf);

        const esp_timer_create_args_t periodic_timer_args = {
                .callback = &periodic_timer_callback,
                /* name is optional, but may help identify the timer when debugging */
                .name = "periodic"
        };
        esp_timer_handle_t periodic_timer;
        ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
        /* Start the timers */
        ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000000));

        //100ms
        xTaskCreate(RECV_CALL_EC, "RECV_CALL_EC", 2048, NULL, 2, &task_1);

        //1000 ms
        xTaskCreate(task_timer_sch, "task_timer_sch", 5120, NULL, 4, &task_3);

        //0ms
        xTaskCreate(RECV_CALL_TFT, "RECV_CALL_TFT", 10240, NULL, 5, &task_2);

        //10000 ms
        xTaskCreate(read_sensor_task, "read_sensor_task", 4096, NULL, 3, &task_4);


        vTaskDelay(500 / portTICK_RATE_MS);

        TFT_RESTART(); /* rest tft */

}
