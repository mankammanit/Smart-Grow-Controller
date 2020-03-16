#ifndef __FUNCTION_H__
#define __FUNCTION_H__

////////////////// tft /////////////////////////
char ec_tft[5], ph_tft[5], water_tft[5];
char tft_val[16], tft_val2[5], tft_val3[5],
     tft_val4[5];
char* mqttpgstage;
char* mqttpgzone[5];
char* mqttpgsch[5];
uint16_t ec_val_plot;
////////////////////////////////////////////////

/////////////////// Timer //////////////////////
uint16_t Timestamp_day,planted_state_1,planted_state_2;
int64_t time_since_boot;
bool first_start[4][4];
time_t Timestamp,current_stamp,Pumpstamp;
bool check_fill_water = true;
uint16_t Day_Index = 0;
bool load_day = false;
///////////////////////////////////////////////

////////////////// sensor //////////////////////
static hdc1080_sensor_t *internal_temp_sensor;
static hdc1080_sensor_t *external_temp_sensor;
float read_temperature[2];
float read_humidity[2];
////////////////////////////////////////////////

static void update_mqtt_cm()
{

        sprintf(tft_val, "%.1f", ec_setpoint);
        esp_mqtt_publish_string("EC_SETPOINT",tft_val);
        sprintf(tft_val, "%.1f", ph_setpoint);
        esp_mqtt_publish_string("PH_SETPOINT",tft_val);

        esp_mqtt_publish_number("PG_Working", Timestamp_day);
        esp_mqtt_publish_string("PG_STAGE",mqttpgstage);

}

static void waterlv()
{

        switch (call_water_lv())
        {
        case 0:
                // printf("No Detect Water ALL\n");

                check_fill_water = true;

                sprintf(str_name, WATERLV, 0);
                send_tft(str_name);
                esp_mqtt_publish_number("Water_LV", 0);

                if (_environment.solenoide_state == 2)
                {
                        if (check_fill_water)
                        {
                                printf("-------------------------------------------------------Fill Water in LV0\n");
                                esp_mqtt_publish_number("Water_fill", 1);
                                SETFILL(SOLENOIDE, true,"SOLENOIDE_ON");
                                sprintf(str_name, PLAY_SOUND,1,2,0);
                                send_tft(str_name);
                                vTaskDelay(2000 / portTICK_RATE_MS);
                                // bz_tone(Eb8, 600);
                        }
                }
                // else printf("solenoide no auto mode\n");

                break;
        case 1:
                // printf("Detect Water LV1\n");

                check_fill_water = true;

                sprintf(str_name, WATERLV, 35);
                send_tft(str_name);
                esp_mqtt_publish_number("Water_LV", 35);

                if (_environment.solenoide_state == 2)
                {
                        if (check_fill_water)
                        {
                                printf("-------------------------------------------------------Fill Water in LV1\n");
                                esp_mqtt_publish_number("Water_fill", 1);
                                SETFILL(SOLENOIDE, true,"SOLENOIDE_ON");
                        }
                }
                // else printf("solenoide no auto mode\n");

                break;
        case 2:
                // printf("Detect Water LV2\n");

                sprintf(str_name, WATERLV, 70);
                send_tft(str_name);
                esp_mqtt_publish_number("Water_LV", 70);
                if (_environment.solenoide_state == 2)
                {
                        if (check_fill_water)
                        {
                                printf("-------------------------------------------------------Fill Water in LV2\n");
                                esp_mqtt_publish_number("Water_fill", 1);
                                SETFILL(SOLENOIDE, true,"SOLENOIDE_ON");
                        }
                }
                // else printf("solenoide no auto mode\n");

                break;
        case 3:
                // printf("Detect Water LV3\n");
                sprintf(str_name, WATERLV, 100);
                send_tft(str_name);
                esp_mqtt_publish_number("Water_LV", 100);

                if (_environment.solenoide_state == 2)
                {
                        if (check_fill_water)
                        {
                                printf("-------------------------------------------------------Fill Water in LV3\n");
                                esp_mqtt_publish_number("Water_fill", 1);
                                SETFILL(SOLENOIDE, false,"SOLENOIDE_OFF");
                                check_fill_water = false;
                                esp_mqtt_publish_number("Water_fill", 0);
                        }
                        // else esp_mqtt_publish_number("Water_fill",0);
                }
                // else printf("solenoide no auto mode\n");

                break;
        }

}

//tft rest
static void TFT_RESTART()
{
        sprintf(str_name, TFT_REST);
        send_tft(str_name);
}

