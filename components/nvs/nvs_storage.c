#include "nvs_storage.h"

void save_ratio(ratio_val ptr)
{
        // Open
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
        }
        else
        {
                // printf("Write ratio_state Done\n");
                // Write
                // printf("Updating restart counter in NVS ... ");
                err = nvs_set_blob(my_handle, "ratio_state", &ptr, sizeof(ratio_val));
                // printf((err != ESP_OK) ? "Failed!\n" : "ratio_state Done\n");

                // printf("Committing updates in NVS ... ");
                err = nvs_commit(my_handle);
                printf((err != ESP_OK) ? "Failed!\n" : "Write ratio_state Done\n");

                // Close
                nvs_close(my_handle);
        }
}

bool read_ratio(ratio_val *ptr)
{
        // Open
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
                return false;
        }
        else
        {
                // printf("Read ratio_state Done\n");

                // Read
                // printf("Reading restart counter from NVS ...\n");
                size_t size_read = 0;
                err = nvs_get_blob(my_handle, "ratio_state", NULL, &size_read);
                if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
                        return err;

                // printf("size :%d\n", size_read);

                err = nvs_get_blob(my_handle, "ratio_state", ptr, &size_read);
                switch (err)
                {
                case ESP_OK:
                        // printf("Done\n");
                        // printf("Restart counter = %d\n", size_read);
                        break;
                case ESP_ERR_NVS_NOT_FOUND:
                        // printf("The value is not initialized yet!\n");
                        return false;
                        break;
                default:
                        // printf("Error (%d) reading!\n", err);
                        return false;
                        break;
                }
                nvs_close(my_handle);
                return true;
        }
}

void save_ph_kvalue(ph_str_val ptr)
{
        // Write
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
        }
        else
        {
                // printf("Write ph_state Done\n");
                // Write
                // printf("Updating restart counter in NVS ... ");
                err = nvs_set_blob(my_handle, "ph_state", &ptr, sizeof(ph_str_val));
                // printf((err != ESP_OK) ? "Failed!\n" : "ph_state Done\n");

                // printf("Committing updates in NVS ... ");
                err = nvs_commit(my_handle);
                printf((err != ESP_OK) ? "Failed!\n" : "Write ph_state Done\n");

                // Close
                nvs_close(my_handle);
        }
}

bool read_ph_kvalue(ph_str_val *ptr)
{
        // Open
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
                return false;
        }
        else
        {
                // printf("Read ph_state Done\n");

                // Read
                // printf("Reading restart counter from NVS ...\n");
                size_t size_read = 0;
                err = nvs_get_blob(my_handle, "ph_state", NULL, &size_read);
                if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
                        return err;

                // printf("size :%d\n", size_read);

                err = nvs_get_blob(my_handle, "ph_state", ptr, &size_read);
                switch (err)
                {
                case ESP_OK:
                        // printf("Done\n");
                        // printf("Restart counter = %d\n", size_read);
                        break;
                case ESP_ERR_NVS_NOT_FOUND:
                        // printf("The value is not initialized yet!\n");
                        return false;
                        break;
                default:
                        // printf("Error (%d) reading!\n", err);
                        return false;
                        break;
                }
                nvs_close(my_handle);
                return true;
        }
}

void save_ec_kvalue(ec_str_val ptr)
{
        // Write
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
        }
        else
        {
                // printf("Write ph_state Done\n");
                // Write
                // printf("Updating restart counter in NVS ... ");
                err = nvs_set_blob(my_handle, "ec_state", &ptr, sizeof(ec_str_val));
                // printf((err != ESP_OK) ? "Failed!\n" : "ph_state Done\n");

                // printf("Committing updates in NVS ... ");
                err = nvs_commit(my_handle);
                printf((err != ESP_OK) ? "Failed!\n" : "Write ec_state Done\n");

                // Close
                nvs_close(my_handle);
        }
}

bool read_ec_kvalue(ec_str_val *ptr)
{
        // Open
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
                return false;
        }
        else
        {
                // printf("Read ec_state Done\n");

                // Read
                // printf("Reading restart counter from NVS ...\n");
                size_t size_read = 0;
                err = nvs_get_blob(my_handle, "ec_state", NULL, &size_read);
                if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
                        return err;

                // printf("size :%d\n", size_read);

                err = nvs_get_blob(my_handle, "ec_state", ptr, &size_read);
                switch (err)
                {
                case ESP_OK:
                        // printf("Done\n");
                        // printf("Restart counter = %d\n", size_read);
                        break;
                case ESP_ERR_NVS_NOT_FOUND:
                        // printf("The value is not initialized yet!\n");
                        return false;
                        break;
                default:
                        // printf("Error (%d) reading!\n", err);
                        return false;
                        break;
                }
                nvs_close(my_handle);
                return true;
        }
}

