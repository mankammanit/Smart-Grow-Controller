#ifndef __UART_CM_H__
#define __UART_CM_H__

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"


#define MY_UART1  UART_NUM_1
#define MY_UART2  UART_NUM_2

//บอร์ดเก่า
// #define TXD1_PIN  (GPIO_NUM_10)
// #define RXD1_PIN  (GPIO_NUM_9)

//แก้ขาใหม่
#define TXD1_PIN  (GPIO_NUM_14)
#define RXD1_PIN  (GPIO_NUM_27)

// ec v2
// #define TXD1_PIN  (GPIO_NUM_12)
// #define RXD1_PIN  (GPIO_NUM_13)

#define TXD2_PIN  (GPIO_NUM_17)
#define RXD2_PIN  (GPIO_NUM_16)

#define BUF_SIZE (1024)

int send_tft(const char* data);
int send_ec(const char* data);
char *readtft();
char *readec();
void init_tft();
void init_ec();

#endif