static void call_time(uint8_t page)
{
        //เวลาที่รันตลอด
        struct tm time;
        readValue(&time);

        uint16_t set_y = time.tm_year + 1900;
        uint8_t set_mo = time.tm_mon + 1;
        uint8_t set_d = time.tm_mday;
        uint8_t set_h = time.tm_hour;
        uint8_t set_m = time.tm_min;
        uint8_t set_s = time.tm_sec;
        printf("Current Time %04d-%02d-%02d %02d:%02d:%02d\n", set_y, set_mo, set_d, set_h,
               set_m, set_s);

        switch (page)
        {
        case 1:
                sprintf(str_name, RTC0, set_y);
                send_tft(str_name);
                sprintf(str_name, RTC1, set_mo);
                send_tft(str_name);
                sprintf(str_name, RTC2, set_d);
                send_tft(str_name);
                sprintf(str_name, RTC3, set_h);
                send_tft(str_name);
                sprintf(str_name, RTC4, set_m);
                send_tft(str_name);
                sprintf(str_name, RTC5, set_s);
                send_tft(str_name);
                printf("set time dash board\n");
                break;
        case 2:
                sprintf(str_name, CALL_Y, set_y);
                send_tft(str_name);
                sprintf(str_name, CALL_MO, set_mo);
                send_tft(str_name);
                sprintf(str_name, CALL_D, set_d);
                send_tft(str_name);
                sprintf(str_name, CALL_H, set_h);
                send_tft(str_name);
                sprintf(str_name, CALL_M, set_m);
                send_tft(str_name);
                sprintf(str_name, CALL_S, set_s);
                send_tft(str_name);
                printf("set time to setdate\n");
                break;
        }
}

static void read_sensor_all()
{
        readAnalogpH();
        //ph
        sprintf(ph_tft, "%.1f", ph_value);
        sprintf(str_name,PH_SEND,ph_tft);
        send_tft(str_name);
        esp_mqtt_publish_string("PH",ph_tft);

        ec_value = ec_read(ec_val_plot);
        //ec
        // printf("EC analog is %d\n",ec_val_plot);
        sprintf(ec_tft, "%.2f",ec_value);
        sprintf(str_name,EC_SEND,ec_tft);
        send_tft(str_name);
        esp_mqtt_publish_string("EC",ec_tft);

        //water temp
        float water_temp = ds18b20_get_temp();
        if(water_temp > 0 && water_temp < 40.00)
        {
                // // send tft
                sprintf(water_tft, "%.1f", water_temp);
                sprintf(str_name,WATER_TEMP_SEND,water_tft);
                send_tft(str_name);
                esp_mqtt_publish_string("WATER_TEMP",water_tft);
        }

        hdc1080_read(external_temp_sensor, &read_temperature[0], &read_humidity[0]);
        hdc1080_read(internal_temp_sensor, &read_temperature[1], &read_humidity[1]);


        printf("\n################## READ SENSOR ###################\n");
        printf("PH Val : %.1f\n",ph_value);
        printf("EC Val : %.2f\n",ec_value);
        printf("EC Temp : %.2f\n",ec_temp);
        printf( "Water TEMP : %.1f\n", water_temp);
        printf("Internal: temperature: %.1f , humidity %.1f\n", read_temperature[0], read_humidity[0]);
        printf("External: temperature: %.1f , humidity %.1f\n", read_temperature[1], read_humidity[1]);
        printf("##################################################\n\n");
}

static uint8_t list_my_stage()
{
        Timestamp_day = Timestamp / 86400 + status_pg.start_day;
        planted_state_1 = (time_pg.dayoff[0] - time_pg.dayon[0])+1;
        planted_state_2 = (time_pg.dayoff[1] - time_pg.dayon[1])+planted_state_1+1;
        //ส่งค่าวันที่ปลูก
        status_pg.start_day = Timestamp_day;
        if(status_pg.switch_mode ==1)
        {
                sprintf(tft_val, "%d", Timestamp_day);
                sprintf(str_name, STATUS_DAY, tft_val);
                send_tft(str_name);

                if(Timestamp_day<=planted_state_1)
                {
                        return 1;
                }
                if(Timestamp_day>planted_state_1 && Timestamp_day<=planted_state_2)
                {
                        return 2;
                }
                if(Timestamp_day>planted_state_2)
                {
                        return 3;
                }
        }
        else
        {
                sprintf(str_name, STATUS_DAY, "-");
                send_tft(str_name);
                return 0;
        }

        return 0;
}

