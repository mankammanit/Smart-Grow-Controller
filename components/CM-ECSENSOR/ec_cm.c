
#include "ec_cm.h"

float ec_temp = 25.00;
float ECcurrent;

#define RES2 820.0
#define ECREF 200.0

void ec_add_val()
{

        read_ec_kvalue(&ec_val);
        ec_val._ecvalue = ec_val._ecvalue;
        ec_val._kvalue = ec_val._kvalue;
        ec_val._kvalueLow = ec_val._kvalueLow;
        ec_val._kvalueHigh = ec_val._kvalueHigh;
        ec_val._voltage = ec_val._voltage;

        if(ec_val._checkwrite==0)
        {
                ec_val._ecvalue = 0.0;
                ec_val._kvalue = 1.0;
                ec_val._kvalueLow = 1.0;
                ec_val._kvalueHigh = 1.0;
                ec_val._voltage = 0.0;
                ec_val._checkwrite=1;
                printf("ec_addval\n");
                save_ec_kvalue(ec_val);
        }
}

void ec_calibration(int voltage,uint8_t mode)
{
        ec_val._voltage = voltage;
        ecCalibration(mode);
}

float ec_read(int voltage)
{
        float value = 0, valueTemp = 0;
        ec_val._rawEC = 1000 * voltage / RES2 / ECREF;
        // printf(">>>_rawEC: %.2f<<<\n",ec_val._rawEC);
        valueTemp = ec_val._rawEC * ec_val._kvalue;
        //automatic shift process
        //First Range:(0,2); Second Range:(2,20)
        if (valueTemp > 2.5)
        {
                ec_val._kvalue = ec_val._kvalueHigh;
        }
        else if (valueTemp < 2.0)
        {
                ec_val._kvalue = ec_val._kvalueLow;
        }

        value = ec_val._rawEC * ec_val._kvalue;              //calculate the EC value after automatic shift
        value = value / (1.0 + 0.0185 * (ec_temp - 25.0));  //temperature compensation
        ec_val._ecvalue = value;                            //store the EC value for Serial CMD calibration
        // printf(">>>_ecvalue: %.2f<<<\n",ec_val._ecvalue);
        return ec_val._ecvalue;
}

