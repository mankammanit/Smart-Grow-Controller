#include "feeding_cm.h"

int hourtomin = 60, mintosec = 60;
time_t on_timestamp;
time_t loop_timestamp;
time_t current_stamp;
char setpoint_[5];
struct tm *timeinfo;
#define BUF_LEN 10
char buf[BUF_LEN] = {0};

bool send_after_connect = false;
uint8_t water_pump_try_send = 0;

bool start_timer_contain = true;
uint8_t contain_pg = 0;   //pg1,2,3
uint8_t contain_mode = 0; //day=1,night=2,unplant=0

bool loop_dosing = false;
float ph_check_err;

uint8_t call_water_lv()
{
        if (!gpio_get_level(WATER_INPUT_LV1) && gpio_get_level(WATER_INPUT_LV2) && gpio_get_level(WATER_INPUT_LV3))
        {
                return 1;
        }
        else if (!gpio_get_level(WATER_INPUT_LV1) && !gpio_get_level(WATER_INPUT_LV2) && gpio_get_level(WATER_INPUT_LV3))
        {
                return 2;
        }
        else if (!gpio_get_level(WATER_INPUT_LV1) && !gpio_get_level(WATER_INPUT_LV2) && !gpio_get_level(WATER_INPUT_LV3))
        {
                return 3;
        }
        else if (gpio_get_level(WATER_INPUT_LV1) && gpio_get_level(WATER_INPUT_LV2) && gpio_get_level(WATER_INPUT_LV3))
        {
                return 0;
        }
        return 0;
}

bool start_dosing_ec = false;
bool start_dosing_ph = false;
expression_t expression_task = fer_ok;

long buf_ontimeA,
     buf_ontimeB,
     buf_ontimeC,
     buf_ontimeD,
     buf_ontimepH,
     buf_wating_time,
     buf_wating_time_ph;

uint8_t on_timeA = 1,
    on_timeB = 1,
    on_timeC = 1,
    on_timeD = 1,
    on_timepH = 1,
    wating_time = 1,
    wating_time_ph = 1;

//ec
float ec_value = 0.0;
float ec_setpoint = 1.2;
//ph
float ph_value = 0.0;
float ph_setpoint = 6.8;
//offset
float below_setpoint_ec = 0.15;
float below_setpoint_ph = 0.15;

float ec_diff, ph_diff;

uint8_t ratio_time[4];
uint8_t ratio_time_ph;

time_t feed_stamp;

bool check_water_pump_on_fill_time;

static void ph_in_timer();
static bool check_ec_fill();

