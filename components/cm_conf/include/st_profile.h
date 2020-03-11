#ifndef __CMSTRUCT_H__
#define __CMSTRUCT_H__

/////////////////////////////////////////////
typedef struct
{
        uint8_t working_day;
        uint8_t working_lastday;
        uint8_t working_nextday;
        uint8_t working_on_h[4];
        uint8_t working_on_m[4];
        uint8_t working_off_h[4];
        uint8_t working_off_m[4];
        uint8_t status_timer[4];
} str_working;
str_working working_timer;

/////////////////////////////////////////////
typedef struct network_set
{
        char *ssid;
        char *pass;

} network_set;
network_set set_network;
/////////////////////////////////////////////

/////////////////////////////////////////////
typedef struct ratio_val
{
        //ค่าแสงแต่ละโซน [a]=zone
//bright_1 .. 2..3 ..4 คือ LED1 ..2 ..3..4
        uint8_t bright_1[4];
        uint8_t bright_2[4];
        uint8_t bright_3[4];
        uint8_t bright_4[4];

} ratio_val;
ratio_val ratio_led;
/////////////////////////////////////////////

/////////////////////////////////////////////
typedef struct ph_str_val
{
        float _temp_c;
        float _phValue;
        float _acidVoltage;    //buffer solution 4.0 at 25C
        float _neutralVoltage; //buffer solution 7.0 at 25C
        float _voltage;
        uint8_t _checkwrite;

} ph_str_val;
ph_str_val ph_val;
/////////////////////////////////////////////

typedef struct
{
        uint8_t ec_set_point;
        uint8_t ph_set_point;
        uint8_t ratio_fer[4];
} ferti_set;
ferti_set ferti_set_val;

/////////////////////////////////////////////
typedef struct ec_str_val
{
        float _voltoffset;

} ec_str_val;
ec_str_val ec_val;
/////////////////////////////////////////////

/////////////////////////////////////////////
//set time in program
typedef struct statuspg
{
        uint16_t start_day;
        uint8_t switch_mode;
        uint8_t OTA_NEXTION;
        uint8_t mydevice_no;
} statuspg;
statuspg status_pg;

typedef struct timepg
{
//STAGE1
// [a],[b] A=STAGE,B=ZONE
        //on
        uint8_t pg_hour_on[4][4];
        uint8_t pg_min_on[4][4];
        uint16_t dayon[4];

        //off
        uint8_t pg_hour_off[4][4];
        uint8_t pg_min_off[4][4];
        uint16_t dayoff[4];

        //ratio
        uint8_t bright_1[4][4];
        uint8_t bright_2[4][4];
        uint8_t bright_3[4][4];
        uint8_t bright_4[4][4];

} timepg;
timepg time_pg;

/////////////////////////////////////////////
typedef struct environment
{
        //water
        uint8_t fill1_state;
        uint8_t fill2_state;
        uint8_t fill3_state;
        uint8_t fill4_state;
        uint8_t pump_ph_state;
        uint8_t solenoide_state;
        uint8_t pump_water_state;

} environment;
environment _environment;
/////////////////////////////////////////////
#endif
