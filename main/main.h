#ifndef __MAIN_H__
#define __MAIN_H__

#define FIRMWARE_ESP32 1.6

//task ec
TaskHandle_t task_1;
//task tft
TaskHandle_t task_2;
//task sch
TaskHandle_t task_3;
//task read_sensor_all
TaskHandle_t task_4;



//timer watchdog
#define WATCHDOG_TIMEOUT_S 90
#define CHECK_ERROR_CODE(returned, expected) ({ \
                if (returned != expected)               \
                {                                       \
                        printf("TWDT ERROR\n");         \
                        abort();                        \
                }                                       \
        })
////////////////gpio pca9685_1/////////////
// | ฟ้า  | เหลือง| เขียว | แดง  |  ดำ  |
// | GND | PWM1 | PWM2 | PWM3 | PWM4 |
// | GND | Red  | Blue | White| NA   |
#define ZONE1_LED1 0
#define ZONE1_LED2 1
#define ZONE1_LED3 2
#define ZONE1_LED4 3

#define ZONE2_LED1 4
#define ZONE2_LED2 5
#define ZONE2_LED3 6
#define ZONE2_LED4 7

#define ZONE3_LED1 8
#define ZONE3_LED2 9
#define ZONE3_LED3 10
#define ZONE3_LED4 11

#define ZONE4_LED1 12
#define ZONE4_LED2 13
#define ZONE4_LED3 14
#define ZONE4_LED4 15

////////////////////////////////////////////
////////////////gpio pca9685_2//////////////
#define SOLENOIDE   0
#define PUMP_A      1
#define PUMP_B      2
#define PUMP_C      3
#define PUMP_D      4
#define PUMP_PH     5
#define DRAIN       6
#define ON_OFF_P1   7
#define ON_OFF_P2   8
#define AIR_PUMP    9
#define FAN_PWM     10
#define CHILLER     11
#define PWM12CH2    12
#define PWM13CH2    13
#define PWM14CH2    14
#define PWM15CH2    15

///////////////////////////////////////////
//LOW = pin2 = ดำจากถัง
//MEDIUM = pin15 = เขียวจากถัง
// HIGH = pin4 = น้ำเงินจากถัง
#define WATER_INPUT_LV1 2
#define WATER_INPUT_LV2 15
#define WATER_INPUT_LV3 4
#define GPIO_INPUT_PIN_SEL ((1ULL << WATER_INPUT_LV1) | (1ULL << WATER_INPUT_LV2) | (1ULL << WATER_INPUT_LV3))
/////////////////////////////////////////////
///////////////////// LCD ///////////////////
char dtmp[256];
char ec_uart[32];
char str_name[256];
#define TFTEND                  "\xFF\xFF\xFF"
#define TFT_REST                "rest" TFTEND
#define SAVE_EEPROM_TFT         "wepo %d,%d" TFTEND
#define PAGE_TFT                "page %s" TFTEND
#define CONNECT_TFT             "connect" TFTEND

#define RTC0                    "rtc0=%d" TFTEND
#define RTC1                    "rtc1=%d" TFTEND
#define RTC2                    "rtc2=%d" TFTEND
#define RTC3                    "rtc3=%d" TFTEND
#define RTC4                    "rtc4=%d" TFTEND
#define RTC5                    "rtc5=%d" TFTEND

#define CALL_H                  "n2.val=%d" TFTEND
#define CALL_M                  "n3.val=%d" TFTEND
#define CALL_S                  "n4.val=%d" TFTEND
#define CALL_D                  "n0.val=%d" TFTEND
#define CALL_MO                 "n1.val=%d" TFTEND
#define CALL_Y                  "n5.val=%d" TFTEND

