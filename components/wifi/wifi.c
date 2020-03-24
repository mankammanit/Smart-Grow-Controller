#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include "esp_spi_flash.h"
#include <string.h>
#include "cJSON.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "mqtt_client.h"
#include "esp_log.h"

#include "wifi.h"
#include "st_profile.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "uart_cm.h"
#include "main.h"
#include "feeding_cm.h"


//wifi
EventGroupHandle_t wifi_event_group;
wifi_config_t wifi_config = {};
system_event_t *event;


//mqtt
char mqtt_msg[512];
char topic_[10];
char subscribed_[10];
// static esp_mqtt_client_handle_t mqtt_thingboard;
static esp_mqtt_client_handle_t mqtt_cm;
static EventGroupHandle_t event_group;
const uint8_t MQTT_CONNECTED_EVENT = BIT2;
const uint8_t MQTT_DISCONNECTED_EVENT = BIT3;

const uint8_t WIFI_STA_CONNECTED_BIT = BIT0;
const uint8_t WIFI_STA_DISCONNECTED_BIT = BIT1;

wifi_scan_config_t scanConf = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false
};

// From auth_mode code to string
static char* getAuthModeName(wifi_auth_mode_t auth_mode) {

        char *names[] = {"OPEN", "WEP", "WPA PSK", "WPA2 PSK", "WPA WPA2 PSK", "MAX"};
        return names[auth_mode];
}

void notify_wifi_connected()
{
        xEventGroupClearBits(wifi_event_group, WIFI_STA_DISCONNECTED_BIT);
        xEventGroupSetBits(wifi_event_group, WIFI_STA_CONNECTED_BIT);
}

void notify_wifi_disconnected()
{
        xEventGroupClearBits(wifi_event_group, WIFI_STA_CONNECTED_BIT);
        xEventGroupSetBits(wifi_event_group, WIFI_STA_DISCONNECTED_BIT);
}

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)

{

        switch(event->event_id) {

        case SYSTEM_EVENT_STA_START:
                esp_wifi_connect();
                break;

        case SYSTEM_EVENT_STA_GOT_IP:
                notify_wifi_connected();
                ESP_LOGI("WIFI", "got ip:%s",
                         ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
                sprintf(topic_,CM_TOPIC,status_pg.mydevice_no);
                printf("topic_ : %s\n", topic_);
                esp_mqtt_client_start(mqtt_cm);
                break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
        {
                esp_wifi_connect();
                notify_wifi_disconnected();
                esp_mqtt_client_stop(mqtt_cm);
                break;
        }
        break;

        default:
                break;
        }
        return ESP_OK;
}


void initialise_wifi()
{
        ESP_LOGI("WIFI", "Initialising");
        sprintf(str_name,DEBUG_TFT,"WIFI Initialising");
        send_tft(str_name);

        wifi_event_group = xEventGroupCreate();
        tcpip_adapter_init();
        ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_get_config( WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));


        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        ESP_LOGI("WIFI", "Connenct SSID %s...", wifi_config.sta.ssid);
        sprintf(str_name,DEBUG_TFT,"Connenct WIFI...");
        send_tft(str_name);
}

void wifi_wait_connected()
{
        xEventGroupWaitBits(wifi_event_group, WIFI_STA_CONNECTED_BIT, false, true, portMAX_DELAY);
}

bool network_is_alive(void)
{
        EventBits_t uxBits = xEventGroupGetBits(wifi_event_group);
        if (uxBits & WIFI_STA_CONNECTED_BIT) {
                return true;
        } else {
                return false;
        }
}


void reconnect_wifi(char *ssid_tft,char *pass_tft)
{

        strcpy((char *)wifi_config.sta.ssid,(char *)ssid_tft);
        strcpy((char *)wifi_config.sta.password,(char *)pass_tft);

        ESP_ERROR_CHECK( esp_wifi_disconnect() );
        ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
        ESP_ERROR_CHECK( esp_wifi_connect() );
}

void tcip_info()
{

        if(network_is_alive() == true)
        {
                // print the local IP address
                tcpip_adapter_ip_info_t ip_info;
                tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);
                printf("IP Address:  %s\n", ip4addr_ntoa(&ip_info.ip));
                printf("Subnet mask: %s\n", ip4addr_ntoa(&ip_info.netmask));
                printf("Gateway:     %s\n", ip4addr_ntoa(&ip_info.gw));
                sprintf(str_name,SSID_TFT,wifi_config.sta.ssid);
                send_tft(str_name);
                sprintf(str_name,IP_TFT,ip4addr_ntoa(&ip_info.ip));
                send_tft(str_name);
                sprintf(str_name,SUBNET_TFT,ip4addr_ntoa(&ip_info.netmask));
                send_tft(str_name);
                sprintf(str_name,GW_TFT,ip4addr_ntoa(&ip_info.gw));
                send_tft(str_name);
        }
        else
        {
                // ESP_LOGI("INFO_WIFI", "Wi-Fi network connection missing");
                sprintf(str_name,DEBUG_TFT,"Wi-Fi network connection missing");
                send_tft(str_name);
        }
}

