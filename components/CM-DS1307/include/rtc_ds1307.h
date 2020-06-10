#ifndef __RTC_DS1307_H__
#define __RTC_DS1307_H__

#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lwip/sockets.h>
#include <stdio.h>
#include <time.h>
#include "sdkconfig.h"
#include "main.h"
#include "uart_cm.h"

#define DS1307_ADDRESS     0x68    /*!< address for DS1307 */
#define SDA_PIN 18
#define SCL_PIN 19
#define I2C_DS1307_NUM I2C_NUM_1
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE   0   /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE   0   /*!< I2C master do not need buffer */


#define CH_BIT      (1 << 7)
#define HOUR12_BIT  (1 << 6)
#define PM_BIT      (1 << 5)
#define SQWE_BIT    (1 << 4)
#define OUT_BIT     (1 << 7)

#define CH_MASK      0x7f
#define SECONDS_MASK 0x7f
#define HOUR12_MASK  0x1f
#define HOUR24_MASK  0x3f
#define SQWEF_MASK   0x03
#define SQWE_MASK    0xef
#define OUT_MASK     0x7f

#define READ_TIMEZONE 0
#define WRITE_TIMEZONE 1

void ds1307_init(void);
esp_err_t resetDS1307(void);
esp_err_t readValue(struct tm *time);
esp_err_t writeValue(const struct tm *time);
void tft_set_time(uint16_t set_y , uint8_t set_mo , uint8_t set_d
                  , uint8_t set_h , uint8_t set_m ,uint8_t set_s);
bool betweenTimes(int start_hour, int end_hour,
                  int start_minute, int end_minute);
uint8_t diff2time(int start_hour, int end_hour,
                  int start_minute, int end_minute);
#endif