//DASH_BOARD status
#define WATER_TEMP_SEND         "Dashboard.d_wtemp.txt=\"%s\"" TFTEND
#define EC_SEND                 "Dashboard.d_ec.txt=\"%s\"" TFTEND
#define PH_SEND                 "Dashboard.d_ph.txt=\"%s\"" TFTEND
#define START_TIME              "Dashboard.d_date_plant.txt=\"%s\"" TFTEND
#define STATUS_DAY              "Dashboard.d_day.txt=\"%s\"" TFTEND
#define LOOPTIME                "Dashboard.d_dayloop.txt=\"%s\"" TFTEND
#define PUMP_TIME1              "Dashboard.d_tpump1.txt=\"%s:%s-%s:%s\"" TFTEND
#define PUMP_TIME2              "Dashboard.d_tpump2.txt=\"%s:%s-%s:%s\"" TFTEND
#define PUMP_TIME3              "Dashboard.d_tpump3.txt=\"%s:%s-%s-%s\"" TFTEND
#define PUMP_TIME4              "Dashboard.d_tpump4.txt=\"%s:%s-%s-%s\"" TFTEND
#define MODE                    "Dashboard.d_mode.txt=\"%s\"" TFTEND
#define MODE_COLOR              "Dashboard.d_mode.pco=%d" TFTEND

#define DASH_TIMERON_led1       "Dashboard.d_timeon1.txt=\"%s:%s\"" TFTEND
#define DASH_TIMERON_led2       "Dashboard.d_timeon2.txt=\"%s:%s\"" TFTEND
#define DASH_TIMERON_led3       "Dashboard.d_timeon3.txt=\"%s:%s\"" TFTEND
#define DASH_TIMERON_led4       "Dashboard.d_timeon4.txt=\"%s:%s\"" TFTEND

#define DASH_TIMEROFF_led1      "Dashboard.d_timeoff1.txt=\"%s:%s\"" TFTEND
#define DASH_TIMEROFF_led2      "Dashboard.d_timeoff2.txt=\"%s:%s\"" TFTEND
#define DASH_TIMEROFF_led3      "Dashboard.d_timeoff3.txt=\"%s:%s\"" TFTEND
#define DASH_TIMEROFF_led4      "Dashboard.d_timeoff4.txt=\"%s:%s\"" TFTEND

#define DASH_DUARATION_led1     "Dashboard.d_dura1.txt=\"%s\"" TFTEND
#define DASH_DUARATION_led2     "Dashboard.d_dura2.txt=\"%s\"" TFTEND
#define DASH_DUARATION_led3     "Dashboard.d_dura3.txt=\"%s\"" TFTEND
#define DASH_DUARATION_led4     "Dashboard.d_dura4.txt=\"%s\"" TFTEND

#define DASH_BRIGHT_led1        "Dashboard.d_led1.txt=\"%s\"" TFTEND
#define DASH_BRIGHT_led2        "Dashboard.d_led2.txt=\"%s\"" TFTEND
#define DASH_BRIGHT_led3        "Dashboard.d_led3.txt=\"%s\"" TFTEND
#define DASH_BRIGHT_led4        "Dashboard.d_led4.txt=\"%s\"" TFTEND

#define DASH_BRIGHT_led5        "Dashboard.d_led5.txt=\"%s\"" TFTEND
#define DASH_BRIGHT_led6        "Dashboard.d_led6.txt=\"%s\"" TFTEND
#define DASH_BRIGHT_led7        "Dashboard.d_led7.txt=\"%s\"" TFTEND
#define DASH_BRIGHT_led8        "Dashboard.d_led8.txt=\"%s\"" TFTEND

#define DASH_BRIGHT_led9        "Dashboard.d_led9.txt=\"%s\"" TFTEND
#define DASH_BRIGHT_led10       "Dashboard.d_led10.txt=\"%s\"" TFTEND
#define DASH_BRIGHT_led11       "Dashboard.d_led11.txt=\"%s\"" TFTEND
#define DASH_BRIGHT_led12       "Dashboard.d_led12.txt=\"%s\"" TFTEND

#define DASH_BRIGHT_led13       "Dashboard.d_led13.txt=\"%s\"" TFTEND
#define DASH_BRIGHT_led14       "Dashboard.d_led14.txt=\"%s\"" TFTEND
#define DASH_BRIGHT_led15       "Dashboard.d_led15.txt=\"%s\"" TFTEND
#define DASH_BRIGHT_led16       "Dashboard.d_led16.txt=\"%s\"" TFTEND

