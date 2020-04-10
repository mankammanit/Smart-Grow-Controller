/***************************************************
   This is a library for the PCA9685 LED PWM Driver
   This chip is connected via I2C, 2 pins are required to interface. The PWM frequency is set for all pins, the PWM for each individually. The set PWM is active as long as the chip is powered.
   Written by Jonas Scharpf <jonas@brainelectronics.de>
   BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "pca9685.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <driver/i2c.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "esp_err.h"
#include <errno.h>
#include "esp_log.h"
#include "esp_system.h"
//kammanit add
#include "st_profile.h"
#include "nvs_storage.h"
#include "main.h"

static char tag[] = "pca9685";
uint8_t PCA9685_ADDR = 0x0;

const uint16_t pwmTable[256] = {0, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 15, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 21, 21, 22, 22, 23, 24, 24, 25, 26, 27, 27, 28, 29, 30, 31, 31, 32, 33, 34, 35, 36, 37, 38, 39, 41, 42, 43, 44, 45, 47, 48, 49, 51, 52, 54, 55, 57, 59, 60, 62, 64, 66, 68, 69, 71, 74, 76, 78, 80, 82, 85, 87, 90, 92, 95, 98, 100, 103, 106, 109, 112, 116, 119, 122, 126, 130, 133, 137, 141, 145, 149, 153, 158, 162, 167, 172, 177, 182, 187, 193, 198, 204, 210, 216, 222, 228, 235, 241, 248, 255, 263, 270, 278, 286, 294, 303, 311, 320, 330, 339, 349, 359, 369, 380, 391, 402, 413, 425, 437, 450, 463, 476, 490, 504, 518, 533, 549, 564, 581, 597, 614, 632, 650, 669, 688, 708, 728, 749, 771, 793, 816, 839, 863, 888, 913, 940, 967, 994, 1023, 1052, 1082, 1114, 1146, 1178, 1212, 1247, 1283, 1320, 1358, 1397, 1437, 1478, 1520, 1564, 1609, 1655, 1703, 1752, 1802, 1854, 1907, 1962, 2018, 2076, 2135, 2197, 2260, 2325, 2391, 2460, 2531, 2603, 2678, 2755, 2834, 2916, 2999, 3085, 3174, 3265, 3359, 3455, 3555, 3657, 3762, 3870, 3981, 4095};

//function map kammanit 0-4095
//return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min
//map invert
uint16_t map(uint8_t x)
{
        //รุ่นเก่า 3000 , 122
        //รุ่นใหม่ 3000 , 200
        // printf("val x in :%d\n", x );
        // x = 100 - x;
        // uint16_t val = (x - 0) * (4095 - 0) / (100 - 0) + 0;
        uint16_t val = (x - 0) * (3000 - 200) / (100 - 0) + 200;
        // printf("val map :%d\n", val );
        // uint16_t val = (x - 0) * (243 - 150) / (100 - 0) + 150;
        return val;
        // printf("pwmTable[%d] , %d\n", val,pwmTable[val]);
        // return pwmTable[val];
}

/**
 * @brief      Sets the adress where the PCA9685 is located
 *
 * @param[in]  addr  The address
 */
void set_pca9685_adress(uint8_t addr)
{
        PCA9685_ADDR = addr;
}

/**
 * @brief      Reset the PCA9685
 *
 * @return     result of command
 */
esp_err_t resetPCA9685(void)
{
        esp_err_t ret;

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (PCA9685_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, MODE1, ACK_CHECK_EN); // 0x0 = "Mode register 1"
        i2c_master_write_byte(cmd, 0x80, ACK_CHECK_EN);  // 0x80 = "Reset"
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        vTaskDelay(50 / portTICK_RATE_MS);

        return ret;
}

/**
 * @brief      Write two 16 bit values to the same register on an i2c device
 *
 * @param[in]  regaddr   The register address
 * @param[in]  valueOn   The value on
 * @param[in]  valueOff  The value off
 *
 * @return     result of command
 */