void task_feeding_all()
{
        // printf("\n################### FEEDING #######################\n");
        // printf("Feeding stamp ph : %ld,%ld,%ld\n", feed_stamp,buf_ontimepH,buf_wating_time_ph);
        // printf("Feeding stamp ec : %ld,%ld,%ld,%ld,%ld,%ld\n", feed_stamp,buf_ontimeA,buf_ontimeB,
        //        buf_ontimeC,buf_ontimeD,buf_wating_time);
        // printf("###################################################\n\n");
        // printf("EC setpoint->%0.2f", ec_setpoint);
        // printf(" value->%0.2f", ec_value);
        // printf(" diff->%0.2f>%0.2f\n", ec_setpoint - ec_value, below_setpoint_ec);
        // printf("Debug ratio a:b:c:d->%d:%d:%d:%d\n", ratio_time[0], ratio_time[1],
        //        ratio_time[2], ratio_time[3]);
        // printf("PH setpoint->%0.1f", ph_setpoint);
        // printf(" value->%0.1f", ph_value);
        // printf(" diff->%0.1f>%0.1f\n", ph_value - ph_setpoint, below_setpoint_ph);
        switch (expression_task)
        {
        case dosing_ec:
                // printf("Debug---> case dosing ec\n");
                // printf("Debug ec_setpoint--->%0.2f\n", ec_setpoint);
                // printf("Debug ec_value--->%0.2f\n", ec_value);
                // printf("Debug ratio a:b:c:d--->%d:%d:%d:%d\n", ratio_time[0],ratio_time[1],
                // ratio_time[2],ratio_time[3]);
                if ((ec_value < ec_setpoint && (ec_setpoint - ec_value) > below_setpoint_ec) || loop_dosing == true)
                {
                        // printf("Debug ec_value < ec_setpoint\n");
                        // printf("start_dosing_ec --> %d\n",start_dosing_ec);
                        if (start_dosing_ec)
                        {
                                if (_environment.fill1_state == MODE_AUTO)
                                {

                                        // printf("Start Dosing!\n");
                                        // pump A on
                                        // printf("Fer A ON\n");
                                        SETFILL(PUMP_A, true,"PUMP_A_ON");
                                        esp_mqtt_publish_number("PUMP_A", 1);

                                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");

                                }
                                else
                                {
                                        // printf("Fer A NO MODE AUTO ---> goto Fer B\n");
                                        start_dosing_ec = false;
                                        loop_dosing = true;
                                        goto pump_b;
                                }

                                if (ratio_time[0] == DISABLE)
                                {
                                        // printf("Fer A Ratio = 0 ---> goto Fer B\n");
                                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                        esp_mqtt_publish_number("PUMP_A", 0);

                                        start_dosing_ec = false;
                                        loop_dosing = true;
                                        goto pump_b;
                                }
                                else
                                {
                                        buf_ontimeA = feed_stamp + (on_timeA * ratio_time[0]);
                                }

                                start_dosing_ec = false;
                                loop_dosing = true;
                                // printf("%s", asctime(localtime(&feed_stamp)));
                                // printf("Feedstamp: %ld\n\n", feed_stamp);
                        }

                        else if (feed_stamp >= buf_ontimeA && buf_ontimeA != 0)
                        {
pump_b:
                                // pump A off
                                if (_environment.fill1_state == MODE_AUTO)
                                {
                                        // printf("Fer A OFF\n");
                                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                        esp_mqtt_publish_number("PUMP_A", 0);

                                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");

                                }
                                // else printf("Fer A NO MODE AUTO\n");

                                // pump B on
                                if (_environment.fill2_state == MODE_AUTO)
                                {
                                        // printf("Fer B ON\n");
                                        SETFILL(PUMP_B, true,"PUMP_B_ON");
                                        esp_mqtt_publish_number("PUMP_B", 1);

                                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                                }
                                else
                                {
                                        // printf("Fer B NO MODE AUTO ---> goto Fer C\n");
                                        buf_ontimeA = 0;
                                        goto pump_c;
                                }

                                if (ratio_time[1] == DISABLE)
                                {
                                        // printf("Fer B Ratio = 0 ---> goto Fer C\n");
                                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                        esp_mqtt_publish_number("PUMP_B", 0);

                                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");

                                        buf_ontimeA = 0;
                                        goto pump_c;
                                }
                                else
                                {
                                        // printf("Fer B BUFF\n");
                                        buf_ontimeA = 0;
                                        buf_ontimeB = feed_stamp + (on_timeB * ratio_time[1]);
                                }
                                // printf("%s", asctime(localtime(&feed_stamp)));
                                // printf("Feedstamp: %ld\n\n", feed_stamp);
                        }

                        else if (feed_stamp >= buf_ontimeB && buf_ontimeB != 0)
                        {
pump_c:
                                // pump B off
                                if (_environment.fill2_state == MODE_AUTO)
                                {
                                        // printf("Fer B OFF\n");
                                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                        esp_mqtt_publish_number("PUMP_B", 0);

                                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");

                                }
                                // else printf("Fer B NO MODE AUTO\n");

                                // pump C on
                                if (_environment.fill3_state == MODE_AUTO)
                                {
                                        // printf("Fer C ON \n");
                                        SETFILL(PUMP_C, true,"PUMP_C_ON");
                                        esp_mqtt_publish_number("PUMP_C", 1);

                                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                                }
                                else
                                {
                                        // printf("Fer C NO MODE AUTO ---> goto Fer D\n");
                                        buf_ontimeB = 0;
                                        goto pump_d;
                                }

                                if (ratio_time[2] == DISABLE)
                                {
                                        // printf("Fer C Ratio = 0 ---> goto Fer D\n");
                                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                        esp_mqtt_publish_number("PUMP_C", 0);

                                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                                        buf_ontimeB = 0;
                                        goto pump_d;
                                }
                                else
                                {
                                        buf_ontimeB = 0;
                                        buf_ontimeC = feed_stamp + (on_timeC * ratio_time[2]);
                                }

                                // printf("%s", asctime(localtime(&feed_stamp)));
                                // printf("Feedstamp: %ld\n\n", feed_stamp);
                        }

                        else if (feed_stamp >= buf_ontimeC && buf_ontimeC != 0)
                        {
pump_d:
                                // pump C off
                                if (_environment.fill3_state == MODE_AUTO)
                                {
                                        // printf("Fer C OFF\n");
                                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                        esp_mqtt_publish_number("PUMP_C", 0);

                                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                                }
                                // else printf("Fer C NO MODE AUTO\n");

                                // pump D on
                                if (_environment.fill4_state == MODE_AUTO)
                                {
                                        // printf("Fer D ON \n");
                                        SETFILL(PUMP_D, true,"PUMP_D_ON");
                                        esp_mqtt_publish_number("PUMP_D", 1);

                                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");

                                }
                                else
                                {
                                        // printf("Fer D NO MODE AUTO ---> goto wait\n");
                                        buf_ontimeC = 0;
                                        goto wait_time;
                                }

                                if (ratio_time[3] == DISABLE)
                                {
                                        // printf("Fer D ratio = 0 ---> goto wait\n");
                                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                        esp_mqtt_publish_number("PUMP_D", 0);

                                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                                        buf_ontimeC = 0;
                                        goto wait_time;
                                }
                                else
                                {
                                        buf_ontimeC = 0;
                                        buf_ontimeD = feed_stamp + (on_timeD * ratio_time[3]);
                                }

                                // printf("%s", asctime(localtime(&feed_stamp)));
                                // printf("Feedstamp: %ld\n\n", feed_stamp);
                        }

                        else if (feed_stamp >= buf_ontimeD && buf_ontimeD != 0)
                        {
wait_time:
                                // pump D off
                                if (_environment.fill3_state == MODE_AUTO)
                                {
                                        // printf("Fer D OFF\n");
                                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                        esp_mqtt_publish_number("PUMP_D", 0);

                                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                                }
                                // else printf("Fer D NO MODE AUTO\n");

                                // wating time
                                // printf("Wating Time...\n");
                                buf_ontimeD = 0;
                                buf_wating_time = feed_stamp + wating_time;
                                // printf("%s", asctime(localtime(&feed_stamp)));
                                // printf("Feedstamp: %ld\n\n", feed_stamp);
                        }

                        else if (feed_stamp >= buf_wating_time && buf_wating_time != 0)
                        {
                                // printf("Start Dosing Again!\n");

                                expression_task = fer_ok;

                                start_dosing_ec = true;
                                start_dosing_ph = true;
                                loop_dosing = false;

                                SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                                esp_mqtt_publish_number("PUMP_PH", 0);
                                SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                esp_mqtt_publish_number("PUMP_A", 0);
                                SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                esp_mqtt_publish_number("PUMP_B", 0);
                                SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                esp_mqtt_publish_number("PUMP_C", 0);
                                SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                esp_mqtt_publish_number("PUMP_D", 0);

                                // printf("%s", asctime(localtime(&feed_stamp)));
                                // printf("Feedstamp: %ld\n\n", feed_stamp);
                                buf_ontimeA = 0;
                                buf_ontimeB = 0;
                                buf_ontimeC = 0;
                                buf_ontimeD = 0;
                                buf_wating_time = 0;
                        }
                }
                else
                {
                        // printf("Debug ec_value > ec_setpoint\n");
                        expression_task = fer_ok;

                        start_dosing_ec = true;
                        start_dosing_ph = true;

                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                        esp_mqtt_publish_number("PUMP_PH", 0);
                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                        esp_mqtt_publish_number("PUMP_A", 0);
                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                        esp_mqtt_publish_number("PUMP_B", 0);
                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                        esp_mqtt_publish_number("PUMP_C", 0);
                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                        esp_mqtt_publish_number("PUMP_D", 0);

                        buf_ontimeA = 0;
                        buf_ontimeB = 0;
                        buf_ontimeC = 0;
                        buf_ontimeD = 0;
                        buf_wating_time = 0;
                }

                break;

        case dosing_ph:
                // printf("Debug---> case dosing ph\n");
                // printf("Debug ph_setpoint--->%0.2f\n", ph_setpoint);
                // printf("Debug ph_value--->%0.2f\n", ph_value);
                if (ph_value > ph_setpoint && (ph_value - ph_setpoint) > below_setpoint_ph)
                {
                        // printf("Debug ph_value > ph_setpoint\n");
                        // printf("start_dosing_ph --> %d\n",start_dosing_ph);
                        if (start_dosing_ph)
                        {
                                ph_check_err = ph_value;
                                if (_environment.pump_ph_state == MODE_AUTO)
                                {
                                        // printf("Start Dosing!\n");
                                        // pump ph on
                                        // printf("PH ON\n");
                                        SETFILL(PUMP_PH, true,"PUMP_PH_ON");
                                        esp_mqtt_publish_number("PUMP_PH", 1);

                                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                }
                                // else printf("PH NO MODE AUTO\n");

                                buf_ontimepH = feed_stamp + (on_timepH*ratio_time_ph);
                                start_dosing_ph = false;
                        }
                        else if (feed_stamp >= buf_ontimepH && buf_ontimepH != 0)
                        {
                                if (_environment.pump_ph_state == MODE_AUTO)
                                {
                                        // pump ph off
                                        // printf("PH OFF\n");
                                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                                        esp_mqtt_publish_number("PUMP_PH", 0);

                                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                }
                                // else printf("PH NO MODE AUTO\n");
                                buf_ontimepH = 0;
                                buf_wating_time_ph = feed_stamp + wating_time_ph;

                        }
                        else if (feed_stamp >= buf_wating_time_ph && buf_wating_time_ph != 0)
                        {
                                // printf("Start Dosing Again!\n");

                                expression_task = fer_ok;

                                start_dosing_ec = true;
                                start_dosing_ph = true;

                                SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                                esp_mqtt_publish_number("PUMP_PH", 0);
                                SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                esp_mqtt_publish_number("PUMP_A", 0);
                                SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                esp_mqtt_publish_number("PUMP_B", 0);
                                SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                esp_mqtt_publish_number("PUMP_C", 0);
                                SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                esp_mqtt_publish_number("PUMP_D", 0);

                                // printf("%s", asctime(localtime(&feed_stamp_ph)));
                                // printf("Feedstamp: %ld\n\n", feed_stamp_ph);
                                buf_ontimepH = 0;
                                buf_wating_time_ph = 0;
                        }
                }
                else
                {
                        // printf("Debug ph_value < ph_setpoint\n");
                        expression_task = fer_ok;

                        start_dosing_ec = true;
                        start_dosing_ph = true;

                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                        esp_mqtt_publish_number("PUMP_PH", 0);
                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                        esp_mqtt_publish_number("PUMP_A", 0);
                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                        esp_mqtt_publish_number("PUMP_B", 0);
                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                        esp_mqtt_publish_number("PUMP_C", 0);
                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                        esp_mqtt_publish_number("PUMP_D", 0);

                        buf_ontimepH = 0;
                        buf_wating_time_ph = 0;
                }

                break;

        case dosing_ph_time:

                if (betweenTimes(ferti_set_val.ph_working_on_h[0], ferti_set_val.ph_working_off_h[0],
                                 ferti_set_val.ph_working_on_m[0], ferti_set_val.ph_working_off_m[0]))
                {
                        if(ferti_set_val.ph_status_timer[0] == ENABLE)
                        {
                                ph_in_timer();

                        }
                }
                else if (betweenTimes(ferti_set_val.ph_working_on_h[1], ferti_set_val.ph_working_off_h[1],
                                      ferti_set_val.ph_working_on_m[1], ferti_set_val.ph_working_off_m[1]))
                {
                        if(ferti_set_val.ph_status_timer[1] == ENABLE)
                        {
                                ph_in_timer();
                        }
                }
                else if (betweenTimes(ferti_set_val.ph_working_on_h[2], ferti_set_val.ph_working_off_h[2],
                                      ferti_set_val.ph_working_on_m[2], ferti_set_val.ph_working_off_m[2]))
                {
                        if(ferti_set_val.ph_status_timer[2] == ENABLE)
                        {
                                ph_in_timer();
                        }
                }
                else if (betweenTimes(ferti_set_val.ph_working_on_h[3], ferti_set_val.ph_working_off_h[3],
                                      ferti_set_val.ph_working_on_m[3], ferti_set_val.ph_working_off_m[3]))
                {
                        if(ferti_set_val.ph_status_timer[3] == ENABLE)
                        {
                                ph_in_timer();
                        }
                }

                break;

        case fer_ok:
                // printf("Debug---> case fer_ok\n");
                //return ec to task feeding
                ec_setpoint = ferti_set_val.ec_set_point/10.0;
                //return ph to task feeding
                ph_setpoint = ferti_set_val.ph_set_point/10.0;
                //return ratio a:b:c:d to task feeding
                for(uint8_t i = 0; i<4; i++)
                {
                        ratio_time[i]=ferti_set_val.ratio_fer[i];
                }
                //return ratio ph to task feeding
                ratio_time_ph = ferti_set_val.ratio_ph;
                //return waiting to task feeding
                wating_time = ferti_set_val.wait_ec;
                wating_time_ph = ferti_set_val.wait_ph;


                if (ec_value < ec_setpoint &&
                    (ec_setpoint - ec_value) > below_setpoint_ec &&
                    ec_setpoint!=0.0 && check_ec_fill())
                {
                        //         printf("EC setpoint->%0.2f", ec_setpoint);
                        //         printf(" value->%0.2f", ec_value);
                        //         printf(" diff->%0.2f>%0.2f\n", ec_setpoint - ec_value, below_setpoint_ec);
                        // printf("Debug ratio a:b:c:d->%d:%d:%d:%d\n", ratio_time[0], ratio_time[1],
                        //        ratio_time[2], ratio_time[3]);
                        printf("-------------------------------------------------------Dosing A,B,C,D Now!\n");
                        if(check_ec_fill())
                        {
                                expression_task = dosing_ec;
                                SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                esp_mqtt_publish_number("PUMP_A", 0);
                                SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                esp_mqtt_publish_number("PUMP_B", 0);
                                SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                esp_mqtt_publish_number("PUMP_C", 0);
                                SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                esp_mqtt_publish_number("PUMP_D", 0);
                                SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                                esp_mqtt_publish_number("PUMP_PH", 0);
                        }
                        else printf("-------------------------------------------------------EC (NO AUTO)\n");
                }

                else if (ph_value > ph_setpoint && (ph_value - ph_setpoint) > below_setpoint_ph && ph_setpoint!=0.0)
                {
                        // printf("PH setpoint->%0.1f", ph_setpoint);
                        // printf(" value->%0.1f", ph_value);
                        // printf(" diff->%0.1f>%0.1f\n", ph_value - ph_setpoint, below_setpoint_ph);
                        printf("-------------------------------------------------------Dosing PH Now!\n");
                        if (_environment.pump_ph_state == MODE_AUTO)
                        {
                                expression_task = dosing_ph;
                                printf("-------------------------------------------------------Dosing PH Now! (MODE AUTO)\n");
                                SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                                esp_mqtt_publish_number("PUMP_PH", 0);
                                SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                esp_mqtt_publish_number("PUMP_A", 0);
                                SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                esp_mqtt_publish_number("PUMP_B", 0);
                                SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                esp_mqtt_publish_number("PUMP_C", 0);
                                SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                esp_mqtt_publish_number("PUMP_D", 0);
                        }
                        else if (_environment.pump_ph_state == MODE_TIME)
                        {
                                printf("-------------------------------------------------------Dosing PH Now! (MODE TIME)\n");
                                if(check_water_pump_on_fill_time)
                                {
                                        expression_task = dosing_ph_time;
                                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                                        esp_mqtt_publish_number("PUMP_PH", 0);
                                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                        esp_mqtt_publish_number("PUMP_A", 0);
                                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                        esp_mqtt_publish_number("PUMP_B", 0);
                                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                        esp_mqtt_publish_number("PUMP_C", 0);
                                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                                        esp_mqtt_publish_number("PUMP_D", 0);
                                }
                                else printf("-------------------------------------------------------Dosing PH Now! (MODE TIME NOT WORK)\n");

                        }

                        else printf("-------------------------------------------------------PH (NO AUTO)\n");
                }

                break;

        default:
                break;
        }
}

