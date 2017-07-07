#ifndef _GPS_DEVICE_IF_H
#define _GPS_DEVICE_IF_H

#include <stddef.h>

#include "Common/CommonDefs.h"

struct GPSDeviceIF {
    int (*open)(void);
    int (*read)(void *buffer, size_t len);
    int (*write)(void *buffer, size_t len);
    int (*close)(void);
};

extern MERBOK_GPS_LOCAL void GPSDeviceInit(void);

extern MERBOK_GPS_LOCAL const struct GPSDeviceIF *GetGPSSNSDevice(void);

extern MERBOK_GPS_LOCAL const struct GPSDeviceIF *GetGPSComDevice(void);

extern MERBOK_GPS_LOCAL const struct GPSDeviceIF *GetGPSDmdsDevice(void);

#endif