void ecCalibration(uint8_t mode)
{

        static bool ecCalibrationFinish = 0;
        static bool enterCalibrationFlag = 0;
        static float compECsolution;
        float KValueTemp;
        switch (mode)
        {
        case 0:
                if (enterCalibrationFlag)
                {
                        // printf(">>>Command Error<<<\n");
                }
                break;

        case 1:
                enterCalibrationFlag = 1;
                ecCalibrationFinish = 0;

                printf(">>>Enter EC Calibration Mode<<<\n");
                // printf(">>>Please put the probe into the 1413us/cm or 2.76ms/cm or 12.88ms/cm buffer solution<<<\n");
                // printf(">>>Only need two point for calibration one low (1413us/com) and one high(2.76ms/cm or 12.88ms/cm)<<<\n");
                sprintf(str_name, INFO_BUFF_EC, "EC Calibration");
                send_tft(str_name);
                sprintf(str_name,PER_BUFF,35);
                send_tft(str_name);
                break;

        case 2:
                if (enterCalibrationFlag)
                {
                        if ((ec_val._rawEC > RAWEC_1413_LOW) && (ec_val._rawEC < RAWEC_1413_HIGH))
                        {
                                printf(">>>Buffer 1.413ms/cm<<<\n");        //recognize 1.413us/cm buffer solution
                                compECsolution = 1.413 * (1.0 + 0.0185 * (ec_temp - 25.0)); //temperature compensation
                                // printf(">>>compECsolution: %.2f<<<\n",compECsolution);
                                sprintf(str_name, INFO_BUFF_EC, "Buff 1.413ms/cm");
                                send_tft(str_name);
                                sprintf(str_name,PER_BUFF,70);
                                send_tft(str_name);
                        }
                        else if ((ec_val._rawEC > RAWEC_276_LOW) && (ec_val._rawEC < RAWEC_276_HIGH))
                        {
                                printf(">>>Buffer 2.76ms/cm<<<\n");                          //recognize 2.76ms/cm buffer solution
                                compECsolution = 2.76 * (1.0 + 0.0185 * (ec_temp - 25.0)); //temperature compensation
                                // printf(">>>compECsolution: %.2f<<<\n",compECsolution);
                                sprintf(str_name, INFO_BUFF_EC, "Buff 2.76ms/cm");
                                send_tft(str_name);
                                sprintf(str_name,PER_BUFF,70);
                                send_tft(str_name);
                        }
                        else if ((ec_val._rawEC > RAWEC_1288_LOW) && (ec_val._rawEC < RAWEC_1288_HIGH))
                        {
                                printf(">>>Buffer 12.88ms/cm<<<\n");                      //recognize 12.88ms/cm buffer solution
                                compECsolution = 12.88 * (1.0 + 0.0185 * (ec_temp - 25.0)); //temperature compensation
                                // printf(">>>compECsolution: %.2f<<<\n",compECsolution);
                                sprintf(str_name, INFO_BUFF_EC, "Buff 12.88ms/cm");
                                send_tft(str_name);
                                sprintf(str_name,PER_BUFF,70);
                                send_tft(str_name);
                        }
                        else
                        {
                                printf(">>>Buffer Solution Error Try Again<<<\n");
                                ecCalibrationFinish = 0;
                                sprintf(str_name, INFO_BUFF_EC, "Buff Try Again");
                                send_tft(str_name);
                                sprintf(str_name,PER_BUFF,0);
                                send_tft(str_name);
                        }

                        KValueTemp = RES2 * ECREF * compECsolution / 1000.0 / ec_val._voltage; //calibrate the k value
                        printf(">>>KValueTemp: %.2f<<<\n",KValueTemp);

                        if ((KValueTemp > 0.5) && (KValueTemp < 2.1))
                        {
                                printf(">>>Successful,K: %.2f<<<\n",KValueTemp);
                                // printf(">>>Send EXITEC to Save and Exit<<<\n");

                                if ((ec_val._rawEC > RAWEC_1413_LOW) && (ec_val._rawEC < RAWEC_1413_HIGH))
                                {
                                        ec_val._kvalueLow = KValueTemp;
                                        printf(">>>_kvalueLow: %.2f<<<\n",ec_val._kvalueLow);
                                }
                                else if ((ec_val._rawEC > RAWEC_276_LOW) && (ec_val._rawEC < RAWEC_276_HIGH))
                                {
                                        ec_val._kvalueHigh = KValueTemp;
                                        printf(">>>_kvalueHigh: %.2f<<<\n",ec_val._kvalueHigh);
                                }
                                else if ((ec_val._rawEC > RAWEC_1288_LOW) && (ec_val._rawEC < RAWEC_1288_HIGH))
                                {
                                        ec_val._kvalueHigh = KValueTemp;
                                        printf(">>>_kvalueHigh: %.2f<<<\n",ec_val._kvalueHigh);
                                }
                                ecCalibrationFinish = 1;
                        }
                        else
                        {

                                printf(">>>KValueTemp out of range 0.5-2.1<<<\n");
                                // printf(">>>KValueTemp: %.2f<<<\n",KValueTemp);
                                // printf(">>>Failed,Try Again<<<\n");
                                ecCalibrationFinish = 0;
                                sprintf(str_name, INFO_BUFF_EC, "Buff Try Again");
                                send_tft(str_name);
                                sprintf(str_name,PER_BUFF,0);
                                send_tft(str_name);
                        }
                }
                break;

        case 3:
                if (enterCalibrationFlag)
                {

                        if (ecCalibrationFinish)
                        {
                                if ((ec_val._rawEC > RAWEC_1413_LOW) && (ec_val._rawEC < RAWEC_1413_HIGH))
                                {
                                        save_ec_kvalue(ec_val);
                                }
                                else if ((ec_val._rawEC > RAWEC_276_LOW) && (ec_val._rawEC < RAWEC_276_HIGH))
                                {
                                        save_ec_kvalue(ec_val);
                                }
                                else if ((ec_val._rawEC > RAWEC_1288_LOW) && (ec_val._rawEC < RAWEC_1288_HIGH))
                                {
                                        save_ec_kvalue(ec_val);
                                }
                                printf(">>>Calibration Successful<<<\n");
                                sprintf(str_name, INFO_BUFF_EC, "Successful!");
                                send_tft(str_name);
                                sprintf(str_name,PER_BUFF,100);
                                send_tft(str_name);
                        }
                        else
                        {
                                printf(">>>Calibration Failed<<<\n");
                                sprintf(str_name, INFO_BUFF_EC, "Buff Try Again");
                                send_tft(str_name);
                                sprintf(str_name,PER_BUFF,0);
                                send_tft(str_name);
                        }
                        // printf(">>>Exit EC Calibration Mode<<<\n");
                        ecCalibrationFinish = 0;
                        enterCalibrationFlag = 0;
                }
                break;
        }
}
