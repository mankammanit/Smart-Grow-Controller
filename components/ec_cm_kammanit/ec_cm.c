
#include "ec_cm.h"

float ec_temp = 25.00;
float ECcurrent;

float ec_read(int averageVoltage)
{
        //1st read nvs ec
        if(read_ec_kvalue(&ec_val));
        else{
                ec_val._voltoffset=0.00;
                save_ec_kvalue(ec_val);
        }

        if(averageVoltage>=216)
        {
                averageVoltage=averageVoltage-ec_val._voltoffset;
        }
        else if(averageVoltage<216)
        {
                averageVoltage=averageVoltage+ec_val._voltoffset;
        }

        // printf("EC AFTER OFFSET : %d\n",averageVoltage);
        float TempCoefficient = 1.0 + 0.0185 * (ec_temp - 25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.0185*(fTP-25.0));
        float CoefficientVolatge = (float)averageVoltage / TempCoefficient;

        if (CoefficientVolatge < 20)
                // printf("No Solution!\n"); //25^C 1413us/cm<-->about 216mv  if the voltage(compensate)<150,that is <1ms/cm,out of the range
                return 0.00;
        else if (CoefficientVolatge > 3300)
                // printf("Out of the range!\n");  //>20ms/cm,out of the range
                return 0.00;
        else
        {

                if (CoefficientVolatge <= 448) {
                        // printf("EC<=3ms\n"); //>20ms/cm,out of the range
                        ECcurrent = 6.84 * CoefficientVolatge - 64.32; //1ms/cm<EC<=3ms/cm
                }else if (CoefficientVolatge <= 1457) {
                        // printf("EC<=10ms\n");  //>20ms/cm,out of the range
                        ECcurrent = 6.98 * CoefficientVolatge - 127; //3ms/cm<EC<=10ms/cm
                } else{
                        // printf("EC<=20ms\n"); //>20ms/cm,out of the range
                        ECcurrent = 5.3 * CoefficientVolatge + 2278; //10ms/cm<EC<20ms/cm
                }

                ECcurrent /= 1000;     //convert us/cm to ms/cm

                // printf("EC VAL : %.2f ms/cm",ECcurrent);
                return ECcurrent;

        }
}

uint8_t calibrat_ec(int volt)
{
        // printf("EC BEFORE OFFSET : %d\n",volt);
        if(volt>=216)
        {
                ec_val._voltoffset=volt - 216;
        }
        else if(volt<216)
        {
                ec_val._voltoffset=216-volt;
        }
        save_ec_kvalue(ec_val);
        return ec_val._voltoffset;
}