static void input_light_stage(uint8_t stage)
{
        for(uint8_t zone = 0; zone <4; zone++)
        {
                if (betweenTimes(time_pg.pg_hour_on[stage][zone], time_pg.pg_hour_off[stage][zone],
                                 time_pg.pg_min_on[stage][zone], time_pg.pg_min_off[stage][zone]))
                {
                        // printf("-------------------------------------------------------LED_ZONE%d_ON\n",zone+1);
                        sprintf(str_name, "LED_ZONE%d_ON",zone+1);
                        sprintf(tft_val, "PG_Schedule%d",zone+1);
                        esp_mqtt_publish_string(tft_val,str_name);

                        if (first_start[stage][zone] == true)
                        {

                                if(zone==0)
                                {
                                        printf("-------------------------------------------------------SET LED_ZONE%d_ON\n",zone+1);
                                        set_bright(ZONE1_LED1, ZONE1_LED2, ZONE1_LED3, ZONE1_LED4,
                                                   time_pg.bright_1[stage][zone],
                                                   time_pg.bright_2[stage][zone],
                                                   time_pg.bright_3[stage][zone],
                                                   time_pg.bright_4[stage][zone]);
                                }
                                else if(zone==1)
                                {
                                        printf("-------------------------------------------------------SET LED_ZONE%d_ON\n",zone+1);
                                        set_bright(ZONE2_LED1, ZONE2_LED2, ZONE2_LED3, ZONE2_LED4,
                                                   time_pg.bright_1[stage][zone],
                                                   time_pg.bright_2[stage][zone],
                                                   time_pg.bright_3[stage][zone],
                                                   time_pg.bright_4[stage][zone]);
                                }
                                else if(zone==2)
                                {
                                        printf("-------------------------------------------------------SET LED_ZONE%d_ON\n",zone+1);
                                        set_bright(ZONE3_LED1, ZONE3_LED2, ZONE3_LED3, ZONE3_LED4,
                                                   time_pg.bright_1[stage][zone],
                                                   time_pg.bright_2[stage][zone],
                                                   time_pg.bright_3[stage][zone],
                                                   time_pg.bright_4[stage][zone]);
                                }
                                else if(zone==3)
                                {
                                        printf("-------------------------------------------------------SET LED_ZONE%d_ON\n",zone+1);
                                        set_bright(ZONE4_LED1, ZONE4_LED2, ZONE4_LED3, ZONE4_LED4,
                                                   time_pg.bright_1[stage][zone],
                                                   time_pg.bright_2[stage][zone],
                                                   time_pg.bright_3[stage][zone],
                                                   time_pg.bright_4[stage][zone]);
                                }
                                first_start[stage][zone] = false;
                        }
                }
                else
                {
                        // printf("-------------------------------------------------------LED_ZONE%d_OFF\n",zone+1);
                        sprintf(str_name, "LED_ZONE%d_OFF",zone+1);
                        sprintf(tft_val, "PG_Schedule%d",zone+1);
                        esp_mqtt_publish_string(tft_val,str_name);

                        if(zone==0)
                        {
                                printf("-------------------------------------------------------SET LED_ZONE%d_OFF\n",zone+1);
                                set_bright(ZONE1_LED1, ZONE1_LED2, ZONE1_LED3, ZONE1_LED4,
                                           0,
                                           0,
                                           0,
                                           0);

                        }
                        else if(zone==1)
                        {
                                printf("-------------------------------------------------------SET LED_ZONE%d_OFF\n",zone+1);
                                set_bright(ZONE2_LED1, ZONE2_LED2, ZONE2_LED3, ZONE2_LED4,
                                           0,
                                           0,
                                           0,
                                           0);

                        }
                        else if(zone==2)
                        {
                                printf("-------------------------------------------------------SET LED_ZONE%d_OFF\n",zone+1);
                                set_bright(ZONE3_LED1, ZONE3_LED2, ZONE3_LED3, ZONE3_LED4,
                                           0,
                                           0,
                                           0,
                                           0);

                        }
                        else if(zone==3)
                        {
                                printf("-------------------------------------------------------SET LED_ZONE%d_OFF\n",zone+1);
                                set_bright(ZONE4_LED1, ZONE4_LED2, ZONE4_LED3, ZONE4_LED4,
                                           0,
                                           0,
                                           0,
                                           0);

                        }
                        first_start[stage][zone] = true;
                }
        }
}

static void call_time_light()
{

        switch (list_my_stage()) {
        case 0:
                printf("-------------------------------------------------------WORKING MANUAL\n");
                mqttpgstage = "MANUAL";
                break;
        case 1:
                printf("-------------------------------------------------------WORKING STAGE1\n");
                mqttpgstage = "STAGE1";
                sprintf(str_name, STATUS_PG1,18);
                send_tft(str_name);
                sprintf(str_name, STATUS_PG2,17);
                send_tft(str_name);
                sprintf(str_name, STATUS_PG3,17);
                send_tft(str_name);
                input_light_stage(0);
                break;
        case 2:
                printf("-------------------------------------------------------WORKING STAGE2\n");
                mqttpgstage = "STAGE2";
                sprintf(str_name, STATUS_PG1,17);
                send_tft(str_name);
                sprintf(str_name, STATUS_PG2,18);
                send_tft(str_name);
                sprintf(str_name, STATUS_PG3,17);
                send_tft(str_name);
                input_light_stage(1);
                break;
        case 3:
                printf("-------------------------------------------------------WORKING STAGE3\n");
                mqttpgstage = "STAGE3";
                sprintf(str_name, STATUS_PG1,17);
                send_tft(str_name);
                sprintf(str_name, STATUS_PG2,17);
                send_tft(str_name);
                sprintf(str_name, STATUS_PG3,18);
                send_tft(str_name);
                input_light_stage(2);
                break;
        }

}