void save_time(struct tm *start_time)
{
        // Write
        // printf("\n");
        // printf("save start_time_state\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
        }
        else
        {
                // printf("Write start_time_state Done\n");
                // Write
                // printf("Updating restart counter in NVS ... ");
                err = nvs_set_blob(my_handle, "time_state", start_time, sizeof(struct tm));
                // printf((err != ESP_OK) ? "Failed!\n" : "start_time_state Done\n");

                // printf("Committing updates in NVS ... ");
                err = nvs_commit(my_handle);
                printf((err != ESP_OK) ? "Failed!\n" : "Write start_time_state Done\n");

                // Close
                nvs_close(my_handle);
        }
}

bool read_time(struct tm *start_time)
{
        // Open
        // printf("\n");
        // printf("read start_time_state\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
                return false;
        }
        else
        {
                // printf("Read start_time_state Done\n");
                // Read
                // printf("Reading restart counter from NVS ...\n");
                size_t size_read = 0;
                err = nvs_get_blob(my_handle, "time_state", NULL, &size_read);
                if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
                        return err;

                // printf("size :%d\n", size_read);

                err = nvs_get_blob(my_handle, "time_state", start_time, &size_read);
                switch (err)
                {
                case ESP_OK:
                        // printf("Done\n");
                        // printf("Restart counter = %d\n", size_read);

                        break;
                case ESP_ERR_NVS_NOT_FOUND:
                        // printf("The value is not initialized yet!\n");
                        return false;
                        break;
                default:
                        // printf("Error (%d) reading!\n", err);
                        return false;
                        break;
                }
                nvs_close(my_handle);
                return true;
        }
}

void save_program(timepg ptr)
{
        // Write
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
        }
        else
        {
                // printf("Write pg_state Done\n");
                // Write
                // printf("Updating restart counter in NVS ... ");
                err = nvs_set_blob(my_handle, "pg_state", &ptr, sizeof(timepg));
                // printf((err != ESP_OK) ? "Failed!\n" : "pg_state Done\n");

                // printf("Committing updates in NVS ... ");
                err = nvs_commit(my_handle);
                printf((err != ESP_OK) ? "Failed!\n" : "Write pg_state Done\n");

                // Close
                nvs_close(my_handle);
        }
}

bool read_program(timepg *ptr)
{
        // Open
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
                return false;
        }
        else
        {
                // printf("Read pg_state Done\n");

                // Read
                // printf("Reading restart counter from NVS ...\n");
                size_t size_read = 0;
                err = nvs_get_blob(my_handle, "pg_state", NULL, &size_read);
                if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
                        return err;

                // printf("size :%d\n", size_read);

                err = nvs_get_blob(my_handle, "pg_state", ptr, &size_read);
                switch (err)
                {
                case ESP_OK:
                        // printf("Done\n");
                        // printf("Restart counter = %d\n", size_read);
                        break;
                case ESP_ERR_NVS_NOT_FOUND:
                        // printf("The value is not initialized yet!\n");
                        return false;
                        break;
                default:
                        // printf("Error (%d) reading!\n", err);
                        return false;
                        break;
                }
                nvs_close(my_handle);
                return true;
        }
}

void save_ferti(ferti_set ptr)
{
        // Open
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
        }
        else
        {
                // printf("Write fertilizer Done\n");
                // Write
                // printf("Updating restart counter in NVS ... ");
                err = nvs_set_blob(my_handle, "fertilizer", &ptr, sizeof(ferti_set));
                // printf((err != ESP_OK) ? "Failed!\n" : "fertilizer Done\n");

                // printf("Committing updates in NVS ... ");
                err = nvs_commit(my_handle);
                printf((err != ESP_OK) ? "Failed!\n" : "Write fertilizer Done\n");

                // Close
                nvs_close(my_handle);
        }
}