esp_err_t generic_write_i2c_register_two_words(uint8_t regaddr, uint16_t valueOn, uint16_t valueOff)
{
        esp_err_t ret;

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (PCA9685_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, regaddr, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, valueOn & 0xff, ACK_VAL);
        i2c_master_write_byte(cmd, valueOn >> 8, NACK_VAL);
        i2c_master_write_byte(cmd, valueOff & 0xff, ACK_VAL);
        i2c_master_write_byte(cmd, valueOff >> 8, NACK_VAL);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        return ret;
}

/**
 * @brief      Write a 16 bit value to a register on an i2c device
 *
 * @param[in]  regaddr  The register address
 * @param[in]  value    The value
 *
 * @return     result of command
 */
esp_err_t generic_write_i2c_register_word(uint8_t regaddr, uint16_t value)
{
        esp_err_t ret;

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (PCA9685_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, regaddr, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, value & 0xff, ACK_VAL);
        i2c_master_write_byte(cmd, value >> 8, NACK_VAL);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        return ret;
}

/**
 * @brief      Write a 8 bit value to a register on an i2c device
 *
 * @param[in]  regaddr  The register address
 * @param[in]  value    The value
 *
 * @return     result of command
 */
esp_err_t generic_write_i2c_register(uint8_t regaddr, uint8_t value)
{
        esp_err_t ret;

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (PCA9685_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, regaddr, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, value, NACK_VAL);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        return ret;
}

/**
 * @brief      Read two 8 bit values from the same register on an i2c device
 *
 * @param[in]  regaddr  The register address
 * @param      valueA   The first value
 * @param      valueB   The second value
 *
 * @return     result of command
 */
esp_err_t generic_read_two_i2c_register(uint8_t regaddr, uint8_t *valueA, uint8_t *valueB)
{
        esp_err_t ret;

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (PCA9685_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, regaddr, ACK_CHECK_EN);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
        if (ret != ESP_OK)
        {
                return ret;
        }
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, PCA9685_ADDR << 1 | I2C_MASTER_READ, ACK_CHECK_EN);
        i2c_master_read_byte(cmd, valueA, ACK_VAL);
        i2c_master_read_byte(cmd, valueB, NACK_VAL);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);

        return ret;
}

/**
 * @brief      Read a 16 bit value from a register on an i2c decivde
 *
 * @param[in]  regaddr  The register address
 * @param      value    The value
 *
 * @return     result of command
 */
esp_err_t generic_read_i2c_register_word(uint8_t regaddr, uint16_t *value)
{
        esp_err_t ret;

        uint8_t valueA;
        uint8_t valueB;

        ret = generic_read_two_i2c_register(regaddr, &valueA, &valueB);
        if (ret != ESP_OK)
        {
                return ret;
        }

        *value = (valueB << 8) | valueA;

        return ret;
}

/**
 * @brief      Sets the frequency of PCA9685
 *
 * @param[in]  freq  The frequency
 *
 * @return     result of command
 */
