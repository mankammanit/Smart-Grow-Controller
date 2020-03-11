/* ADC2 Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/dac.h"
#include "esp_system.h"
#include "esp_adc_cal.h"
#include "string.h"
#include "st_profile.h"
#include "nvs_storage.h"
#include "ph_cm.h"
#include "main.h"
#include "uart_cm.h"
#include "feeding_cm.h"

void check_efuse()
{
        //Check TP is burned into eFuse
        if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
        {
                printf("eFuse Two Point: Supported\n");
        }
        else
        {
                printf("eFuse Two Point: NOT supported\n");
        }

        //Check Vref is burned into eFuse
        if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
        {
                printf("eFuse Vref: Supported\n");
        }
        else
        {
                printf("eFuse Vref: NOT supported\n");
        }
}

void print_char_val_type(esp_adc_cal_value_t val_type)
{
        if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
                printf("Characterized using Two Point Value\n");
        } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
                printf("Characterized using eFuse Vref\n");
        } else {
                printf("Characterized using Default Vref\n");
        }
}


void ph_add_val()
{

        read_ph_kvalue(&ph_val);
        ph_val._temp_c = ph_val._temp_c;
        ph_val._phValue = ph_val._phValue;
        ph_val._acidVoltage = ph_val._acidVoltage; //buffer solution 4.0 at 25C
        ph_val._neutralVoltage = ph_val._neutralVoltage; //buffer solution 7.0 at 25C
        ph_val._voltage = ph_val._voltage;

        if(ph_val._checkwrite==0)
        {
                ph_val._temp_c = 25.0;
                ph_val._phValue = 7.0;
                ph_val._acidVoltage = CENTER_POINT4; //buffer solution 4.0 at 25C
                ph_val._neutralVoltage = CENTER_POINT7; //buffer solution 7.0 at 25C
                ph_val._voltage = CENTER_POINT7;
                printf("ph_addval\n");
                save_ph_kvalue(ph_val);
        }
}

void calibration(float voltage,uint8_t mode)
{
        ph_val._voltage = voltage;
        ph_val._temp_c = 25.0;
        phCalibration(mode);
        // printf("calibration : %.1f , %.1f \n",ph_val._voltage,ph_val._temp_c);
}


void phCalibration(uint8_t mode)
{
        static bool phCalibrationFinish = 0;
        static bool enterCalibrationFlag = 0;
        char ph_tft[6];
        switch (mode)
        {
        case 0:
                if (enterCalibrationFlag)
                {
                        printf(">>>pH Command Error<<<\n");
                }
                break;

        case 1:
                enterCalibrationFlag = 1;
                phCalibrationFinish = 0;

                sprintf(ph_tft, "%.1f", adc_reading_buff);
                sprintf(str_name,INFO_BUFF_PH,ph_tft);
                send_tft(str_name);

                printf(">>>Enter PH Calibration Mode<<<\n");
                printf(">>>Please put the probe into the 4.0 or 7.0 standard buffer solution<<<\n");

                sprintf(str_name,PER_BUFF,30);
                send_tft(str_name);

                break;


        case 2:

                if(enterCalibrationFlag) {

                        if((ph_val._voltage>PH_8_VOLTAGE)&&(ph_val._voltage<PH_6_VOLTAGE))
                        // buffer solution:7.0
                        {
                                sprintf(str_name,INFO_BUFF_PH,"Buffer 7.0");
                                send_tft(str_name);
                                printf(">>>pH Buffer Solution:7.0\n");
                                ph_val._neutralVoltage = ph_val._voltage;
                                printf(",Send EXITPH to Save and Exit<<<\n");
                                phCalibrationFinish = 1;
                                sprintf(str_name,PER_BUFF,60);
                                send_tft(str_name);
                        }
                        else if((ph_val._voltage>PH_5_VOLTAGE)&&(ph_val._voltage<PH_3_VOLTAGE))
                        { //buffer solution:4.0
                                sprintf(str_name,INFO_BUFF_PH,"Buffer 4.0");
                                send_tft(str_name);
                                printf(">>>pH Buffer Solution:4.0\n");
                                ph_val._acidVoltage = ph_val._voltage;
                                printf(",Send EXITPH to Save and Exit<<<\n");
                                phCalibrationFinish = 1;
                                sprintf(str_name,PER_BUFF,60);
                                send_tft(str_name);
                        }
                        else
                        {
                                sprintf(str_name,INFO_BUFF_PH,"Out Range!");
                                send_tft(str_name);
                                printf(">>>pH Buffer Solution Error Try Again<<<\n");
                                // not buffer solution or faulty operation
                                phCalibrationFinish = 0;
                                sprintf(str_name,PER_BUFF,10);
                                send_tft(str_name);
                        }
                }
                break;

        case 3:
                if (enterCalibrationFlag)
                {


                        if(phCalibrationFinish)
                        {
                                if((ph_val._voltage>PH_8_VOLTAGE)&&(ph_val._voltage<PH_6_VOLTAGE))
                                {
                                        save_ph_kvalue(ph_val);
                                        printf("debug ph nvs_save1\n");
                                }
                                else if((ph_val._voltage>PH_5_VOLTAGE)&&(ph_val._voltage<PH_3_VOLTAGE))
                                {
                                        save_ph_kvalue(ph_val);
                                        printf("debug ph nvs_save2\n");
                                }
                                sprintf(str_name,INFO_BUFF_PH,"Successful");
                                send_tft(str_name);
                                printf(">>>pH Calibration Successful\n");
                                sprintf(str_name,PER_BUFF,100);
                                send_tft(str_name);
                        }
                        else
                        {
                                sprintf(str_name,INFO_BUFF_PH,"Buff Fail");
                                send_tft(str_name);
                                printf(">>>pH Calibration Failed\n");
                                sprintf(str_name,PER_BUFF,10);
                                send_tft(str_name);
                        }
                        printf(",Exit PH Calibration Mode<<<\n");
                        phCalibrationFinish = 0;
                        enterCalibrationFlag = 0;
                }
                break;
        }
}

float readPH(float voltage)
{
        float slope = (7.0-4.0)/((ph_val._neutralVoltage-CENTER_POINT7)/3.0 - (ph_val._acidVoltage-CENTER_POINT7)/3.0); // two point: (_neutralVoltage,7.0),(_acidVoltage,4.0)
        float intercept =  7.0 - slope*(ph_val._neutralVoltage-CENTER_POINT7)/3.0;

        ph_val._phValue = slope*(voltage-CENTER_POINT7)/3.0+intercept; //y = k*x + b
        return ph_val._phValue;
}

void readAnalogpH()
{
        //ph
        adc_reading_ph = 0;
                #define ArrayLenth 100
        for(int i=0; i<ArrayLenth; i++)
        {
                adc_reading_ph += adc1_get_raw((adc1_channel_t)PH_CH);
                vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        adc_reading_ph /= ArrayLenth;
        // adc_reading_ph = adc_reading_ph / 4096.00 * 3300;
        // printf("PH analog is %4.2f\n", adc_reading_ph);
        if(adc_reading_ph==4095.00)
        {
                ph_value = 0.0;
        }
        else
        {
                adc_reading_buff = adc_reading_ph;
                ///////////// ph read DF Lbr//////////////////
                ph_value = readPH(adc_reading_buff);
        }

}

void init_ph()
{
        ph_add_val();
        check_efuse();
        //Configure ADC
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(PH_CH, atten);
}