#define DASH_SOLENOIDE          "Dashboard.d_mode_wfill.txt=\"%s\"" TFTEND
#define DASH_WATERPUMP          "Dashboard.d_mode_wpump.txt=\"%s\"" TFTEND
#define WATERLV                 "Dashboard.d_w_level.val=%d" TFTEND

#define loopTime_tft            "Dashboard.d_dayloop.txt=\"%s\"" TFTEND
#define timer_on_tft            "Dashboard.d_onpump.txt=\"%s:%s\"" TFTEND
#define timer_off_tft           "Dashboard.d_offpump.txt=\"%s:%s\"" TFTEND

#define DASH_TPUMP1            "Dashboard.time1.pic=%d" TFTEND
#define DASH_TPUMP2            "Dashboard.time2.pic=%d" TFTEND
#define DASH_TPUMP3            "Dashboard.time3.pic=%d" TFTEND
#define DASH_TPUMP4            "Dashboard.time4.pic=%d" TFTEND

//ratio menu
#define TXT_ZONE1_LED1          "Light_Main.z1_led1.val=%d" TFTEND
#define TXT_ZONE1_LED2          "Light_Main.z1_led2.val=%d" TFTEND
#define TXT_ZONE1_LED3          "Light_Main.z1_led3.val=%d" TFTEND
#define TXT_ZONE1_LED4          "Light_Main.z1_led4.val=%d" TFTEND

#define TXT_ZONE2_LED1          "Light_Main.z2_led1.val=%d" TFTEND
#define TXT_ZONE2_LED2          "Light_Main.z2_led2.val=%d" TFTEND
#define TXT_ZONE2_LED3          "Light_Main.z2_led3.val=%d" TFTEND
#define TXT_ZONE2_LED4          "Light_Main.z2_led4.val=%d" TFTEND

#define TXT_ZONE3_LED1          "Light_Main.z3_led1.val=%d" TFTEND
#define TXT_ZONE3_LED2          "Light_Main.z3_led2.val=%d" TFTEND
#define TXT_ZONE3_LED3          "Light_Main.z3_led3.val=%d" TFTEND
#define TXT_ZONE3_LED4          "Light_Main.z3_led4.val=%d" TFTEND

#define TXT_ZONE4_LED1          "Light_Main.z4_led1.val=%d" TFTEND
#define TXT_ZONE4_LED2          "Light_Main.z4_led2.val=%d" TFTEND
#define TXT_ZONE4_LED3          "Light_Main.z4_led3.val=%d" TFTEND
#define TXT_ZONE4_LED4          "Light_Main.z4_led4.val=%d" TFTEND

//ratio manaul
#define SET_ZONE_LED1           "h0.val=%d" TFTEND
#define SET_ZONE_LED2           "h1.val=%d" TFTEND
#define SET_ZONE_LED3           "h2.val=%d" TFTEND
#define SET_ZONE_LED4           "h3.val=%d" TFTEND

//page program stage
#define DAY_PLANTED             "L_auto.n12.val=%d" TFTEND
//program stage1
#define STAGE_1_ON              "st1on.val=%d" TFTEND
#define STAGE_1_OFF             "st1off.val=%d" TFTEND
#define ON_OFF_1                "onoff1.txt=\"%s:%s-%s:%s\"" TFTEND
#define DUARATION_1             "dura1.txt=\"%s\"" TFTEND

#define ZONE1_led1              "n0.val=%d" TFTEND
#define ZONE1_led2              "n4.val=%d" TFTEND
#define ZONE1_led3              "n8.val=%d" TFTEND
#define ZONE1_led4              "n14.val=%d" TFTEND

#define ZONE2_led1              "n1.val=%d" TFTEND
#define ZONE2_led2              "n5.val=%d" TFTEND
#define ZONE2_led3              "n9.val=%d" TFTEND
#define ZONE2_led4              "n15.val=%d" TFTEND

#define ZONE3_led1              "n2.val=%d" TFTEND
#define ZONE3_led2              "n6.val=%d" TFTEND
#define ZONE3_led3              "n10.val=%d" TFTEND
#define ZONE3_led4              "n16.val=%d" TFTEND

#define ZONE4_led1              "n3.val=%d" TFTEND
#define ZONE4_led2              "n7.val=%d" TFTEND
#define ZONE4_led3              "n11.val=%d" TFTEND
#define ZONE4_led4              "n17.val=%d" TFTEND