esp_err_t setFrequencyPCA9685(uint16_t freq)
{
        esp_err_t ret;

        // Send to sleep
        ret = generic_write_i2c_register(MODE1, 0x10);
        if (ret != ESP_OK)
        {
                return ret;
        }

        // Set prescaler
        // calculation on page 25 of datasheet
        uint8_t prescale_val = round((CLOCK_FREQ / 4096 / (0.9 * freq)) - 1 + 0.5);
        ret = generic_write_i2c_register(PRE_SCALE, prescale_val);
        if (ret != ESP_OK)
        {
                return ret;
        }

        // reset again
        resetPCA9685();

        // Send to sleep again
        ret = generic_write_i2c_register(MODE1, 0x10);
        if (ret != ESP_OK)
        {
                return ret;
        }

        // wait
        vTaskDelay(5 / portTICK_PERIOD_MS);

        // Write 0xa0 for auto increment LED0_x after received cmd
        ret = generic_write_i2c_register(MODE1, 0xa0);
        if (ret != ESP_OK)
        {
                return ret;
        }

        // // Read back set data
        // cmd = i2c_cmd_link_create();
        // i2c_master_start(cmd);
        // i2c_master_write_byte(cmd, (PCA9685_ADDR << 1) | I2C_MASTER_READ, 1);
        // i2c_master_read_byte(cmd, data,   0);
        // i2c_master_read_byte(cmd, data+1, 0);
        // i2c_master_read_byte(cmd, data+2, 1);
        // i2c_master_stop(cmd);
        // ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100/portTICK_PERIOD_MS);
        // i2c_cmd_link_delete(cmd);
        // if (ret != ESP_OK) {
        //     return ret;
        // }

        return ret;
}

/**
 * @brief      Sets the pwm of the pin
 *
 * @param[in]  num   The pin number
 * @param[in]  on    On time
 * @param[in]  off   Off time
 *
 * @return     result of command
 */
esp_err_t setPWM(uint8_t num, uint16_t on, uint16_t off)
{
        esp_err_t ret;
        uint8_t pinAddress = LED0_ON_L + LED_MULTIPLYER * num;
        ret = generic_write_i2c_register_two_words(pinAddress & 0xff, on, off);

        return ret;
}

/**
 * @brief      Gets the pwm of a pin detail
 *
 * Have read each LED0_ON_L and LED0_OFF_L seperate
 *
 * @param[in]  num           The number
 * @param      dataReadOn0   The data read on 0
 * @param      dataReadOn1   The data read on 1
 * @param      dataReadOff0  The data read off 0
 * @param      dataReadOff1  The data read off 1
 *
 * @return     result of command
 */
esp_err_t getPWMDetail(uint8_t num, uint8_t *dataReadOn0, uint8_t *dataReadOn1, uint8_t *dataReadOff0, uint8_t *dataReadOff1)
{
        esp_err_t ret;

        uint8_t pinAddress = LED0_ON_L + LED_MULTIPLYER * num;

        ret = generic_read_two_i2c_register(pinAddress, dataReadOn0, dataReadOn1);
        if (ret != ESP_OK)
        {
                return ret;
        }

        pinAddress = LED0_OFF_L + LED_MULTIPLYER * num;
        ret = generic_read_two_i2c_register(pinAddress, dataReadOff0, dataReadOff1);

        return ret;
}

/**
 * @brief      Gets the pwm of a pin
 *
 * @param[in]  num      The number
 * @param      dataOn   The data on
 * @param      dataOff  The data off
 *
 * @return     result of command
 */
esp_err_t getPWM(uint8_t num, uint16_t *dataOn, uint16_t *dataOff)
{
        esp_err_t ret;

        uint8_t readPWMValueOn0;
        uint8_t readPWMValueOn1;
        uint8_t readPWMValueOff0;
        uint8_t readPWMValueOff1;

        ret = getPWMDetail(num, &readPWMValueOn0, &readPWMValueOn1, &readPWMValueOff0, &readPWMValueOff1);

        *dataOn = (readPWMValueOn1 << 8) | readPWMValueOn0;
        *dataOff = (readPWMValueOff1 << 8) | readPWMValueOff0;

        return ret;
}

/**
 * @brief      Turn all LEDs off
 *
 * @return     result of command
 */
esp_err_t turnAllOff(void)
{
        esp_err_t ret;
        uint16_t valueOn = 0;
        uint16_t valueOff = 4096;
        ret = generic_write_i2c_register_two_words(ALLLED_ON_L, valueOn, valueOff);

        return ret;
}

/**
 * @brief      fade pin up to maximum and back down
 *
 * @param[in]  pin   The pin
 *
 * @return     result of command
 */