static uint8_t call_status_pump()
{
        Day_Index = Pumpstamp/86400;
        // printf("Day_Index %d , working_day %d \n", Day_Index,working_timer.working_day);

        if(_environment.pump_water_state==2)
        {

                if (working_timer.working_day == 0 )
                {
                        printf("-------------------------------------------------------Every day\n");
                        if (betweenTimes(working_timer.working_on_h[0], working_timer.working_off_h[0],
                                         working_timer.working_on_m[0], working_timer.working_off_m[0]))
                        {
                                if(working_timer.status_timer[0] == 1)
                                {
                                        return 1;

                                }
                        }
                        else if (betweenTimes(working_timer.working_on_h[1], working_timer.working_off_h[1],
                                              working_timer.working_on_m[1], working_timer.working_off_m[1]))
                        {
                                if(working_timer.status_timer[1] == 1)
                                {
                                        return 2;
                                }
                        }
                        else if (betweenTimes(working_timer.working_on_h[2], working_timer.working_off_h[2],
                                              working_timer.working_on_m[2], working_timer.working_off_m[2]))
                        {
                                if(working_timer.status_timer[2] == 1)
                                {
                                        return 3;
                                }
                        }
                        else if (betweenTimes(working_timer.working_on_h[3], working_timer.working_off_h[3],
                                              working_timer.working_on_m[3], working_timer.working_off_m[3]))
                        {
                                if(working_timer.status_timer[3] == 1)
                                {
                                        return 4;
                                }
                        }
                        else return 0;
                }
                else if (working_timer.working_day !=0)
                {
                        // printf("-------------------------------------------------------Some day\n");

                        if(Day_Index==0)

                        {
                                working_timer.working_nextday = Day_Index + working_timer.working_day+1;
                                working_timer.working_lastday = Day_Index;
                                save_working(working_timer);
                                // printf("working_nextday %d,working_lastday %d\n",working_timer.working_nextday,
                                //        working_timer.working_lastday);
                                printf("-------------------------------------------------------Some day %d-->%d\n",working_timer.working_lastday,
                                       working_timer.working_nextday);

                                if (betweenTimes(working_timer.working_on_h[0], working_timer.working_off_h[0],
                                                 working_timer.working_on_m[0], working_timer.working_off_m[0]))
                                {
                                        if(working_timer.status_timer[0] == 1)
                                        {
                                                return 1;

                                        }
                                }
                                else if (betweenTimes(working_timer.working_on_h[1], working_timer.working_off_h[1],
                                                      working_timer.working_on_m[1], working_timer.working_off_m[1]))
                                {
                                        if(working_timer.status_timer[1] == 1)
                                        {
                                                return 2;
                                        }
                                }
                                else if (betweenTimes(working_timer.working_on_h[2], working_timer.working_off_h[2],
                                                      working_timer.working_on_m[2], working_timer.working_off_m[2]))
                                {
                                        if(working_timer.status_timer[2] == 1)
                                        {
                                                return 3;
                                        }
                                }
                                else if (betweenTimes(working_timer.working_on_h[3], working_timer.working_off_h[3],
                                                      working_timer.working_on_m[3], working_timer.working_off_m[3]))
                                {
                                        if(working_timer.status_timer[3] == 1)
                                        {
                                                return 4;
                                        }
                                }

                                else return 0;
                        }
                        if(Day_Index == working_timer.working_nextday)
                        {

                                working_timer.working_nextday = Day_Index + working_timer.working_day+1;
                                working_timer.working_lastday = Day_Index;
                                save_working(working_timer);
                                // printf("working_nextday %d,working_lastday %d\n",working_timer.working_nextday,
                                //        working_timer.working_lastday);
                                printf("-------------------------------------------------------Some day %d-->%d\n",working_timer.working_lastday,
                                       working_timer.working_nextday);

                                if (betweenTimes(working_timer.working_on_h[0], working_timer.working_off_h[0],
                                                 working_timer.working_on_m[0], working_timer.working_off_m[0]))
                                {
                                        if(working_timer.status_timer[0] == 1)
                                        {
                                                return 1;

                                        }
                                }
                                else if (betweenTimes(working_timer.working_on_h[1], working_timer.working_off_h[1],
                                                      working_timer.working_on_m[1], working_timer.working_off_m[1]))
                                {
                                        if(working_timer.status_timer[1] == 1)
                                        {
                                                return 2;
                                        }
                                }
                                else if (betweenTimes(working_timer.working_on_h[2], working_timer.working_off_h[2],
                                                      working_timer.working_on_m[2], working_timer.working_off_m[2]))
                                {
                                        if(working_timer.status_timer[2] == 1)
                                        {
                                                return 3;
                                        }
                                }
                                else if (betweenTimes(working_timer.working_on_h[3], working_timer.working_off_h[3],
                                                      working_timer.working_on_m[3], working_timer.working_off_m[3]))
                                {
                                        if(working_timer.status_timer[3] == 1)
                                        {
                                                return 4;
                                        }
                                }

                                else return 0;
                        }

                }
        }

        else if(_environment.pump_water_state==1) return 5;

        else return 6;

        return 0;
}

static void call_time_waterpump()
{

        switch (call_status_pump()) {

        case 1:
                printf("-------------------------------------------------------PUMP Fer in TIME1 (MODE-ON)\n");
                SETFILL(ON_OFF_P1, true,"PUMP_WATER_ON");
                esp_mqtt_publish_number("WATER_PUMP", 1);

                break;
        case 2:
                printf("-------------------------------------------------------PUMP Fer in TIME2 (MODE-ON)\n");
                SETFILL(ON_OFF_P1, true,"PUMP_WATER_ON");
                esp_mqtt_publish_number("WATER_PUMP", 1);

                break;
        case 3:
                printf("-------------------------------------------------------PUMP Fer in TIME3 (MODE-ON)\n");
                SETFILL(ON_OFF_P1, true,"PUMP_WATER_ON");
                esp_mqtt_publish_number("WATER_PUMP", 1);

                break;
        case 4:
                printf("-------------------------------------------------------PUMP Fer in TIME4 (MODE-ON)\n");
                SETFILL(ON_OFF_P1, true,"PUMP_WATER_ON");
                esp_mqtt_publish_number("WATER_PUMP", 1);

                break;
        case 0:
                printf("-------------------------------------------------------PUMP Fer ALL (MODE-OFF)\n");
                SETFILL(ON_OFF_P1, false,"PUMP_WATER_OFF");
                esp_mqtt_publish_number("WATER_PUMP", 0);
                break;
        case 5: //mode on
                printf("-------------------------------------------------------PUMP Fer ALL (MANUAL-ON)\n");
                SETFILL(ON_OFF_P1, true,"PUMP_WATER_ON");
                esp_mqtt_publish_number("WATER_PUMP", 1);
                break;
        case 6: //mode off
                printf("-------------------------------------------------------PUMP Fer ALL (MANUAL-OFF)\n");
                SETFILL(ON_OFF_P1, false,"PUMP_WATER_OFF");
                esp_mqtt_publish_number("WATER_PUMP", 0);
                break;
        }
}