//program stage2
#define STAGE_2_ON              "st2on.val=%d" TFTEND
#define STAGE_2_OFF             "st2off.val=%d" TFTEND
#define ON_OFF_2                "onoff2.txt=\"%s:%s-%s:%s\"" TFTEND
#define DUARATION_2             "dura2.txt=\"%s\"" TFTEND
#define PG2_ZONE1               "n4.val=%d" TFTEND
#define PG2_ZONE2               "n5.val=%d" TFTEND
#define PG2_ZONE3               "n6.val=%d" TFTEND
#define PG2_ZONE4               "n7.val=%d" TFTEND

//program stage3
#define STAGE_3_ON              "st3on.val=%d" TFTEND
#define STAGE_3_OFF             "st3off.val=%d" TFTEND
#define ON_OFF_3                "onoff3.txt=\"%s:%s-%s:%s\"" TFTEND
#define DUARATION_3             "dura3.txt=\"%s\"" TFTEND

#define ON_OFF_4                "onoff4.txt=\"%s:%s-%s:%s\"" TFTEND
#define DUARATION_4             "dura4.txt=\"%s\"" TFTEND
#define PG3_ZONE1               "n8.val=%d" TFTEND
#define PG3_ZONE2               "n9.val=%d" TFTEND
#define PG3_ZONE3               "n10.val=%d" TFTEND
#define PG3_ZONE4               "n11.val=%d" TFTEND

//program timestage
#define START_D                 "n0.val=%d" TFTEND
#define END_D                   "n1.val=%d" TFTEND
#define ON_HR                   "n2.val=%d" TFTEND
#define ON_MIN                  "n3.val=%d" TFTEND
#define ON_SEC                  "n4.val=%d" TFTEND
#define OFF_HR                  "n5.val=%d" TFTEND
#define OFF_MIN                 "n6.val=%d" TFTEND
#define OFF_SEC                 "n7.val=%d" TFTEND

//status of program
#define STATUS_PG1              "L_auto.stage1.pic=%d" TFTEND //16(on),17(off)
#define STATUS_PG2              "L_auto.stage2.pic=%d" TFTEND //16(on),17(off)
#define STATUS_PG3              "L_auto.stage3.pic=%d" TFTEND //16(on),17(off)

//pump control
#define LOOPTIME_               "Set_pump.n0.val=%d" TFTEND
#define TIMERON_H               "Set_pump.n1.val=%d" TFTEND
#define TIMERON_M               "Set_pump.n2.val=%d" TFTEND
#define TIMEROFF_H              "Set_pump.n3.val=%d" TFTEND
#define TIMEROFF_M              "Set_pump.n4.val=%d" TFTEND

//environment state
//pic id 40,on 41,off 42,auto
#define FILL_STATE_A            "p0.pic=%d" TFTEND
#define FILL_STATE_B            "p1.pic=%d" TFTEND
#define FILL_STATE_C            "p2.pic=%d" TFTEND
#define FILL_STATE_D            "p3.pic=%d" TFTEND
#define PUMP_STATE_PH           "p4.pic=%d" TFTEND
#define SOLENOIDE_STATE         "p5.pic=%d" TFTEND
#define PUMP_WATER_STATE        "p6.pic=%d" TFTEND

//network
#define SSID_TFT                "t5.txt=\"%s\"" TFTEND
#define IP_TFT                  "t6.txt=\"%s\"" TFTEND
#define SUBNET_TFT              "t7.txt=\"%s\"" TFTEND
#define GW_TFT                  "t8.txt=\"%s\"" TFTEND
#define SSID_SCAN               "ssid%d.txt=\"%s\"" TFTEND
#define RSSI_SCAN               "rssi%d.pic=%d" TFTEND //27=4 28=3 29=2 30=1
#define AUTH_MODE_WIFI          "ssid%d.pco=%d" TFTEND
#define MYRSSI                  "Loading.checkwifi.val=%d" TFTEND
#define WAIT_WIFI               "wait.val=%d" TFTEND