void call_rssi()
{
        wifi_ap_record_t wifidata;
        if (esp_wifi_sta_get_ap_info(&wifidata)==0) {
                // printf("wifi rssi:%d\r\n", wifidata.rssi);
                if(wifidata.rssi>-50)
                {
                        // printf("Excellent\n");
                        sprintf(str_name,MYRSSI,4);
                        send_tft(str_name);
                }
                if(wifidata.rssi<-50 && wifidata.rssi >-60)
                {
                        // printf("Good\n");
                        sprintf(str_name,MYRSSI,3);
                        send_tft(str_name);
                }
                if(wifidata.rssi<-60 && wifidata.rssi >-70)
                {
                        // printf("Fair\n");
                        sprintf(str_name,MYRSSI,2);
                        send_tft(str_name);
                }
                if(wifidata.rssi<-70)
                {
                        // printf("Weak\n");
                        sprintf(str_name,MYRSSI,1);
                        send_tft(str_name);
                }
        }
        else
        {
                // printf("Wifi No Connection\n");
                sprintf(str_name,MYRSSI,0);
                send_tft(str_name);
        }

}
void scan_wifi()
{
        sprintf(str_name,WAIT_WIFI,1);
        send_tft(str_name);
        esp_wifi_disconnect();
        esp_wifi_scan_start(&scanConf, true);
        char ssid_tft[64];
        uint16_t ap_num;
        esp_wifi_scan_get_ap_num(&ap_num);
        wifi_ap_record_t ap_records[ap_num];
        esp_wifi_scan_get_ap_records(
                &ap_num, ap_records
                );
        printf("================ SCAN WIFI ================\n");
        printf("%d Networks found:\n", ap_num); // %d Networks found
        printf(" Channel | RSSI | SSID | Auth_Mode \n");  // Channel | RSSI | SSID
        for(uint8_t i = 0; i < ap_num; i++)
        {

                printf(" %d | %d | %s | %s \n",
                       ap_records[i].primary,
                       ap_records[i].rssi,
                       (char *)ap_records[i].ssid,
                       getAuthModeName(ap_records[i].authmode)
                       );
                if(strcmp(getAuthModeName(ap_records[i].authmode), "OPEN") == 0)
                {
                        sprintf(str_name,AUTH_MODE_WIFI,i+1,0);
                        send_tft(str_name);
                        sprintf(ssid_tft,"%s",(char *)ap_records[i].ssid);
                        sprintf(str_name,SSID_SCAN,i+1,ssid_tft);
                        send_tft(str_name);
                        if(ap_records[i].rssi>-50)
                        {
                                // printf("Excellent\n");
                                sprintf(str_name,RSSI_SCAN,i+1,27);
                                send_tft(str_name);
                        }
                        if(ap_records[i].rssi<-50 && ap_records[i].rssi >-60)
                        {
                                // printf("Good\n");
                                sprintf(str_name,RSSI_SCAN,i+1,28);
                                send_tft(str_name);
                        }
                        if(ap_records[i].rssi<-60 && ap_records[i].rssi >-70)
                        {
                                // printf("Fair\n");
                                sprintf(str_name,RSSI_SCAN,i+1,29);
                                send_tft(str_name);
                        }
                        if(ap_records[i].rssi<-70)
                        {
                                // printf("Weak\n");
                                sprintf(str_name,RSSI_SCAN,i+1,30);
                                send_tft(str_name);
                        }
                }
                else
                {
                        sprintf(str_name,AUTH_MODE_WIFI,i+1,63488);
                        send_tft(str_name);
                        sprintf(ssid_tft,"%s",(char *)ap_records[i].ssid);
                        sprintf(str_name,SSID_SCAN,i+1,ssid_tft);
                        send_tft(str_name);
                        if(ap_records[i].rssi>-50)
                        {
                                // printf("Excellent\n");
                                sprintf(str_name,RSSI_SCAN,i+1,27);
                                send_tft(str_name);
                        }
                        if(ap_records[i].rssi<-50 && ap_records[i].rssi >-60)
                        {
                                // printf("Good\n");
                                sprintf(str_name,RSSI_SCAN,i+1,28);
                                send_tft(str_name);
                        }
                        if(ap_records[i].rssi<-60 && ap_records[i].rssi >-70)
                        {
                                // printf("Fair\n");
                                sprintf(str_name,RSSI_SCAN,i+1,29);
                                send_tft(str_name);
                        }
                        if(ap_records[i].rssi<-70)
                        {
                                // printf("Weak\n");
                                sprintf(str_name,RSSI_SCAN,i+1,30);
                                send_tft(str_name);
                        }
                }


        }
        printf("===========================================\n");
        sprintf(str_name,WAIT_WIFI,0);
        send_tft(str_name);
        esp_wifi_scan_stop();
        vTaskDelay( 5000 / portTICK_RATE_MS );
        esp_wifi_connect();

}


