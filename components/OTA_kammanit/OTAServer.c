
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "string.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "String.h"
#include "cJSON.h"
#include "main.h"
#include "uart_cm.h"

// receive buffer
char rcv_buffer[200];

#if REVISION==1
#define UPDATE_JSON_BIN   "http://183.88.218.59:5090/fw_grow_controller/contain_ver.json"
#define UPDATE_JSON_TFT   "http://183.88.218.59:5090/fw_grow_controller/tft_ver.json"
#define DOWNLOAD_BIN      "http://183.88.218.59:5090/fw_grow_controller/fw_contain.bin"
#elif REVISION==2
#define UPDATE_JSON_BIN   "http://183.88.218.59:5090/fw_grow_controller_lab_b/contain_ver.json"
#define UPDATE_JSON_TFT   "http://183.88.218.59:5090/fw_grow_controller_lab_b/tft_ver.json"
#define DOWNLOAD_BIN      "http://183.88.218.59:5090/fw_grow_controller_lab_b/fw_contain.bin"
#else
#define UPDATE_JSON_BIN   "http://183.88.218.59:5090/fw_grow_controller_ver1/contain_ver.json"
#define UPDATE_JSON_TFT   "http://183.88.218.59:5090/fw_grow_controller_ver1/tft_ver.json"
#define DOWNLOAD_BIN      "http://183.88.218.59:5090/fw_grow_controller_ver1/fw_contain.bin"
#endif


//map file to percent
//return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min
int mapfile(int val, int max)
{
        return (val - 0) * (100 - 0) / (max - 0) + 0;
}

int cal_size = 0;
int max_size = 0;
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
        switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
                ESP_LOGE("ota_http", "HTTP_EVENT_ERROR");
                break;
        case HTTP_EVENT_ON_CONNECTED:
                ESP_LOGI("ota_http", "HTTP_EVENT_ON_CONNECTED");
                break;
        case HTTP_EVENT_HEADER_SENT:
                ESP_LOGI("ota_http", "HTTP_EVENT_HEADER_SENT");
                break;
        case HTTP_EVENT_ON_HEADER:
                ESP_LOGI("ota_http", "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
                break;
        case HTTP_EVENT_ON_DATA:
                ESP_LOGI("ota_http", "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
                if (!esp_http_client_is_chunked_response(evt->client)) {
                        strncpy(rcv_buffer, (char*)evt->data, evt->data_len);
                }
                break;
        case HTTP_EVENT_ON_FINISH:
                ESP_LOGI("ota_http", "HTTP_EVENT_ON_FINISH");
                break;
        case HTTP_EVENT_DISCONNECTED:
                ESP_LOGE("ota_http", "HTTP_EVENT_DISCONNECTED");
                break;
        }
        return ESP_OK;
}

esp_err_t _ota_event_handler(esp_http_client_event_t *evt)
{
        switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
                ESP_LOGE("ota_http", "HTTP_EVENT_ERROR");
                break;
        case HTTP_EVENT_ON_CONNECTED:
                ESP_LOGI("ota_http", "HTTP_EVENT_ON_CONNECTED");
                break;
        case HTTP_EVENT_HEADER_SENT:
                ESP_LOGI("ota_http", "HTTP_EVENT_HEADER_SENT");
                break;
        case HTTP_EVENT_ON_HEADER:
                ESP_LOGI("ota_http", "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
                if(strcmp(evt->header_key, "Content-Length") == 0)
                {
                        max_size = atoi(evt->header_value);
                }
                cal_size=0;
                break;
        case HTTP_EVENT_ON_DATA:
                ESP_LOGI("ota_http", "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
                cal_size = cal_size + evt->data_len;
                printf("percent %d:%d\n",cal_size,max_size);
                char val_[50];
                sprintf(val_,"Download %d:%d ",cal_size,max_size);
                sprintf(str_name,INFO_FW,val_,"Byte");
                send_tft(str_name);
                sprintf(str_name,PERCENT_FW,mapfile(cal_size,max_size));
                send_tft(str_name);
                break;
        case HTTP_EVENT_ON_FINISH:
                ESP_LOGI("ota_http", "HTTP_EVENT_ON_FINISH");
                break;
        case HTTP_EVENT_DISCONNECTED:
                ESP_LOGE("ota_http", "HTTP_EVENT_DISCONNECTED");
                break;
        }
        return ESP_OK;
}


void ota_http_task(void * pvParameter)
{
        ESP_LOGI("ota_http", "Starting OTA HTTP...");

        esp_http_client_config_t config = {
                .url = DOWNLOAD_BIN,
                .event_handler = _ota_event_handler,
        };

        esp_err_t ret = esp_https_ota(&config);
        if (ret == ESP_OK) {
                ESP_LOGI("ota_http", "Complete Wait Restart");
                esp_restart();
        } else {
                ESP_LOGE("ota_http", "Firmware Upgrades Failed");
        }
        while (1) {
                ESP_LOGI("ota_http", "Wait Update FW ESP32");
                vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
}

float read_ver_bin()
{
        esp_http_client_config_t config = {
                .url = UPDATE_JSON_BIN,
                .event_handler = _http_event_handler,
        };
        esp_http_client_handle_t client = esp_http_client_init(&config);

        // downloading the json file
        esp_err_t err = esp_http_client_perform(client);

        if(err == ESP_OK) {
                // parse the json file
                cJSON *json = cJSON_Parse(rcv_buffer);
                if(json == NULL) printf("downloaded file is not a valid json, aborting...\n");
                else {
                        cJSON *version = cJSON_GetObjectItemCaseSensitive(json, "ver_esp32");
                        // check the version
                        if(!cJSON_IsNumber(version)) printf("unable to read new version, aborting...\n");
                        else {
                                double new_version = version->valuedouble;
                                // cleanup
                                esp_http_client_cleanup(client);
                                return new_version;
                        }

                }
        }
        else{
                return 0.0;
        }
        return 0.0;
}

float read_ver_tft()
{
        esp_http_client_config_t config = {
                .url = UPDATE_JSON_TFT,
                .event_handler = _http_event_handler,
        };
        esp_http_client_handle_t client = esp_http_client_init(&config);

        // downloading the json file
        esp_err_t err = esp_http_client_perform(client);

        if(err == ESP_OK) {
                // parse the json file
                cJSON *json = cJSON_Parse(rcv_buffer);
                if(json == NULL) printf("downloaded file is not a valid json, aborting...\n");
                else {
                        cJSON *version = cJSON_GetObjectItemCaseSensitive(json, "ver_tft");
                        // check the version
                        if(!cJSON_IsNumber(version)) printf("unable to read new version, aborting...\n");
                        else {
                                double new_version = version->valuedouble;
                                // cleanup
                                esp_http_client_cleanup(client);
                                return new_version;
                        }

                }
        }
        else{
                return 0.0;
        }
        return 0.0;
}

void ota_http()
{
        xTaskCreate(&ota_http_task, "ota_http_task", 10240, NULL, 10, NULL);
}