//FERTILIZER
#define INFO_BUFF_PH            "t0.txt=\"PH %s\"" TFTEND
#define INFO_BUFF_PH_2          "t0.txt=\"NOW %s\"" TFTEND
#define INFO_BUFF_PH_3          "t0.txt=\"Analog %s\"" TFTEND
#define INFO_BUFF_EC            "t0.txt=\"EC %s\"" TFTEND
#define INFO_BUFF_EC_2          "t0.txt=\"OFFSET %s\"" TFTEND
#define INFO_BUFF_EC_3          "t0.txt=\"NOW %s\"" TFTEND
#define INFO_BUFF_EC_4          "t0.txt=\"Analog %s\"" TFTEND
#define PER_BUFF                "j1.val=%d" TFTEND

#define fer_LOOPTIME_           "Fertilizer.n4.val=%d" TFTEND
#define fer_TIMERON_H           "Fertilizer.n5.val=%d" TFTEND
#define fer_TIMERON_M           "Fertilizer.n6.val=%d" TFTEND
#define fer_TIMEROFF_H          "Fertilizer.n7.val=%d" TFTEND
#define fer_TIMEROFF_M          "Fertilizer.n8.val=%d" TFTEND
#define fer_TIMERON_H2          "Fertilizer.n9.val=%d" TFTEND
#define fer_TIMERON_M2          "Fertilizer.n10.val=%d" TFTEND
#define fer_TIMEROFF_H2         "Fertilizer.n11.val=%d" TFTEND
#define fer_TIMEROFF_M2         "Fertilizer.n12.val=%d" TFTEND
#define fer_TIMERON_H3          "Fertilizer.n13.val=%d" TFTEND
#define fer_TIMERON_M3          "Fertilizer.n14.val=%d" TFTEND
#define fer_TIMEROFF_H3         "Fertilizer.n15.val=%d" TFTEND
#define fer_TIMEROFF_M3         "Fertilizer.n16.val=%d" TFTEND
#define fer_TIMERON_H4          "Fertilizer.n17.val=%d" TFTEND
#define fer_TIMERON_M4          "Fertilizer.n18.val=%d" TFTEND
#define fer_TIMEROFF_H4         "Fertilizer.n19.val=%d" TFTEND
#define fer_TIMEROFF_M4         "Fertilizer.n20.val=%d" TFTEND
#define FER_TPUMP1              "Fertilizer.time1.pic=%d" TFTEND
#define FER_TPUMP2              "Fertilizer.time2.pic=%d" TFTEND
#define FER_TPUMP3              "Fertilizer.time3.pic=%d" TFTEND
#define FER_TPUMP4              "Fertilizer.time4.pic=%d" TFTEND


#define VERSION_FW              "x0.val=%d" TFTEND
#define INFO_FW                 "t1.txt=\"%s%s\"" TFTEND
#define INSTALL_FW              "vis p0,%d" TFTEND
#define PERCENT_FW              "j0.val=%d" TFTEND
#define WAIT_OTA                "wait.val=%d" TFTEND
//page SET_FERTILIZER
#define SET_EC_POINT            "x0.val=%d" TFTEND
#define SET_PH_POINT            "x1.val=%d" TFTEND
#define SET_RATIO_A             "n0.val=%d" TFTEND
#define SET_RATIO_B             "n1.val=%d" TFTEND
#define SET_RATIO_C             "n2.val=%d" TFTEND
#define SET_RATIO_D             "n3.val=%d" TFTEND
#define SET_EC_POINT_           "Set_fer.x0.val=%d" TFTEND
#define SET_PH_POINT_           "Set_fer.x1.val=%d" TFTEND
#define SET_RATIO_A_            "Set_fer.n0.val=%d" TFTEND
#define SET_RATIO_B_            "Set_fer.n1.val=%d" TFTEND
#define SET_RATIO_C_            "Set_fer.n2.val=%d" TFTEND
#define SET_RATIO_D_            "Set_fer.n3.val=%d" TFTEND

#define SET_STATUS_TPUMP1       "Set_pump.sw0.val=%d" TFTEND
#define SET_STATUS_TPUMP2       "Set_pump.sw1.val=%d" TFTEND
#define SET_STATUS_TPUMP3       "Set_pump.sw2.val=%d" TFTEND
#define SET_STATUS_TPUMP4       "Set_pump.sw3.val=%d" TFTEND