bool read_ferti(ferti_set *ptr)
{
        // Open
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
                return false;
        }
        else
        {
                // printf("Read fertilizer Done\n");

                // Read
                // printf("Reading restart counter from NVS ...\n");
                size_t size_read = 0;
                err = nvs_get_blob(my_handle, "fertilizer", NULL, &size_read);
                if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
                        return err;

                // printf("size :%d\n", size_read);

                err = nvs_get_blob(my_handle, "fertilizer", ptr, &size_read);
                switch (err)
                {
                case ESP_OK:
                        // printf("Done\n");
                        // printf("Restart counter = %d\n", size_read);
                        break;
                case ESP_ERR_NVS_NOT_FOUND:
                        // printf("The value is not initialized yet!\n");
                        return false;
                        break;
                default:
                        // printf("Error (%d) reading!\n", err);
                        return false;
                        break;
                }
                nvs_close(my_handle);
                return true;
        }
}

void save_environment(environment ptr)
{
        // Write
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
        }
        else
        {
                // printf("Write environment Done\n");
                // Write
                // printf("Updating restart counter in NVS ... ");
                err = nvs_set_blob(my_handle, "en_state", &ptr, sizeof(environment));
                // printf((err != ESP_OK) ? "Failed!\n" : "environment Done\n");

                // printf("Committing updates in NVS ... ");
                err = nvs_commit(my_handle);
                printf((err != ESP_OK) ? "Failed!\n" : "Write environment Done\n");

                // Close
                nvs_close(my_handle);
        }
}

bool read_environment(environment *ptr)
{
        // Open
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
                return false;
        }
        else
        {
                // printf("Read environment Done\n");

                // Read
                // printf("Reading restart counter from NVS ...\n");
                size_t size_read = 0;
                err = nvs_get_blob(my_handle, "en_state", NULL, &size_read);
                if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
                        return err;

                // printf("size :%d\n", size_read);

                err = nvs_get_blob(my_handle, "en_state", ptr, &size_read);
                switch (err)
                {
                case ESP_OK:
                        // printf("Done\n");
                        // printf("Restart counter = %d\n", size_read);
                        break;
                case ESP_ERR_NVS_NOT_FOUND:
                        // printf("The value is not initialized yet!\n");
                        return false;
                        break;
                default:
                        // printf("Error (%d) reading!\n", err);
                        return false;
                        break;
                }
                nvs_close(my_handle);
                return true;
        }
}

void save_statuspg(statuspg ptr)
{
        // Write
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
        }
        else
        {
                // printf("Write statuspg Done\n");
                // Write
                // printf("Updating restart counter in NVS ... ");
                err = nvs_set_blob(my_handle, "statuspg", &ptr, sizeof(statuspg));
                // printf((err != ESP_OK) ? "Failed!\n" : "statuspg Done\n");

                // printf("Committing updates in NVS ... ");
                err = nvs_commit(my_handle);
                printf((err != ESP_OK) ? "Failed!\n" : "Write statuspg Done\n");

                // Close
                nvs_close(my_handle);
        }
}

bool read_statuspg(statuspg *ptr)
{
        // Open
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
                return false;
        }
        else
        {
                // printf("Read statuspg Done\n");

                // Read
                // printf("Reading restart counter from NVS ...\n");
                size_t size_read = 0;
                err = nvs_get_blob(my_handle, "statuspg", NULL, &size_read);
                if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
                        return err;

                // printf("size :%d\n", size_read);

                err = nvs_get_blob(my_handle, "statuspg", ptr, &size_read);
                switch (err)
                {
                case ESP_OK:
                        // printf("Done\n");
                        // printf("Restart counter = %d\n", size_read);
                        break;
                case ESP_ERR_NVS_NOT_FOUND:
                        // printf("The value is not initialized yet!\n");
                        return false;
                        break;
                default:
                        // printf("Error (%d) reading!\n", err);
                        return false;
                        break;
                }
                nvs_close(my_handle);
                return true;
        }
}

esp_err_t resetfactory()
{
        esp_err_t ret;
        ret = nvs_flash_erase();
        return ret;
}