bool mqtt_is_alive(void)
{
        EventBits_t uxBits = xEventGroupGetBits(event_group);
        if (uxBits & MQTT_CONNECTED_EVENT) {
                return true;
        } else {
                return false;
        }
}
void removeChar(char *s, int c){

        int j, n = strlen(s);
        for (int i=j=0; i<n; i++)
                if (s[i] != c)
                        s[j++] = s[i];

        s[j] = '\0';
}

void esp_mqtt_publish_number(const char *topic, const double payload)
{
        if(network_is_alive() == true)
        {
                if(mqtt_is_alive()==true)
                {

                        cJSON *root = cJSON_CreateObject();
                        cJSON_AddNumberToObject(root, topic, payload);
                        char *post_data = cJSON_PrintUnformatted(root);
                        cJSON_Delete(root);

                        // printf("JSON: %s\n",post_data);
                        // esp_mqtt_client_publish(mqtt_cm, topic_, post_data, 0, 1, 0);
                        if(esp_mqtt_client_publish(mqtt_cm, topic_, post_data, 0, 1, 0));
                        else ESP_LOGE("MQTT_CM", "MQTT_PUBLISHED_ERROR");

                        removeChar(post_data, '{');
                        removeChar(post_data, '}');
                        removeChar(post_data, '"');

                        sprintf(str_name,DEBUG_TFT,post_data);
                        send_tft(str_name);
                        free(post_data);
                }
                else
                {
                        ESP_LOGI("MQTT_CM", "MQTT network connection missing");
                        sprintf(str_name,DEBUG_TFT,"MQTT network connection missing");
                        send_tft(str_name);
                }
        }
        else
        {
                ESP_LOGI("INFO_WIFI", "Wi-Fi network connection missing");
                sprintf(str_name,DEBUG_TFT,"Wi-Fi network connection missing");
                send_tft(str_name);
        }
}
void esp_mqtt_publish_string(const char *topic, const char *payload)
{
        if(network_is_alive() == true)
        {
                if(mqtt_is_alive()==true)
                {

                        cJSON *root = cJSON_CreateObject();
                        cJSON_AddStringToObject(root, topic, payload);
                        char *post_data = cJSON_PrintUnformatted(root);
                        cJSON_Delete(root);

                        // printf("JSON: %s\n",post_data);
                        // esp_mqtt_client_publish(mqtt_cm, topic_, post_data, 0, 1, 0);
                        if(esp_mqtt_client_publish(mqtt_cm, topic_, post_data, 0, 1, 0));
                        else ESP_LOGE("MQTT_CM", "MQTT_PUBLISHED_ERROR");

                        removeChar(post_data, '{');
                        removeChar(post_data, '}');
                        removeChar(post_data, '"');

                        sprintf(str_name,DEBUG_TFT,post_data);
                        send_tft(str_name);
                        free(post_data);
                }
                else
                {
                        ESP_LOGI("MQTT_CM", "MQTT network connection missing");
                        sprintf(str_name,DEBUG_TFT,"MQTT network connection missing");
                        send_tft(str_name);
                }
        }
        else
        {
                ESP_LOGI("INFO_WIFI", "Wi-Fi network connection missing");
                sprintf(str_name,DEBUG_TFT,"Wi-Fi network connection missing");
                send_tft(str_name);
        }
}

void publish_array_object(char *post_data)
{
  if(network_is_alive() == true)
  {
          if(mqtt_is_alive()==true)
          {

                  if(esp_mqtt_client_publish(mqtt_cm, topic_, post_data, 0, 1, 0));
                  else ESP_LOGE("MQTT_CM", "MQTT_PUBLISHED_ERROR");

                  removeChar(post_data, '{');
                  removeChar(post_data, '}');
                  removeChar(post_data, '"');

                  sprintf(str_name,DEBUG_TFT,post_data);
                  send_tft(str_name);
                  free(post_data);
          }
          else
          {
                  ESP_LOGI("MQTT_CM", "MQTT network connection missing");
                  sprintf(str_name,DEBUG_TFT,"MQTT network connection missing");
                  send_tft(str_name);
          }
  }
  else
  {
          ESP_LOGI("INFO_WIFI", "Wi-Fi network connection missing");
          sprintf(str_name,DEBUG_TFT,"Wi-Fi network connection missing");
          send_tft(str_name);
  }
}