//DEBUG
#define DEVICE_NO               "Debug.mqtt_device.val=%d" TFTEND
#define QR_CODE                 "qr0.txt=\"%s\"" TFTEND
#define DEBUG_TFT               "Debug.debug.txt=\"%s\"" TFTEND

//transfer firmware tft
#define TW_FILE                 "twfile \"sd0/fw.tft\",%d" TFTEND
#define RD_FILE                 "rdfile \"sd0/fw.tft\",0,10,10" TFTEND

#define PLAY_SOUND              "play %d,%d,%d" TFTEND
//line <x1>,<y1>,<x2>,<y2>,<color>
#define DRAW_SPECTRUM           "line %d,%d,%d,%d,%d" TFTEND
//picq <destx>,<desty>,<w>,<h>,<picid>
#define CLEAR_SPECTRUM          "ref 0" TFTEND

enum tft_cmd
{
        //hex ต้องห้าม
        //hex 01 ถึง 1f ห้ามใช้
        //hex return form tft
        Escape_Character                          = 0x20,
        Variable_name_too_long                    = 0x23,
        Serial_Buffer_Overflow                    = 0x24,
        Touch_Event                               = 0x65,
        Current_Page_Number                       = 0x66,
        Touch_Coordinate_awake                    = 0x67,
        Touch_Coordinate_sleep                    = 0x68,
        String_Data_Enclosed                      = 0x70,
        Numeric_Data_Enclosed                     = 0x71,
        Auto_Entered_Sleep_Mode                   = 0x86,
        Auto_Wake_from_Sleep                      = 0x87,
        Nextion_Ready                             = 0x88,
        Start_microSD_Upgrade                     = 0x89,
        Transparent_Data_Finished                 = 0xfd,
        Transparent_Data_Ready                    = 0xfe,

        PAGE_LOADING                              = 0x12,
        DASH_BOARD                                = 0x11,
        PAGE_HOME                                 = 0x10,

        //manual
        PAGE_RATIO                                = 0x2d,
        RATIO                                     = 0x2e,

        //auto
        // L_auto
        PAGE_PROGRAM                              = 0x2f,
        // L_zone
        PAGE_ZONE                                 = 0x3c,
        SAVE_DAY_ZONE                             = 0x3d,
        //start plant
        ONOFF_PROGRAM                             = 0x2c,
        SET_PROGRAM                               = 0x25,
        //manual , auto
        STATUS_PROGRAM                            = 0x26,
        PAGE_RATIO_PG                             = 0x27,
        PAGE_RATIO_MENU                           = 0x64,
        SET_RATIO_PG                              = 0x28,
        SAVE_RATIO_PG                             = 0x29,
        PAGE_TIMG_PG                              = 0x2a,
        SAVE_TIMG_PG                              = 0x2b,

        PAGE_FERTILIZER                           = 0x30,

        SET_PUMPCONTROL                           = 0x32,
        SET_FERTILIZER                            = 0x3a,
        PAGE_SET_FERTILIZER                       = 0x3b,

        PAGE_ENVIRONMENT                          = 0x40,
        SET_ENVIRONMENT                           = 0x41,

        PAGE_SETTING                              = 0x50,
        ERASE_FLASH                               = 0x51,
        ESP32_TFT_RESTART                         = 0x58,
        PAGE_NETWORK                              = 0x52,
        PAGE_DATE                                 = 0x53,
        SET_DATE                                  = 0x54,
        PAGE_FW                                   = 0x55,

        UPDATE_ESP32                              = 0x5a,
        UPDATE_TFT                                = 0x5b,

        SCAN_NETWORK                              = 0x56,
        CONNECT_NETWORK                           = 0x57,

        PAGE_CALIBRATE_EC                         = 0x60,
        SET_CALIBRATE_EC                          = 0x61,
        PAGE_CALIBRATE_PH                         = 0x62,
        SET_CALIBRATE_PH                          = 0x63,

        PAGE_COMMUNITY                            = 0x7a,

        PAGE_DEBUG                                = 0x90,
        DEBUG_SETMQTT                             = 0x91,

};

#endif