void load_default_nvs()
{

        for (size_t i = 0; i < 4; i++)
        {

                ratio_led.bright_1[i] = 100;
                ratio_led.bright_2[i] = 100;
                ratio_led.bright_3[i] = 100;
                ratio_led.bright_4[i] = 100;
        }
        save_ratio(ratio_led);
        save_ratio(ratio_led);

        for (uint8_t i = 0; i < 4; i++)
        {
                for(uint8_t j=0; j<4; j++ )
                {
                        time_pg.dayon[i] = 1;
                        time_pg.dayoff[i] = 2;

                        time_pg.pg_hour_on[i][j] = 18;
                        time_pg.pg_min_on[i][j] = 0;


                        time_pg.pg_hour_off[i][j] = 8;
                        time_pg.pg_min_off[i][j] = 0;


                        time_pg.bright_1[i][j] = 100;
                        time_pg.bright_2[i][j] = 100;
                        time_pg.bright_3[i][j] = 100;
                        time_pg.bright_4[i][j] = 100;
                }
        }
        save_program(time_pg);

        _environment.fill1_state=0;
        _environment.fill2_state=0;
        _environment.fill3_state=0;
        _environment.fill4_state=0;
        _environment.pump_ph_state=0;
        _environment.solenoide_state=0;
        _environment.pump_water_state=0;
        save_environment(_environment);

        ferti_set_val.ec_set_point = 12;
        ferti_set_val.ph_set_point = 68;
        for (uint8_t i = 0; i < 4; i++)
        {
                ferti_set_val.ratio_fer[i] = 1;
                ferti_set_val.ratio_fer[i] = 1;
                ferti_set_val.ratio_fer[i] = 1;
        }
        save_ferti(ferti_set_val);

        working_timer.working_on_h[0] = 8;
        working_timer.working_on_m[0] = 0;
        working_timer.working_off_h[0] = 8;
        working_timer.working_off_m[0] = 10;

        working_timer.working_on_h[1] = 9;
        working_timer.working_on_m[1] = 0;
        working_timer.working_off_h[1] = 9;
        working_timer.working_off_m[1] = 10;

        working_timer.working_on_h[2] = 10;
        working_timer.working_on_m[2] = 0;
        working_timer.working_off_h[2] = 10;
        working_timer.working_off_m[2] = 10;

        working_timer.working_on_h[3] = 11;
        working_timer.working_on_m[3] = 0;
        working_timer.working_off_h[3] = 11;
        working_timer.working_off_m[3] = 10;

        working_timer.status_timer[0] = 0;
        working_timer.status_timer[1] = 0;
        working_timer.status_timer[2] = 0;
        working_timer.status_timer[3] = 0;


        working_timer.working_day = 0;
        working_timer.working_nextday = 0;
        working_timer.working_lastday = 0;

        save_working(working_timer);

        status_pg.switch_mode = 0;
        status_pg.OTA_NEXTION = 1;
        save_statuspg(status_pg);

        read_statuspg(&status_pg);
        sprintf(str_name, DAY_PLANTED, status_pg.start_day);
        send_tft(str_name);
        //wepo to status
        sprintf(str_name, SAVE_EEPROM_TFT, status_pg.switch_mode, 30);
        send_tft(str_name);
        sprintf(str_name, DEVICE_NO, status_pg.mydevice_no);
        send_tft(str_name);
}

void save_working(str_working ptr)
{
        // Open
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
        }
        else
        {
                // printf("Write working Done\n");
                // Write
                // printf("Updating restart counter in NVS ... ");
                err = nvs_set_blob(my_handle, "working", &ptr, sizeof(str_working));
                // printf((err != ESP_OK) ? "Failed!\n" : "working Done\n");

                // printf("Committing updates in NVS ... ");
                err = nvs_commit(my_handle);
                printf((err != ESP_OK) ? "Failed!\n" : "Write working Done\n");

                // Close
                nvs_close(my_handle);
        }
}

bool read_working(str_working *ptr)
{
        // Open
        // printf("\n");
        // printf("Opening Non-Volatile Storage (NVS) handle...\n");
        nvs_handle my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK)
        {
                printf("Error (%d) opening NVS handle!\n", err);
                return false;
        }
        else
        {
                // printf("Read working Done\n");

                // Read
                // printf("Reading restart counter from NVS ...\n");
                size_t size_read = 0;
                err = nvs_get_blob(my_handle, "working", NULL, &size_read);
                if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
                        return err;

                // printf("size :%d\n", size_read);

                err = nvs_get_blob(my_handle, "working", ptr, &size_read);
                switch (err)
                {
                case ESP_OK:
                        // printf("Done\n");
                        // printf("Restart counter = %d\n", size_read);
                        break;
                case ESP_ERR_NVS_NOT_FOUND:
                        // printf("The value is not initialized yet!\n");
                        return false;
                        break;
                default:
                        // printf("Error (%d) reading!\n", err);
                        return false;
                        break;
                }
                nvs_close(my_handle);
                return true;
        }
}