static void ph_in_timer()
{
        if (ph_value > ph_setpoint && (ph_value - ph_setpoint) > below_setpoint_ph)
        {

                if (start_dosing_ph)
                {
                        ph_check_err = ph_value;
                        if (_environment.pump_ph_state == MODE_TIME)
                        {
                                // printf("Start Dosing!\n");
                                // pump ph on
                                // printf("PH ON\n");
                                SETFILL(PUMP_PH, true,"PUMP_PH_ON");
                                esp_mqtt_publish_number("PUMP_PH", 1);
                                SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                SETFILL(PUMP_D, false,"PUMP_D_OFF");
                        }
                        buf_ontimepH = feed_stamp + (on_timepH*ratio_time_ph);
                        start_dosing_ph = false;
                }
                else if (feed_stamp >= buf_ontimepH && buf_ontimepH != 0)
                {
                        if (_environment.pump_ph_state == MODE_TIME)
                        {
                                // pump ph off
                                // printf("PH OFF\n");
                                SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                                esp_mqtt_publish_number("PUMP_PH", 0);
                                SETFILL(PUMP_A, false,"PUMP_A_OFF");
                                SETFILL(PUMP_B, false,"PUMP_B_OFF");
                                SETFILL(PUMP_C, false,"PUMP_C_OFF");
                                SETFILL(PUMP_D, false,"PUMP_D_OFF");
                        }
                        // else printf("PH NO MODE AUTO\n");
                        buf_ontimepH = 0;
                        buf_wating_time_ph = feed_stamp + wating_time_ph;

                }
                else if (feed_stamp >= buf_wating_time_ph && buf_wating_time_ph != 0)
                {
                        // printf("Start Dosing Again!\n");

                        expression_task = fer_ok;

                        start_dosing_ec = true;
                        start_dosing_ph = true;

                        SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                        esp_mqtt_publish_number("PUMP_PH", 0);
                        SETFILL(PUMP_A, false,"PUMP_A_OFF");
                        esp_mqtt_publish_number("PUMP_A", 0);
                        SETFILL(PUMP_B, false,"PUMP_B_OFF");
                        esp_mqtt_publish_number("PUMP_B", 0);
                        SETFILL(PUMP_C, false,"PUMP_C_OFF");
                        esp_mqtt_publish_number("PUMP_C", 0);
                        SETFILL(PUMP_D, false,"PUMP_D_OFF");
                        esp_mqtt_publish_number("PUMP_D", 0);

                        // printf("%s", asctime(localtime(&feed_stamp_ph)));
                        // printf("Feedstamp: %ld\n\n", feed_stamp_ph);
                        buf_ontimepH = 0;
                        buf_wating_time_ph = 0;
                }
        }
        else
        {
                // printf("Debug ph_value < ph_setpoint\n");
                expression_task = fer_ok;

                start_dosing_ec = true;
                start_dosing_ph = true;

                SETFILL(PUMP_PH, false,"PUMP_PH_OFF");
                esp_mqtt_publish_number("PUMP_PH", 0);
                SETFILL(PUMP_A, false,"PUMP_A_OFF");
                esp_mqtt_publish_number("PUMP_A", 0);
                SETFILL(PUMP_B, false,"PUMP_B_OFF");
                esp_mqtt_publish_number("PUMP_B", 0);
                SETFILL(PUMP_C, false,"PUMP_C_OFF");
                esp_mqtt_publish_number("PUMP_C", 0);
                SETFILL(PUMP_D, false,"PUMP_D_OFF");
                esp_mqtt_publish_number("PUMP_D", 0);

                buf_ontimepH = 0;
                buf_wating_time_ph = 0;
        }
}

static bool check_ec_fill()
{
        if(_environment.fill1_state == MODE_AUTO) return true;
        else if(_environment.fill2_state == MODE_AUTO) return true;
        else if(_environment.fill3_state == MODE_AUTO) return true;
        else if(_environment.fill4_state == MODE_AUTO) return true;
        else if(_environment.fill1_state != MODE_AUTO&&
                _environment.fill2_state != MODE_AUTO&&
                _environment.fill3_state != MODE_AUTO&&
                _environment.fill4_state != MODE_AUTO) return false;
        else return false;

}
