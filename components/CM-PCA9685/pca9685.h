/***************************************************
   This is a library for the PCA9685 LED PWM Driver

   This chip is connected via I2C, 2 pins are required to interface. The PWM frequency is set for all pins, the PWM for each individually. The set PWM is active as long as the chip is powered.

   Written by Jonas Scharpf <jonas@brainelectronics.de>
   BSD license, all text above must be included in any redistribution
 ****************************************************/

#ifndef PCA9685_DRIVER_H
#define PCA9685_DRIVER_H

#include <stdint.h>
#include <driver/i2c.h>
#include <math.h>
#include "esp_err.h"
#include <errno.h>
#include "esp_log.h"
#include "esp_system.h"
#include <freertos/FreeRTOS.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define I2C_EXAMPLE_MASTER_SCL_IO   22    /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO   21    /*!< gpio number for I2C master data  */

#define I2C_EXAMPLE_MASTER_FREQ_HZ  100000     /*!< I2C master clock frequency */
#define I2C_EXAMPLE_MASTER_NUM      I2C_NUM_0   /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE   0   /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE   0   /*!< I2C master do not need buffer */

#define I2C_ADDRESS_1     0x41    /*!< address for PCA9685 */
#define I2C_ADDRESS_2     0x42    /*!< address for PCA9685 */
#define ACK_CHECK_EN    0x1     /*!< I2C master will check ack from slave */
#define ACK_CHECK_DIS   0x0     /*!< I2C master will not check ack from slave */
#define ACK_VAL         0x0     /*!< I2C ack value */
#define NACK_VAL        0x1     /*!< I2C nack value */

#undef ESP_ERROR_CHECK
#define ESP_ERROR_CHECK(x)   do { esp_err_t rc = (x); if (rc != ESP_OK) { ESP_LOGE("err", "esp_err_t = %d", rc); assert(0 && #x);} } while(0);


#define ACK_CHECK_EN    0x1     /*!< I2C master will check ack from slave */
#define ACK_CHECK_DIS   0x0     /*!< I2C master will not check ack from slave */
#define ACK_VAL         0x0     /*!< I2C ack value */
#define NACK_VAL        0x1     /*!< I2C nack value */

#define MODE1           0x00    /*!< Mode register 1 */
#define MODE2           0x01    /*!< Mode register 2 */
#define SUBADR1         0x02    /*!< I2C-bus subaddress 1 */
#define SUBADR2         0x03    /*!< I2C-bus subaddress 2 */
#define SUBADR3         0x04    /*!< I2C-bus subaddress 3 */
#define ALLCALLADR      0x05    /*!< LED All Call I2C-bus address */
#define LED0            0x6     /*!< LED0 start register */
#define LED0_ON_L       0x6     /*!< LED0 output and brightness control byte 0 */
#define LED0_ON_H       0x7     /*!< LED0 output and brightness control byte 1 */
#define LED0_OFF_L      0x8     /*!< LED0 output and brightness control byte 2 */
#define LED0_OFF_H      0x9     /*!< LED0 output and brightness control byte 3 */
#define LED_MULTIPLYER  4       /*!< For the other 15 channels */
#define ALLLED_ON_L     0xFA    /*!< load all the LEDn_ON registers, byte 0 (turn 0-7 channels on) */
#define ALLLED_ON_H     0xFB    /*!< load all the LEDn_ON registers, byte 1 (turn 8-15 channels on) */
#define ALLLED_OFF_L    0xFC    /*!< load all the LEDn_OFF registers, byte 0 (turn 0-7 channels off) */
#define ALLLED_OFF_H    0xFD    /*!< load all the LEDn_OFF registers, byte 1 (turn 8-15 channels off) */
#define PRE_SCALE       0xFE    /*!< prescaler for output frequency */
#define CLOCK_FREQ      25000000.0  /*!< 25MHz default osc clock */

extern void set_pca9685_adress(uint8_t addr);
extern esp_err_t resetPCA9685(void);
extern esp_err_t setFrequencyPCA9685(uint16_t freq);
extern esp_err_t turnAllOff(void);
extern esp_err_t turnOffLED(uint8_t pin);
extern esp_err_t setPWM(uint8_t num, uint16_t on, uint16_t off);
extern esp_err_t getPWMDetail(uint8_t num, uint8_t* dataReadOn0, uint8_t* dataReadOn1, uint8_t* dataReadOff0, uint8_t* dataReadOff1);
// extern esp_err_t getPWM(uint8_t num, uint16_t* dataReadOn, uint16_t* dataReadOff);
// extern esp_err_t getPWM(uint8_t num);
extern esp_err_t fade_pin_up_down(uint8_t pin);
extern esp_err_t getPWM(uint8_t num, uint16_t* dataOn, uint16_t* dataOff);
extern esp_err_t fade_all_up_down(void);

//kammanit function
// extern void set_bright(uint8_t num1,uint8_t num2,uint8_t num3,
//                        uint8_t num4,
//                        uint8_t b1,uint8_t b2,uint8_t b3,uint8_t b4
//                        );
extern void set_bright(uint8_t num1, uint8_t num2, uint8_t num3,
                uint8_t num4,
                uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4,char* info);
extern void SETFILL(uint8_t pin, bool val,char* info);
extern uint16_t map(uint8_t x);
extern uint8_t PCA9685_ADDR;

extern esp_err_t generic_write_i2c_register_two_words(uint8_t regaddr, uint16_t valueOn, uint16_t valueOff);
extern esp_err_t generic_write_i2c_register_word(uint8_t regaddr, uint16_t value);
extern esp_err_t generic_write_i2c_register(uint8_t regaddr, uint8_t value);
extern esp_err_t generic_read_i2c_register_word(uint8_t regaddr, uint16_t* value);
extern esp_err_t generic_read_two_i2c_register(uint8_t regaddr, uint8_t* valueA, uint8_t* valueB);
extern void disp_buf(uint16_t* buf, uint8_t len);
extern void pca9685_init(void);


#endif /* PCA9685_DRIVER_H */
