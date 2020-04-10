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


        if(ph_val._checkwrite==0)
        {
                ph_val._temp_c = 25.0;
                ph_val._phValue = 7.0;

                ph_val._acidVoltage = CENTER_POINT4; //buffer solution 4.0 at 25C
                ph_val._neutralVoltage = CENTER_POINT7; //buffer solution 7.0 at 25C
                ph_val._voltage = CENTER_POINT7;

                ph_val.PH_7_VOL = CENTER_POINT7;
                ph_val.PH_4_VOL = CENTER_POINT4;

                ph_val.PH_8_VOL = PH_8_VOLTAGE;
                ph_val.PH_6_VOL = PH_6_VOLTAGE;
                ph_val.PH_5_VOL = PH_5_VOLTAGE;
                ph_val.PH_3_VOL = PH_3_VOLTAGE;

                ph_val._checkwrite=1;
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
        switch (mode)
        {
        case 0:
                if (enterCalibrationFlag)
                {
                        // printf(">>>pH Command Error<<<\n");
                }
                break;

        case 1:
                enterCalibrationFlag = 1;
                phCalibrationFinish = 0;

                // printf(">>>Enter PH Calibration Mode<<<\n");
                // printf(">>>Please put the probe into the 4.0 or 7.0 standard buffer solution<<<\n");
                sprintf(str_name,INFO_BUFF_PH,"PH Calibration");
                send_tft(str_name);
                sprintf(str_name,PER_BUFF,35);
                send_tft(str_name);

                break;


        case 2:

                if(enterCalibrationFlag) {

                        if((ph_val._voltage>ph_val.PH_8_VOL)&&(ph_val._voltage<ph_val.PH_6_VOL))
                        // buffer solution:7.0
                        {
                                // printf(">>>pH Buffer Solution:7.0\n");
                                ph_val._neutralVoltage = ph_val._voltage;
                                // printf(",Send EXITPH to Save and Exit<<<\n");
                                phCalibrationFinish = 1;

                                sprintf(str_name,INFO_BUFF_PH,"Buffer 7.0");
                                send_tft(str_name);
                                sprintf(str_name,PER_BUFF,70);
                                send_tft(str_name);

                        }
                        else if((ph_val._voltage>ph_val.PH_5_VOL)&&(ph_val._voltage<ph_val.PH_3_VOL))
                        { //buffer solution:4.0
                                // printf(">>>pH Buffer Solution:4.0\n");
                                ph_val._acidVoltage = ph_val._voltage;
                                // printf(",Send EXITPH to Save and Exit<<<\n");
                                phCalibrationFinish = 1;

                                sprintf(str_name,INFO_BUFF_PH,"Buffer 4.0");
                                send_tft(str_name);
                                sprintf(str_name,PER_BUFF,70);
                                send_tft(str_name);
                        }
                        else
                        {
                                // printf(">>>pH Buffer Solution Error Try Again<<<\n");
                                // not buffer solution or faulty operation
                                phCalibrationFinish = 0;
                                sprintf(str_name,INFO_BUFF_PH,"Buff Try Again");
                                send_tft(str_name);
                                sprintf(str_name,PER_BUFF,0);
                                send_tft(str_name);
                        }
                }
                break;

        case 3:
                if (enterCalibrationFlag)
                {


                        if(phCalibrationFinish)
                        {
                                if((ph_val._voltage>ph_val.PH_8_VOL)&&(ph_val._voltage<ph_val.PH_6_VOL))
                                {
                                        save_ph_kvalue(ph_val);
                                        // printf("debug ph nvs_save1\n");
                                }
                                else if((ph_val._voltage>ph_val.PH_5_VOL)&&(ph_val._voltage<ph_val.PH_3_VOL))
                                {
                                        save_ph_kvalue(ph_val);
                                        // printf("debug ph nvs_save2\n");
                                }
                                // printf(">>>pH Calibration Successful\n");
                                sprintf(str_name,INFO_BUFF_PH,"Successful!");
                                send_tft(str_name);
                                sprintf(str_name,PER_BUFF,100);
                                send_tft(str_name);
                        }
                        else
                        {
                                printf(">>>pH Calibration Failed\n");
                                sprintf(str_name,INFO_BUFF_PH,"Buff Try Again");
                                send_tft(str_name);
                                sprintf(str_name,PER_BUFF,0);
                                send_tft(str_name);
                        }
                        // printf(",Exit PH Calibration Mode<<<\n");
                        phCalibrationFinish = 0;
                        enterCalibrationFlag = 0;
                }
                break;
        }
}

float readPH(float voltage)
{
        float slope = (7.0-4.0)/((ph_val._neutralVoltage-ph_val.PH_7_VOL)/3.0 - (ph_val._acidVoltage-ph_val.PH_7_VOL)/3.0); // two point: (_neutralVoltage,7.0),(_acidVoltage,4.0)
        float intercept =  7.0 - slope*(ph_val._neutralVoltage-ph_val.PH_7_VOL)/3.0;

        ph_val._phValue = slope*(voltage-ph_val.PH_7_VOL)/3.0+intercept; //y = k*x + b
        return ph_val._phValue;
}

void readAnalogpH()
{

        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
                if (unit == ADC_UNIT_1) {
                        adc_reading_ph += adc1_get_raw((adc1_channel_t)PH_CH);
                } else {
                        int raw;
                        adc2_get_raw((adc2_channel_t)PH_CH, ADC_WIDTH, &raw);
                        adc_reading_ph += raw;
                }
        }
        adc_reading_ph /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading_ph, adc_chars);
        printf("Raw: %.2f\tVoltage: %dmV\n", adc_reading_ph, voltage);

        printf("VOLTAGE_PH 4 --> %.2f,%.2f,%.2f\n",ph_val.PH_5_VOL,
               ph_val.PH_4_VOL,ph_val.PH_3_VOL);
        printf("VOLTAGE_PH 7 --> %.2f,%.2f,%.2f\n",ph_val.PH_8_VOL,
               ph_val.PH_7_VOL,ph_val.PH_6_VOL);

        printf("_neutralVoltage %.2f --> _acidVoltage %.2f\n",ph_val._neutralVoltage,ph_val._acidVoltage);

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
        // Configure ADC
        adc1_config_width(ADC_WIDTH);
        adc1_config_channel_atten(PH_CH, atten);

        //Characterize ADC
        adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
        esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH, VREF, adc_chars);
        print_char_val_type(val_type);

        adc_chars->coeff_a = (VREF * adc1_vref_atten_scale[atten]) / (ADC_12_BIT_RES);
        adc_chars->coeff_b = adc1_vref_atten_offset[atten];


}
