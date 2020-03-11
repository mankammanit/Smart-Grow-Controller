/*
   wifi.h - Wi-Fi setup routines

   This file is part of the ESP32 Everest Run project
   https://github.com/krzychb/esp32-everest-run

   Copyright (c) 2016 Krzysztof Budzynski <krzychb@gazeta.pl>
   This work is licensed under the Apache License, Version 2.0, January 2004
   See the file LICENSE for details.
 */
#ifndef WIFI_H
#define WIFI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/event_groups.h"

extern EventGroupHandle_t wifi_event_group;
extern const uint8_t WIFI_STA_CONNECTED_BIT;
extern char mqtt_msg[512];



void initialise_wifi();
void reconnect_wifi(char *ssid_tft,char *pass_tft);
void tcip_info();
void scan_wifi();
void stop_wifi();
void call_rssi();
void wifi_wait_connected();
bool network_is_alive();



//extern mqtt to global
#define MQTT_URL "mqtt://cloud.thingsboard.io"
#define MQTT_PORT 1883
//CONTAIN_MQTT_TOKEN
#define THINGBOARD_TOKEN    "Token_Container%03d"
//CONTAIN_LINK_DASHBOARD
#define CONTAIN_DASHBOARD "http://cm-smartgrow.com"
#define TB_TELEMETRY_TOPIC "v1/devices/me/telemetry"
#define TB_ATTRIBUTES_TOPIC "v1/devices/me/attributes"
#define TB_ATTRIBUTES_SUBSCRIBE_TO_RESPONSE_TOPIC "v1/devices/me/rpc/request/+"
//mqtt cm forward to THINGBOARD
#define MQTT_CM_URL "mqtt://13.228.40.81"
#define MQTT_CM_ID "test_01"
#define MQTT_CM_PASS "12345678"
#define CM_TOPIC "/mycanabis%03d"
#define CM_SUBSCRIBE "/container%03d"
// void initialise_mqtt(void);
void initialise_mqtt_cm(void);
bool mqtt_is_alive(void);
void esp_mqtt_publish_string(const char *topic, const char *payload);
void esp_mqtt_publish_number(const char *topic, const double payload);

#ifdef __cplusplus
}
#endif

#endif