esp_err_t fade_pin_up_down(uint8_t pin)
{
        esp_err_t ret;

        for (uint8_t i = 0; i < 255; i++)
        {
                // fade up
                ret = setPWM(pin, 0, pwmTable[i]);
                if (ret != ESP_OK)
                {
                        return ret;
                }

                vTaskDelay(50 / portTICK_PERIOD_MS);
        }

        for (uint8_t i = 0; i < 255; i++)
        {
                // fade down
                ret = setPWM(pin, pwmTable[i], 0);
                if (ret != ESP_OK)
                {
                        return ret;
                }

                vTaskDelay(50 / portTICK_PERIOD_MS);
        }

        // ensure it's really off
        ret = setPWM(pin, 0, 4096);
        if (ret != ESP_OK)
        {
                return ret;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);

        return ret;
}

/**
 * @brief      fade each pin up to maximum and back down
 *
 * @return     result of command
 */
esp_err_t fade_all_up_down(void)
{
        esp_err_t ret;

        for (uint8_t pin = 0; pin < 16; pin++)
        {
                for (uint8_t i = 0; i < 255; i++)
                {
                        // fade up
                        // printf("fade all up %d \n",i);
                        ret = setPWM(pin, 0, pwmTable[i]);
                        if (ret != ESP_OK)
                        {
                                return ret;
                        }

                        vTaskDelay(50 / portTICK_PERIOD_MS);
                }

                for (uint8_t i = 0; i < 255; i++)
                {
                        // fade down
                        // printf("fade all down %d \n",i);
                        ret = setPWM(pin, pwmTable[i], 0);
                        if (ret != ESP_OK)
                        {
                                return ret;
                        }

                        vTaskDelay(50 / portTICK_PERIOD_MS);
                }

                // ensure it's really off
                ret = setPWM(pin, 0, 4096);
                if (ret != ESP_OK)
                {
                        return ret;
                }
                vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        return ret;
}

/**
 * @brief      test function to show buffer
 *
 * @param      buf   The buffer
 * @param[in]  len   The length
 */
void disp_buf(uint16_t *buf, uint8_t len)
{
        uint8_t i;
        for (i = 0; i < len; i++)
        {
                printf("%02x ", buf[i]);
                if ((i + 1) % 16 == 0)
                {
                        printf("\n");
                }
        }
        printf("\n");
}
static void initI2C()
{
        {
                i2c_config_t conf;
                conf.mode = I2C_MODE_MASTER;
                conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
                conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
                conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
                conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
                conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;

                int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
                ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));
                ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode,
                                                   I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                                                   I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0));
        }
}
/* init i2c pca9685 */
void pca9685_init(void)
{
        ESP_LOGI(tag, ">> pca9685");
        initI2C();
}

void set_bright(uint8_t num1, uint8_t num2, uint8_t num3,
                uint8_t num4,
                uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4,char* info)

{
        printf("-------------------------------------------------------WORK'S %s\n",info);
        
        // enable_pca9685_1();
        if (b1 == 255)
                b1 = 0;
        if (b2 == 255)
                b2 = 0;
        if (b3 == 255)
                b3 = 0;
        if (b4 == 255)
                b4 = 0;

        enable_pca9685_1();
        if(b1==0) setPWM(num1, 4096, 0);
        else setPWM(num1, map(b1), 0);

        if(b2==0) setPWM(num2, 4096, 0);
        else setPWM(num2, map(b2), 0);

        if(b3==0) setPWM(num3, 4096, 0);
        else setPWM(num3, map(b3), 0);

        if(b4==0) setPWM(num4, 4096, 0);
        else setPWM(num4, map(b4), 0);

        // esp_mqtt_publish_string("DEBUG_BRIGHT",info);

}


//led
void enable_pca9685_1()
{
        set_pca9685_adress(I2C_ADDRESS_1);
}
//environment
void enable_pca9685_2()
{
        set_pca9685_adress(I2C_ADDRESS_2);
}