static esp_err_t mqtt_event_handler_cm(esp_mqtt_event_handle_t event)
{
        assert(event != NULL);
        switch (event->event_id)
        {
        case MQTT_EVENT_CONNECTED:
                xEventGroupClearBits(event_group, MQTT_DISCONNECTED_EVENT);
                xEventGroupSetBits(event_group, MQTT_CONNECTED_EVENT);

                ESP_LOGI("MQTT_CM", "MQTT_EVENT_CONNECTED");
                sprintf(str_name,DEBUG_TFT,"MQTT_EVENT_CONNECTED");
                send_tft(str_name);

                sprintf(subscribed_,CM_SUBSCRIBE,status_pg.mydevice_no);
                printf("subscribed_ : %s\n", subscribed_);
                esp_mqtt_client_subscribe(mqtt_cm, subscribed_, 1);

                break;
        case MQTT_EVENT_DISCONNECTED:
                xEventGroupClearBits(event_group, MQTT_CONNECTED_EVENT);
                xEventGroupSetBits(event_group, MQTT_DISCONNECTED_EVENT);
                ESP_LOGE("MQTT_CM", "MQTT_EVENT_DISCONNECTED");
                sprintf(str_name,DEBUG_TFT,"MQTT_EVENT_DISCONNECTED");
                send_tft(str_name);
                break;
        case MQTT_EVENT_SUBSCRIBED:
                ESP_LOGI("MQTT_CM", "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
                break;
        case MQTT_EVENT_UNSUBSCRIBED:
                ESP_LOGE("MQTT_CM", "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
                break;
        case MQTT_EVENT_PUBLISHED:
                ESP_LOGI("MQTT_CM", "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
                sprintf(str_name,DEBUG_TFT,"MQTT_EVENT_PUBLISHED");
                send_tft(str_name);
                break;
        case MQTT_EVENT_DATA:

                ESP_LOGD("MQTT_CM", "MQTT_EVENT_DATA, msg_id=%d, %s", event->msg_id, event->topic);
                if (event->data_len >= (sizeof(mqtt_msg) - 1))
                {
                        ESP_LOGE("MQTT_CM", "Received MQTT message size [%d] more than expected [%d]", event->data_len, (sizeof(mqtt_msg) - 1));
                        return ESP_FAIL;
                }

                memcpy(mqtt_msg, event->data, event->data_len);
                mqtt_msg[event->data_len] = 0;
                cJSON *attributes = cJSON_Parse(mqtt_msg);
                // if (attributes != NULL)
                // {
                //         char* value_ = cJSON_GetObjectItem(attributes,"program_Stutas_Night")->valuestring;
                //         ESP_LOGI("MQTT_CM", "value : %s", value_);
                // }

                char *attributes_string = cJSON_Print(attributes);
                cJSON_Delete(attributes);
                ESP_LOGI("MQTT_CM", "Shared attributes response: %s", attributes_string);
                // esp_mqtt_client_publish(mqtt_thingboard, TB_TELEMETRY_TOPIC, attributes_string, 0, 1, 0);
                // Free is intentional, it's client responsibility to free the result of cJSON_Print
                free(attributes_string);

                break;
        case MQTT_EVENT_ERROR:
                ESP_LOGE("MQTT_CM", "MQTT_EVENT_ERROR");
                sprintf(str_name,DEBUG_TFT,"MQTT_EVENT_ERROR");
                send_tft(str_name);
                break;
        case MQTT_EVENT_BEFORE_CONNECT:
                ESP_LOGI("MQTT_CM", "MQTT_EVENT_BEFORE_CONNECT");
                sprintf(str_name,DEBUG_TFT,"MQTT_EVENT_BEFORE_CONNECT");
                send_tft(str_name);
                break;
        }
        return ESP_OK;
}

void initialise_mqtt_cm()
{
        event_group = xEventGroupCreate();

        ESP_LOGI("MQTT_CM", "MQTT_DEVICE_CM");
        const esp_mqtt_client_config_t mqtt_cfg_cm = {
                .uri = MQTT_CM_URL,
                .event_handle = mqtt_event_handler_cm,
                .port = MQTT_PORT,
                .username = MQTT_CM_ID,
                .password = MQTT_CM_PASS,
        };
        mqtt_cm = esp_mqtt_client_init(&mqtt_cfg_cm);
        esp_mqtt_client_start(mqtt_cm);
}
