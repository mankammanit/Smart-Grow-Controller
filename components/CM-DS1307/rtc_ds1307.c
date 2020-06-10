#include "rtc_ds1307.h"

static char tag[] = "ds1307";
#define CHECK_ARG(ARG) do { if (!ARG) return ESP_ERR_INVALID_ARG; } while (0)

static uint8_t bcd2dec(uint8_t val)
{
  return (val >> 4) * 10 + (val & 0x0f);
}

static uint8_t dec2bcd(uint8_t val)
{
  return ((val / 10) << 4) + (val % 10);
}


static void initI2C() {
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = SDA_PIN;
  conf.scl_io_num = SCL_PIN;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = 100000;
  i2c_param_config(I2C_DS1307_NUM, &conf);
  i2c_driver_install(I2C_DS1307_NUM, I2C_MODE_MASTER, I2C_EXAMPLE_MASTER_RX_BUF_DISABLE
    , I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);

  }

  esp_err_t writeValue(const struct tm *time) {

    retry_read:
    CHECK_ARG(time);
    esp_err_t errRc;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (DS1307_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x0, 1));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, dec2bcd(time->tm_sec), 1));  // seconds
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, dec2bcd(time->tm_min), 1 )); // minutes
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, dec2bcd(time->tm_hour), 1 )); // hours
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, dec2bcd(time->tm_wday + 1), 1 )); // week day
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, dec2bcd(time->tm_mday), 1)); // date of month
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, dec2bcd(time->tm_mon + 1), 1)); // month
    // ESP_ERROR_CHECK(i2c_master_write_byte(cmd, dec2bcd(time->tm_year-2000), 1)); // year
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, dec2bcd(time->tm_year), 1)); // year
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    errRc = i2c_master_cmd_begin(I2C_DS1307_NUM, cmd, 1000/portTICK_PERIOD_MS);
    switch (errRc)
    {
    case ESP_OK:
            ESP_LOGI(tag, "writeValue OK");
            break;
    case ESP_ERR_INVALID_ARG:
            ESP_LOGE(tag, "Parameter error");
            goto retry_read;
            break;
    case ESP_FAIL:
            ESP_LOGE(tag, "Sending command error, slave doesn’t ACK the transfer.");
            goto retry_read;
            break;
    case ESP_ERR_INVALID_STATE:
            ESP_LOGE(tag, "I2C driver not installed or not in master mode.");
            break;
    case ESP_ERR_TIMEOUT:
            ESP_LOGE(tag, "Operation timeout because the bus is busy.");
            goto retry_read;
            break;
    default:
            ESP_LOGE(tag, "NO CASE");
            break;
    }
    i2c_cmd_link_delete(cmd);
    return ESP_OK;
  }


  /*
  * The value read from the DS1307 is 7 bytes encoded in BCD:
  * 0 - Seconds - 00-59
  * 1 - Minutes - 00-59
  * 2 - Hours   - 00-23
  * 3 - Day     - 01-07
  * 4 - Date    - 01-31
  * 5 - Month   - 01-12
  * 6 - Year    - 00-99
  *
  */
  esp_err_t readValue(struct tm *time) {

    retry_read:
    CHECK_ARG(time);
    uint8_t buf[7];
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (DS1307_ADDRESS << 1), true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x0, 1));
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (DS1307_ADDRESS << 1) | 1, true /* expect ack */));
    ESP_ERROR_CHECK(i2c_master_read(cmd, buf, 7, I2C_MASTER_LAST_NACK));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    esp_err_t errRc = i2c_master_cmd_begin(I2C_DS1307_NUM, cmd, 1000/portTICK_PERIOD_MS);
    switch (errRc)
    {
    case ESP_OK:
            // ESP_LOGI(tag, "readValue OK");
            break;
    case ESP_ERR_INVALID_ARG:
            ESP_LOGE(tag, "Parameter error");
            goto retry_read;
            break;
    case ESP_FAIL:
            ESP_LOGE(tag, "Sending command error, slave doesn’t ACK the transfer.");
            goto retry_read;
            break;
    case ESP_ERR_INVALID_STATE:
            ESP_LOGE(tag, "I2C driver not installed or not in master mode.");
            break;
    case ESP_ERR_TIMEOUT:
            ESP_LOGE(tag, "Operation timeout because the bus is busy.");
            goto retry_read;
            break;
    default:
            ESP_LOGE(tag, "NO CASE");
            break;
    }
    i2c_cmd_link_delete(cmd);



    time->tm_sec = bcd2dec(buf[0] & SECONDS_MASK);
    time->tm_min = bcd2dec(buf[1]);
    if (buf[2] & HOUR12_BIT)
    {
      // RTC in 12-hour mode
      time->tm_hour = bcd2dec(buf[2] & HOUR12_MASK) - 1;
      if (buf[2] & PM_BIT)
      time->tm_hour += 12;
    }
    else
    time->tm_hour = bcd2dec(buf[2] & HOUR24_MASK);
    time->tm_wday = bcd2dec(buf[3]) - 1;
    time->tm_mday = bcd2dec(buf[4]);
    time->tm_mon  = bcd2dec(buf[5]) - 1;
    // time->tm_year = bcd2dec(buf[6])+2000;
    time->tm_year = bcd2dec(buf[6]);
    time->tm_isdst = 0;

    return ESP_OK;
  }


  void tft_set_time(uint16_t set_y , uint8_t set_mo , uint8_t set_d , uint8_t set_h , uint8_t set_m ,uint8_t set_s)
  {
    // setup datetime: 2018-04-11 00:52:10
    struct tm time;
    // printf("debug1--->%d\n",set_y );
    // printf("debug2--->%d\n",set_y-2000 );

    time.tm_year = set_y-1900;
    time.tm_mon  = set_mo-1;
    time.tm_mday = set_d;
    time.tm_hour = set_h;
    time.tm_min  = set_m;
    time.tm_sec  = set_s;

    writeValue(&time);
  }

  void ds1307_init(void) {
    // ESP_LOGI(tag, ">> ds1307");
    initI2C();
  }

  bool betweenTimes(int start_hour, int end_hour,
    int start_minute, int end_minute)
    {
      struct tm time;
      readValue(&time);

      int hours_diff, minutes_diff;
      int hours_diff2, minutes_diff2;
      double diff,diff2;
      time_t end, start;

      struct tm times;
      times.tm_sec = 0;
      times.tm_min = 0;
      times.tm_hour = 0;
      times.tm_mday = 0;
      times.tm_mon = 0;
      times.tm_year = 0;
      times.tm_wday = 0;
      times.tm_yday = 0;

      times.tm_hour=start_hour;
      times.tm_min=start_minute;
      start = mktime(&times);
      times.tm_hour=end_hour;
      times.tm_min=end_minute;
      end = mktime(&times);
      if( end < start )
      {
        end += 24 * 60 * 60;
      }
      diff = difftime(end, start);
      hours_diff = (int) diff / 3600;
      minutes_diff = (int) diff % 3600 / 60;
      /////////////////////////////
      times.tm_hour=time.tm_hour;
      times.tm_min=time.tm_min;
      start = mktime(&times);
      times.tm_hour=end_hour;
      times.tm_min=end_minute;
      end = mktime(&times);
      if( end < start )
      {
        end += 24 * 60 * 60;
      }
      diff2 = difftime(end, start);
      hours_diff2 = (int) diff2 / 3600;
      minutes_diff2 = (int) diff2 % 3600 / 60;


      const int MINUTES_PER_HOUR = 60;
      //minute of day (out of 1440) from passed now object
      // int current_time = time.tm_hour*MINUTES_PER_HOUR + time.tm_min;
      // int on_time = start_hour*MINUTES_PER_HOUR + start_minute;
      // int off_time = end_hour*MINUTES_PER_HOUR + end_minute;
      time_t diff_time = hours_diff*MINUTES_PER_HOUR + minutes_diff;
      time_t diff_time2 = hours_diff2*MINUTES_PER_HOUR + minutes_diff2;
      // printf("TIME CURRENT : %02d:%02d\n",time.tm_hour,time.tm_min);
      // printf("TIME ON : %02d:%02d\n",start_hour,start_minute);
      // printf("TIME OFF : %02d:%02d\n",end_hour,end_minute);
      // printf("The difference ON-OFF : %02d:%02d\n", hours_diff, minutes_diff);
      // printf("The difference CURRENT-OFF : %02d:%02d\n", hours_diff2, minutes_diff2);
      // printf("diff2time %ld - %ld\n",diff_time,diff_time2);

      if (diff_time2<=diff_time)
      {
        // printf("on\n");
        if (diff_time2==0)
        {
          return false;
        }
        else return true;
      }

      else if(diff_time2>diff_time)
      {
        // printf("off\n");
        return false;
      }

      else return false;

      return false;
    }

    uint8_t diff2time(int start_hour, int end_hour,
      int start_minute, int end_minute)
      {
        struct tm time;
        readValue(&time);

        int hours_diff, minutes_diff;
        double diff;
        time_t end, start;

        struct tm times;
        times.tm_sec = 0;
        times.tm_min = 0;
        times.tm_hour = 0;
        times.tm_mday = 0;
        times.tm_mon = 0;
        times.tm_year = 0;
        times.tm_wday = 0;
        times.tm_yday = 0;

        times.tm_hour=start_hour;
        times.tm_min=start_minute;
        start = mktime(&times);
        times.tm_hour=end_hour;
        times.tm_min=end_minute;
        end = mktime(&times);
        if( end < start )
        {
          end += 24 * 60 * 60;
        }
        diff = difftime(end, start);
        hours_diff = (int) diff / 3600;
        minutes_diff = (int) diff % 3600 / 60;

        const int MINUTES_PER_HOUR = 60;

        int diff_time = hours_diff*MINUTES_PER_HOUR + minutes_diff;

        return diff_time/60;
      }
