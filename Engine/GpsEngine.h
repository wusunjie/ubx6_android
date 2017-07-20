#ifndef _GPS_ENGINE_H
#define _GPS_ENGINE_H

#include <stdbool.h>
#include <stdint.h>

#include "Common/CommonDefs.h"

MERBOK_EXTERN_C_BEGIN

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

struct GPS_SV_INFO {
    uint8_t chn;
    uint8_t svid;
    union {
        uint8_t data;
        struct {
            uint8_t svUsed:1;
            uint8_t diffCorr:1;
            uint8_t orbitAvail:1;
            uint8_t orbitEph:1;
            uint8_t unhealthy:1;
            uint8_t orbitAlm:1;
            uint8_t orbitAop:1;
            uint8_t smoothed:1;
        } bits;
    } flags;
    uint8_t cno;
    int8_t elev;
    int16_t azim;
};

typedef void (*GpsDataRMCCallback)(struct GPS_NMEA_RMC_DATA *data);
typedef void (*GpsDataGGACallback)(struct GPS_NMEA_GGA_DATA *data);
typedef void (*GpsDataUTCTimeCallback)(uint64_t utc);
typedef void (*GpsDataSVInfoCallback)(int num, struct GPS_SV_INFO *infos);
typedef void (*GpsDataNMEACallback)(char *nmea, int length);

struct GpsDataCallbacks {
    GpsDataRMCCallback rmc_func;
    GpsDataGGACallback gga_func;
    GpsDataUTCTimeCallback time_func;
    GpsDataSVInfoCallback svinfo_func;
    GpsDataNMEACallback nmea_func;
};

extern MERBOK_GPS_LOCAL int GpsEngineInit(struct GpsDataCallbacks *cb);

extern MERBOK_GPS_LOCAL int GpsEngineSetup(void);

extern MERBOK_GPS_LOCAL int GpsEnginePollEvent(void);

MERBOK_EXTERN_C_END

#endif