static void update_dashboard()
{

        sprintf(tft_val, "%d", working_timer.working_day);
        sprintf(str_name, loopTime_tft, tft_val);
        send_tft(str_name);

        if(working_timer.status_timer[0] == 1)
        {
                sprintf(str_name, DASH_TPUMP1,18);
                send_tft(str_name);
        }
        else
        {
                sprintf(str_name, DASH_TPUMP1,17);
                send_tft(str_name);
        }
        if(working_timer.status_timer[1] == 1)
        {
                sprintf(str_name, DASH_TPUMP2,18);
                send_tft(str_name);
        }
        else
        {
                sprintf(str_name, DASH_TPUMP2,17);
                send_tft(str_name);
        }
        if(working_timer.status_timer[2] == 1)
        {
                sprintf(str_name, DASH_TPUMP3,18);
                send_tft(str_name);
        }
        else
        {
                sprintf(str_name, DASH_TPUMP3,17);
                send_tft(str_name);
        }
        if(working_timer.status_timer[3] == 1)
        {
                sprintf(str_name, DASH_TPUMP4,18);
                send_tft(str_name);
        }
        else
        {
                sprintf(str_name, DASH_TPUMP4,17);
                send_tft(str_name);
        }

        sprintf(tft_val, "%02d", working_timer.working_on_h[0]);
        sprintf(tft_val2, "%02d", working_timer.working_on_m[0]);
        sprintf(tft_val3, "%02d", working_timer.working_off_h[0]);
        sprintf(tft_val4, "%02d", working_timer.working_off_m[0]);
        sprintf(str_name, PUMP_TIME1, tft_val, tft_val2,tft_val3,tft_val4);
        send_tft(str_name);

        sprintf(tft_val, "%02d", working_timer.working_on_h[1]);
        sprintf(tft_val2, "%02d", working_timer.working_on_m[1]);
        sprintf(tft_val3, "%02d", working_timer.working_off_h[1]);
        sprintf(tft_val4, "%02d", working_timer.working_off_m[1]);
        sprintf(str_name, PUMP_TIME2, tft_val, tft_val2,tft_val3,tft_val4);
        send_tft(str_name);

        sprintf(tft_val, "%02d", working_timer.working_on_h[2]);
        sprintf(tft_val2, "%02d", working_timer.working_on_m[2]);
        sprintf(tft_val3, "%02d", working_timer.working_off_h[2]);
        sprintf(tft_val4, "%02d", working_timer.working_off_m[2]);
        sprintf(str_name, PUMP_TIME3, tft_val, tft_val2,tft_val3,tft_val4);
        send_tft(str_name);

        sprintf(tft_val, "%02d", working_timer.working_on_h[3]);
        sprintf(tft_val2, "%02d", working_timer.working_on_m[3]);
        sprintf(tft_val3, "%02d", working_timer.working_off_h[3]);
        sprintf(tft_val4, "%02d", working_timer.working_off_m[3]);
        sprintf(str_name, PUMP_TIME4, tft_val, tft_val2,tft_val3,tft_val4);
        send_tft(str_name);


        switch (_environment.solenoide_state)
        {
        case 0:
                sprintf(str_name, DASH_SOLENOIDE, "OFF");
                send_tft(str_name);
                break;
        case 1:
                sprintf(str_name, DASH_SOLENOIDE, "ON");
                send_tft(str_name);
                break;
        case 2:
                sprintf(str_name, DASH_SOLENOIDE, "AUTO");
                send_tft(str_name);
                break;
        }

        switch (_environment.pump_water_state)
        {
        case 0:
                sprintf(str_name, DASH_WATERPUMP, "OFF");
                send_tft(str_name);
                break;
        case 1:
                sprintf(str_name, DASH_WATERPUMP, "ON");
                send_tft(str_name);
                break;
        case 2:
                sprintf(str_name, DASH_WATERPUMP, "AUTO");
                send_tft(str_name);
                break;
        }

        switch (list_my_stage()) {
        case 0:

                sprintf(str_name, MODE, "MANUAL");
                send_tft(str_name);
                sprintf(str_name, DASH_STAGE,"-");
                send_tft(str_name);

                sprintf(str_name, STATUS_DAY, "-");
                send_tft(str_name);

                sprintf(tft_val, "%d",ratio_led.bright_1[0]);
                sprintf(str_name, DASH_BRIGHT_led1,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",ratio_led.bright_2[0]);
                sprintf(str_name, DASH_BRIGHT_led2,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",ratio_led.bright_3[0]);
                sprintf(str_name, DASH_BRIGHT_led3,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",ratio_led.bright_4[0]);
                sprintf(str_name, DASH_BRIGHT_led4,tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d",ratio_led.bright_1[1]);
                sprintf(str_name, DASH_BRIGHT_led5,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",ratio_led.bright_2[1]);
                sprintf(str_name, DASH_BRIGHT_led6,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",ratio_led.bright_3[1]);
                sprintf(str_name, DASH_BRIGHT_led7,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",ratio_led.bright_4[1]);
                sprintf(str_name, DASH_BRIGHT_led8,tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d",ratio_led.bright_1[2]);
                sprintf(str_name, DASH_BRIGHT_led9,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",ratio_led.bright_2[2]);
                sprintf(str_name, DASH_BRIGHT_led10,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",ratio_led.bright_3[2]);
                sprintf(str_name, DASH_BRIGHT_led11,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",ratio_led.bright_4[2]);
                sprintf(str_name, DASH_BRIGHT_led12,tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d",ratio_led.bright_1[3]);
                sprintf(str_name, DASH_BRIGHT_led13,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",ratio_led.bright_2[3]);
                sprintf(str_name, DASH_BRIGHT_led14,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",ratio_led.bright_3[3]);
                sprintf(str_name, DASH_BRIGHT_led15,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",ratio_led.bright_4[3]);
                sprintf(str_name, DASH_BRIGHT_led16,tft_val);
                send_tft(str_name);

                sprintf(str_name, DASH_TIMERON_led1, "-", "-");
                send_tft(str_name);

                sprintf(str_name, DASH_TIMERON_led2, "-", "-");
                send_tft(str_name);

                sprintf(str_name, DASH_TIMERON_led3, "-", "-");
                send_tft(str_name);

                sprintf(str_name, DASH_TIMERON_led4, "-", "-");
                send_tft(str_name);

                sprintf(str_name, DASH_TIMEROFF_led1, "-", "-");
                send_tft(str_name);

                sprintf(str_name, DASH_TIMEROFF_led2, "-", "-");
                send_tft(str_name);

                sprintf(str_name, DASH_TIMEROFF_led3, "-", "-");
                send_tft(str_name);

                sprintf(str_name, DASH_TIMEROFF_led4, "-", "-");
                send_tft(str_name);

                sprintf(str_name, DASH_DUARATION_led1, "-");
                send_tft(str_name);

                sprintf(str_name, DASH_DUARATION_led2, "-");
                send_tft(str_name);

                sprintf(str_name, DASH_DUARATION_led3, "-");
                send_tft(str_name);

                sprintf(str_name, DASH_DUARATION_led4, "-");
                send_tft(str_name);

                break;
        case 1:

                sprintf(str_name, MODE, "AUTO");
                send_tft(str_name);
                sprintf(str_name, DASH_STAGE,"1");
                send_tft(str_name);


                sprintf(tft_val, "%d",time_pg.bright_1[0][0]);
                sprintf(str_name, DASH_BRIGHT_led1,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_2[0][0]);
                sprintf(str_name, DASH_BRIGHT_led2,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_3[0][0]);
                sprintf(str_name, DASH_BRIGHT_led3,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_4[0][0]);
                sprintf(str_name, DASH_BRIGHT_led4,tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d",time_pg.bright_1[0][1]);
                sprintf(str_name, DASH_BRIGHT_led5,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_2[0][1]);
                sprintf(str_name, DASH_BRIGHT_led6,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_3[0][1]);
                sprintf(str_name, DASH_BRIGHT_led7,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_4[0][1]);
                sprintf(str_name, DASH_BRIGHT_led8,tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d",time_pg.bright_1[0][2]);
                sprintf(str_name, DASH_BRIGHT_led9,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_2[0][2]);
                sprintf(str_name, DASH_BRIGHT_led10,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_3[0][2]);
                sprintf(str_name, DASH_BRIGHT_led11,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_4[0][2]);
                sprintf(str_name, DASH_BRIGHT_led12,tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d",time_pg.bright_1[0][3]);
                sprintf(str_name, DASH_BRIGHT_led13,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_2[0][3]);
                sprintf(str_name, DASH_BRIGHT_led14,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_3[0][3]);
                sprintf(str_name, DASH_BRIGHT_led15,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_4[0][3]);
                sprintf(str_name, DASH_BRIGHT_led16,tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_on[0][0]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_on[0][0]);
                sprintf(str_name, DASH_TIMERON_led1, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_on[0][1]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_on[0][1]);
                sprintf(str_name, DASH_TIMERON_led2, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_on[0][2]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_on[0][2]);
                sprintf(str_name, DASH_TIMERON_led3, tft_val,tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_on[0][3]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_on[0][3]);
                sprintf(str_name, DASH_TIMERON_led4, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_off[0][0]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_off[0][0]);
                sprintf(str_name, DASH_TIMEROFF_led1, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_off[0][1]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_off[0][1]);
                sprintf(str_name, DASH_TIMEROFF_led2, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_off[0][2]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_off[0][2]);
                sprintf(str_name, DASH_TIMEROFF_led3, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_off[0][3]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_off[0][3]);
                sprintf(str_name, DASH_TIMEROFF_led4, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[0][0], time_pg.pg_hour_off[0][0],
                                                 time_pg.pg_min_on[0][0], time_pg.pg_min_off[0][0]));
                sprintf(str_name, DASH_DUARATION_led1, tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[0][1], time_pg.pg_hour_off[0][1],
                                                 time_pg.pg_min_on[0][1], time_pg.pg_min_off[0][1]));
                sprintf(str_name, DASH_DUARATION_led2, tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[0][2], time_pg.pg_hour_off[0][2],
                                                 time_pg.pg_min_on[0][2], time_pg.pg_min_off[0][2]));
                sprintf(str_name, DASH_DUARATION_led3, tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[0][3], time_pg.pg_hour_off[0][3],
                                                 time_pg.pg_min_on[0][3], time_pg.pg_min_off[0][3]));
                sprintf(str_name, DASH_DUARATION_led4, tft_val);
                send_tft(str_name);
                break;

        case 2:

                sprintf(str_name, MODE, "AUTO");
                send_tft(str_name);
                sprintf(str_name, DASH_STAGE,"2");
                send_tft(str_name);

                sprintf(tft_val, "%d",time_pg.bright_1[1][0]);
                sprintf(str_name, DASH_BRIGHT_led1,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_2[1][0]);
                sprintf(str_name, DASH_BRIGHT_led2,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_3[1][0]);
                sprintf(str_name, DASH_BRIGHT_led3,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_4[1][0]);
                sprintf(str_name, DASH_BRIGHT_led4,tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d",time_pg.bright_1[1][1]);
                sprintf(str_name, DASH_BRIGHT_led5,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_2[1][1]);
                sprintf(str_name, DASH_BRIGHT_led6,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_3[1][1]);
                sprintf(str_name, DASH_BRIGHT_led7,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_4[1][1]);
                sprintf(str_name, DASH_BRIGHT_led8,tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d",time_pg.bright_1[1][2]);
                sprintf(str_name, DASH_BRIGHT_led9,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_2[1][2]);
                sprintf(str_name, DASH_BRIGHT_led10,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_3[1][2]);
                sprintf(str_name, DASH_BRIGHT_led11,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_4[1][2]);
                sprintf(str_name, DASH_BRIGHT_led12,tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d",time_pg.bright_1[1][3]);
                sprintf(str_name, DASH_BRIGHT_led13,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_2[1][3]);
                sprintf(str_name, DASH_BRIGHT_led14,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_3[1][3]);
                sprintf(str_name, DASH_BRIGHT_led15,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_4[1][3]);
                sprintf(str_name, DASH_BRIGHT_led16,tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_on[1][0]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_on[1][0]);
                sprintf(str_name, DASH_TIMERON_led1, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_on[1][1]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_on[1][1]);
                sprintf(str_name, DASH_TIMERON_led2, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_on[1][2]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_on[1][2]);
                sprintf(str_name, DASH_TIMERON_led3, tft_val,tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_on[1][3]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_on[1][3]);
                sprintf(str_name, DASH_TIMERON_led4, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_off[1][0]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_off[1][0]);
                sprintf(str_name, DASH_TIMEROFF_led1, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_off[1][1]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_off[1][1]);
                sprintf(str_name, DASH_TIMEROFF_led2, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_off[1][2]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_off[1][2]);
                sprintf(str_name, DASH_TIMEROFF_led3, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_off[1][3]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_off[1][3]);
                sprintf(str_name, DASH_TIMEROFF_led4, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[1][0], time_pg.pg_hour_off[1][0],
                                                 time_pg.pg_min_on[1][0], time_pg.pg_min_off[1][0]));
                sprintf(str_name, DASH_DUARATION_led1, tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[1][1], time_pg.pg_hour_off[1][1],
                                                 time_pg.pg_min_on[1][1], time_pg.pg_min_off[1][1]));
                sprintf(str_name, DASH_DUARATION_led2, tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[1][2], time_pg.pg_hour_off[1][2],
                                                 time_pg.pg_min_on[1][2], time_pg.pg_min_off[1][2]));
                sprintf(str_name, DASH_DUARATION_led3, tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[1][3], time_pg.pg_hour_off[1][3],
                                                 time_pg.pg_min_on[1][3], time_pg.pg_min_off[1][3]));
                sprintf(str_name, DASH_DUARATION_led4, tft_val);
                send_tft(str_name);

                break;
        case 3:

                sprintf(str_name, MODE, "AUTO");
                send_tft(str_name);
                sprintf(str_name, DASH_STAGE,"3");
                send_tft(str_name);

                sprintf(tft_val, "%d",time_pg.bright_1[2][0]);
                sprintf(str_name, DASH_BRIGHT_led1,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_2[2][0]);
                sprintf(str_name, DASH_BRIGHT_led2,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_3[2][0]);
                sprintf(str_name, DASH_BRIGHT_led3,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_4[2][0]);
                sprintf(str_name, DASH_BRIGHT_led4,tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d",time_pg.bright_1[2][1]);
                sprintf(str_name, DASH_BRIGHT_led5,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_2[2][1]);
                sprintf(str_name, DASH_BRIGHT_led6,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_3[2][1]);
                sprintf(str_name, DASH_BRIGHT_led7,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_4[2][1]);
                sprintf(str_name, DASH_BRIGHT_led8,tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d",time_pg.bright_1[2][2]);
                sprintf(str_name, DASH_BRIGHT_led9,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_2[2][2]);
                sprintf(str_name, DASH_BRIGHT_led10,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_3[2][2]);
                sprintf(str_name, DASH_BRIGHT_led11,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_4[2][2]);
                sprintf(str_name, DASH_BRIGHT_led12,tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d",time_pg.bright_1[2][3]);
                sprintf(str_name, DASH_BRIGHT_led13,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_2[2][3]);
                sprintf(str_name, DASH_BRIGHT_led14,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_3[2][3]);
                sprintf(str_name, DASH_BRIGHT_led15,tft_val);
                send_tft(str_name);
                sprintf(tft_val, "%d",time_pg.bright_4[2][3]);
                sprintf(str_name, DASH_BRIGHT_led16,tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_on[2][0]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_on[2][0]);
                sprintf(str_name, DASH_TIMERON_led1, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_on[2][1]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_on[2][1]);
                sprintf(str_name, DASH_TIMERON_led2, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_on[2][2]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_on[2][2]);
                sprintf(str_name, DASH_TIMERON_led3, tft_val,tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_on[2][3]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_on[2][3]);
                sprintf(str_name, DASH_TIMERON_led4, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_off[2][0]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_off[2][0]);
                sprintf(str_name, DASH_TIMEROFF_led1, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_off[2][1]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_off[2][1]);
                sprintf(str_name, DASH_TIMEROFF_led2, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_off[2][2]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_off[2][2]);
                sprintf(str_name, DASH_TIMEROFF_led3, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%02d",time_pg.pg_hour_off[2][3]);
                sprintf(tft_val2, "%02d",time_pg.pg_min_off[2][3]);
                sprintf(str_name, DASH_TIMEROFF_led4, tft_val, tft_val2);
                send_tft(str_name);

                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[2][0], time_pg.pg_hour_off[2][0],
                                                 time_pg.pg_min_on[2][0], time_pg.pg_min_off[2][0]));
                sprintf(str_name, DASH_DUARATION_led1, tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[2][1], time_pg.pg_hour_off[2][1],
                                                 time_pg.pg_min_on[2][1], time_pg.pg_min_off[2][1]));
                sprintf(str_name, DASH_DUARATION_led2, tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[2][2], time_pg.pg_hour_off[2][2],
                                                 time_pg.pg_min_on[2][2], time_pg.pg_min_off[2][2]));
                sprintf(str_name, DASH_DUARATION_led3, tft_val);
                send_tft(str_name);

                sprintf(tft_val, "%d", diff2time(time_pg.pg_hour_on[2][3], time_pg.pg_hour_off[2][3],
                                                 time_pg.pg_min_on[2][3], time_pg.pg_min_off[2][3]));
                sprintf(str_name, DASH_DUARATION_led4, tft_val);
                send_tft(str_name);

                break;
        }

}

static void scan_i2c()
{
        int i;
        esp_err_t espRc;
        printf("Starting scan\n");
        for (i = 3; i < 0x78; i++)
        {
                i2c_cmd_handle_t cmd = i2c_cmd_link_create();
                i2c_master_start(cmd);
                i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
                i2c_master_stop(cmd);

                espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_PERIOD_MS);
                if (espRc == 0)
                {
                        printf("Found device at I2C_NUM_0 ID 0x%.2x\n", i);
                }
                i2c_cmd_link_delete(cmd);
        }
        for (i = 3; i < 0x78; i++)
        {
                i2c_cmd_handle_t cmd = i2c_cmd_link_create();
                i2c_master_start(cmd);
                i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
                i2c_master_stop(cmd);

                espRc = i2c_master_cmd_begin(I2C_NUM_1, cmd, 100 / portTICK_PERIOD_MS);
                if (espRc == 0)
                {
                        printf("Found device at I2C_NUM_1 ID 0x%.2x\n", i);
                }
                i2c_cmd_link_delete(cmd);
        }
        printf("Scan complete \n");
}

static void temp_init()
{
        internal_temp_sensor = hdc1080_init_sensor(0, HDC1080_ADDR);
        external_temp_sensor = hdc1080_init_sensor(1, HDC1080_ADDR);
        hdc1080_set_resolution(internal_temp_sensor, hdc1080_14bit, hdc1080_14bit);
        hdc1080_set_resolution(external_temp_sensor, hdc1080_14bit, hdc1080_14bit);

        if (internal_temp_sensor && external_temp_sensor)
        {
                hdc1080_registers_t registers = hdc1080_get_registers(internal_temp_sensor);
                hdc1080_registers_t registers2 = hdc1080_get_registers(external_temp_sensor);
                printf("Initialized HDC1080 internal_temp_sensor: manufacurer 0x%x, device id 0x%x\n", hdc1080_get_manufacturer_id(internal_temp_sensor), hdc1080_get_device_id(internal_temp_sensor));
                printf("Initialized HDC1080 external_temp_sensor: manufacurer 0x%x, device2 id 0x%x\n", hdc1080_get_manufacturer_id(external_temp_sensor), hdc1080_get_device_id(external_temp_sensor));
                registers.acquisition_mode = 1;
                registers2.acquisition_mode = 1;
                hdc1080_set_registers(internal_temp_sensor, registers);
                hdc1080_set_registers(external_temp_sensor, registers2);
        }
        else
        {
                printf("Could not initialize HDC1080 1&2 internal_temp_sensor\n");
        }
}

#endif
