#include <stdio.h>
#include <string.h>
#include <math.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_heap_caps.h"
#include "uart_cm.h"


static void Configure_Uart(int uart_num, int tx_pin, int rx_pin,int buad) {
        uart_config_t uart_config = {
                .baud_rate = buad,
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_DISABLE,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                .rx_flow_ctrl_thresh = 122,
        };
        if (uart_param_config(uart_num, &uart_config) < 0) {
                ESP_LOGE("INIT_UART", "uart_param_config() error!");
        }
        if (uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE,
                         UART_PIN_NO_CHANGE) < 0) {
                ESP_LOGE("INIT_UART", "uart_set_pin() error!");
        }
        if (uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0) < 0) {
                ESP_LOGE("INIT_UART", "uart_driver_install() error!");
        }
}

void string2hexString(char* input, char* output)
{
        int loop;
        int i;

        i=0;
        loop=0;

        while(input[loop] != '\0')
        {
                sprintf((char*)(output+i),"%02X", input[loop]);
                loop+=1;
                i+=2;
        }
        //insert NULL at the end of the output string
        output[i++] = '\0';
}



char *readtft(){
// uint8_t length;
        static char message_tft[256];
        memset(message_tft, 0, sizeof(message_tft));
        const int TFT_BUF_SIZE = BUF_SIZE;
        esp_log_level_set("RX_TASK2", ESP_LOG_INFO);
        uint8_t* tft_data = (uint8_t*) malloc(TFT_BUF_SIZE+1);
        const int tft_rx = uart_read_bytes(MY_UART2, tft_data, TFT_BUF_SIZE, 10 / portTICK_RATE_MS);

        if (tft_rx > 0) {

                for(int i =0; i<=tft_rx; i++)
                {
                        tft_data[i]+=1;
                }
                tft_data[tft_rx] = 0;
                strcpy(message_tft, (char*)tft_data);
                // ESP_LOGI("MEMORY", "FREE_HEAP %d", esp_get_free_heap_size());
                // ESP_LOGI("RX_TASK2", "Read %d bytes: '%s'", tft_rx, tft_data);
                ESP_LOG_BUFFER_HEXDUMP("RX_TASK2", tft_data, tft_rx, ESP_LOG_INFO);
                // printf("READ TFT : %s \n", message_tft);
                // length = strlen(message_tft);
                // printf("Length of the string : %d\n", length);
        }
        free(tft_data);
        return message_tft;
}

char *readec(){
// uint8_t length;
        static char message_ec[32];
        memset(message_ec, 0, sizeof(message_ec));
        const int EC_BUF_SIZE = BUF_SIZE;
        esp_log_level_set("RX_TASK1", ESP_LOG_INFO);
        uint8_t* ec_data = (uint8_t*) malloc(EC_BUF_SIZE+1);
        const int ec_rx = uart_read_bytes(MY_UART1, ec_data, EC_BUF_SIZE, 20 / portTICK_RATE_MS);
        if (ec_rx > 0) {
                ec_data[ec_rx] = 0;
                strcpy(message_ec, (char*)ec_data);
                // ESP_LOGI("MEMORY", "FREE_HEAP %d", esp_get_free_heap_size());
                // ESP_LOGI("RX_TASK1", "Read %d bytes: '%s'", ec_rx, ec_data);
                // ESP_LOG_BUFFER_HEXDUMP("RX_TASK1", ec_data, ec_rx, ESP_LOG_INFO);
                // printf("%s ", message_ec);
                // length = strlen(message_ec);
                // printf("Length ofthe string : %d\n", length);
        }
        free(ec_data);
        return message_ec;
}


void init_tft()
{
        Configure_Uart(MY_UART2, TXD2_PIN, RXD2_PIN, 921600);
        ESP_LOGI("INIT_UART", "MY_UART2 = %d", MY_UART2);

}

void init_ec()
{
        Configure_Uart(MY_UART1, TXD1_PIN, RXD1_PIN, 9600);
        ESP_LOGI("INIT_UART", "MY_UART1 = %d", MY_UART1);
}

int send_tft(const char* data)
{
        const int len = strlen(data);
        const int txBytes = uart_write_bytes(MY_UART2, data, len);
        // printf("sendtft is %s\n",data);
        return txBytes;
}

int send_ec(const char* data)
{
        const int len = strlen(data);
        const int txBytes = uart_write_bytes(MY_UART1, data, len);
        // printf("sendec\n");
        return txBytes;
}
