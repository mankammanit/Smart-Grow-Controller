#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

//uart kammanit
#include "uart_cm.h"
#include "main.h"
#include "st_profile.h"
#include "nvs_storage.h"


#if REVISION==1
#define NEX_OTA_URL               "http://183.88.218.59:5090/fw_grow_controller/Container_1_0.tft"  /* URL to Nextion firmware */
#elif REVISION==2
#define NEX_OTA_URL               "http://183.88.218.59:5090/fw_grow_controller_lab_b/Container_1_0.tft"  /* URL to Nextion firmware */
#else
#define NEX_OTA_URL               "http://183.88.218.59:5090/fw_grow_controller_ver1/Container_1_0.tft"  /* URL to Nextion firmware */
#endif

#define NEX_ACK_TIMEOUT           500000                  /* Nextion ack timeout (500ms) */
#define MAX_HTTP_RECV_BUFFER      4096                    /* Http buffer, same as Nextion packet size (4096) */

static const char* TAG = "Nextion OTA";


/**
 * Returns data from UART to given buffer
 */
static int nex_ota_uart_read(uint8_t *uart_data, size_t size)
{
								if (!uart_data) return 0;

								memset(uart_data, 0, size);
								const int tft_rx = uart_read_bytes(MY_UART2, uart_data, size, 10 / portTICK_RATE_MS);

								// ESP_LOGI("RX_TASK2", "Read %d bytes: '%s'", tft_rx, uart_data);
								// ESP_LOG_BUFFER_HEXDUMP("RX_TASK2", uart_data, tft_rx, ESP_LOG_INFO);

								return tft_rx;
}

/**
 * Send command via UART to Nextion.
 */
static int nex_ota_uart_command(char *command)
{
								int len = 100;
								char str[len];
								sprintf(str, "%s\xFF\xFF\xFF", command);

								return uart_write_bytes(MY_UART2, str, strlen(str));
}

/**
 * Read from UART and wait for Nextion acknowledge.
 * This is "0x05" when sending packets.
 */
static int nex_ota_uart_wait_ack(int ack)
{
								int ret = 0;
								const int NEXTION_SIZE = 1024;
								uint8_t *uart_data = (uint8_t *) malloc(NEXTION_SIZE);
								if(!uart_data)
								{
																ESP_LOGE(TAG, "Cannot malloc http uart buffer");
																return ret;
								}

								int64_t time = esp_timer_get_time();


								while (esp_timer_get_time() < time + NEX_ACK_TIMEOUT)
								{
																if (nex_ota_uart_read(uart_data, NEXTION_SIZE) && uart_data[0] == ack) {
																								ret = 1;
																								break;
																}
								}

								free(uart_data);

								return ret;
}

/**
 * Send the upload command, and wait for Nextion ack response.
 */
static int nex_ota_upload_command(int filesize, int baud)
{
								char cmd[100];
								sprintf(cmd, "whmi-wri %d,%d,0", filesize, baud);

								nex_ota_uart_command("");
								nex_ota_uart_command(cmd);

								vTaskDelay(100 / portTICK_PERIOD_MS);

								return nex_ota_uart_wait_ack(0x05);
}

void nex_ota_config_default(esp_http_client_config_t *config)
{
								config->url = 0;
								config->host = 0;
								config->port = 0;
								config->username = 0;
								config->password = 0;
								config->auth_type = HTTP_AUTH_TYPE_NONE;
								config->path = 0;
								config->query = 0;
								config->cert_pem = 0;
								config->method = HTTP_METHOD_GET;
								config->timeout_ms = 0;
								config->disable_auto_redirect = 0;
								config->max_redirection_count = 0;
								config->event_handler = 0;
								config->transport_type = HTTP_TRANSPORT_UNKNOWN;
								config->buffer_size = 0;
								config->user_data = 0;
								config->is_async = 0;
}

static esp_http_client_handle_t nex_ota_http_request()
{
								esp_err_t err;
								esp_http_client_config_t config;
								nex_ota_config_default(&config);
								config.url = NEX_OTA_URL;

								// create client
								esp_http_client_handle_t client = esp_http_client_init(&config);

								esp_http_client_set_header(client, "Content-Type", "application/json");
								//esp_http_client_set_header(client, "Authorization", " Bearer $2y$10$9ij0QHtsrwRDVS53IznV1e95Av.psw53AzRcZ96AMCj.yEeRW/6SO");

								if ((err = esp_http_client_open(client, 0)) != ESP_OK) {
																ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
																return 0;
								}

								return client;
}

static void nex_ota_clean_http_client(esp_http_client_handle_t client)
{
								esp_http_client_close(client);
								esp_http_client_cleanup(client);
}

static int nex_ota_send_bytes(esp_http_client_handle_t client, int content_length)
{
								int ret            = 1;
								int total_read_len = 0;
								int read_len       = 0;

								char *buffer = (char*)malloc(MAX_HTTP_RECV_BUFFER + 1);
								if (buffer == NULL) {
																ESP_LOGE(TAG, "Cannot malloc http receive buffer");
																return 0;
								}

								while (total_read_len < content_length)
								{
																read_len = esp_http_client_read(client, buffer, MAX_HTTP_RECV_BUFFER);
																if (read_len <= 0)
																{
																								ESP_LOGE(TAG, "Error read data");
																								ret = 0;
																								break;
																}

																uart_write_bytes(MY_UART2, (const char*)buffer, read_len);

																if(!nex_ota_uart_wait_ack(0x05))
																{
																								ESP_LOGE(TAG, "Ack timeout!");
																								ret = 0;
																								break;
																}

																total_read_len += read_len;
																ESP_LOGI(TAG, "DOWNLOAD %d:%d\n",total_read_len,content_length);
								}

								free(buffer);

								return ret;
}

/**
 * Perform HTTP request and download the firmware into the Nextion.
 */
int nex_ota_upload()
{
								esp_http_client_handle_t client = nex_ota_http_request();
								if (!client) {
																nex_ota_clean_http_client(client);
																return 0;
								}

								int content_length = esp_http_client_fetch_headers(client);

								if (!nex_ota_upload_command(content_length, 921600))
								{
																ESP_LOGE(TAG, "Error sending command!");
																nex_ota_clean_http_client(client);
																return 0;
								}

								if (!nex_ota_send_bytes(client, content_length))
								{
																ESP_LOGE(TAG, "Error sending bytes!");
																nex_ota_clean_http_client(client);
																return 0;
								}

								nex_ota_clean_http_client(client);
								return 1;
}
