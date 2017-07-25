#include "Adapter/Configuration.h"

#include "Common/GPSLog.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct merbok_gps_configs {
    uint32_t capabilities;
    uint16_t year_of_hw;
};

static volatile struct merbok_gps_configs merbok_configs = {
    .capabilities = GPS_CAPABILITY_SCHEDULING,
    .year_of_hw = 2015
};

static int first = 1;

static void merbok_configuration_update(const char* config_data, int32_t length);

static const GnssConfigurationInterface configs = {
    .size = sizeof(GnssConfigurationInterface),
    .configuration_update = merbok_configuration_update
};

MERBOK_GPS_LOCAL const GnssConfigurationInterface *GetConfigurationInterface(void)
{
    return &configs;
}

MERBOK_GPS_LOCAL uint32_t GetConfigCapabilities(void)
{
    GPSLOGD("GetConfigCapabilities %x", merbok_configs.capabilities);
    return merbok_configs.capabilities;
}

MERBOK_GPS_LOCAL uint16_t GetConfigYearOfHW(void)
{
    GPSLOGD("GetConfigYearOfHW %d", merbok_configs.year_of_hw);
    return merbok_configs.year_of_hw;
}

static void merbok_configuration_update(const char* config_data, int32_t length)
{
    if (!first) {
        return;
    }

    {
        char *saveptr = NULL;
        char *buffer = (char *)malloc(length + 1);
        char *line = NULL;
        memcpy(buffer, config_data, length);
        buffer[length] = '\0';

        line = strtok_r(buffer, "\n", &saveptr);
        while (line) {
            char *pch = strchr(line, '=');
            if (pch) {
                if (!strncmp("CAPABILITIES", line, 12)) {
                    sscanf(pch + 1, "%x", &(merbok_configs.capabilities));
                    GPSLOGD("Config Update CAPABILITIES: %s", pch + 1);
                }
                else if (!strncmp("YEAR_OF_HW", line, 10)) {
                    sscanf(pch + 1, "%hu", &(merbok_configs.year_of_hw));
                    GPSLOGD("Config Update YEAR_OF_HW: %s", pch + 1);
                }
            }
            line = strtok_r(NULL, "\n", &saveptr);
        }

        free(buffer);
    }

    first = 0;
}