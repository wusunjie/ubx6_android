#ifndef _GPS_ENGINE_H
#define _GPS_ENGINE_H

#include <stdbool.h>

#include "Common/CommonDefs.h"

struct GPS_NMEA_DATE {
    int day;
    int month;
    int year;
};

struct GPS_NMEA_TIME {
    int hours;
    int minutes;
    int seconds;
    int microseconds;
};

struct GPS_NMEA_RMC_DATA {
	struct GPS_NMEA_TIME time;
    bool valid;
    float latitude;
    float longitude;
    float speed;
    float course;
    struct GPS_NMEA_DATE date;
    float variation;
};

struct GPS_NMEA_GGA_DATA {
	struct GPS_NMEA_TIME time;
    float latitude;
    float longitude;
    int fix_quality;
    int satellites_tracked;
    float hdop;
    float altitude;
    char altitude_units;
    float height;
    char height_units;
    int dgps_age;
};

typedef void (*GpsDataRMCCallback)(struct GPS_NMEA_RMC_DATA *data);
typedef void (*GpsDataGGACallback)(struct GPS_NMEA_GGA_DATA *data);

struct GpsDataCallbacks {
    GpsDataRMCCallback rmc_func;
    GpsDataGGACallback gga_func;
};

extern MERBOK_GPS_LOCAL int GpsEngineInit(struct GpsDataCallbacks *cb);

extern MERBOK_GPS_LOCAL int GpsEngineSetup(void);

extern MERBOK_GPS_LOCAL int GpsEnginePollEvent(void);

#endif