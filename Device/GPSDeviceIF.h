#ifndef _GPS_DEVICE_IF_H
#define _GPS_DEVICE_IF_H

#include <stddef.h>

struct GPSDeviceIF {
	int (*open)(void);
	int (*read)(void *buffer, size_t len);
	int (*write)(void *buffer, size_t len);
	int (*close)(void);
};

extern void GPSDeviceInit(void);

extern const struct GPSDeviceIF *GetGPSSNSDevice(void);

extern const struct GPSDeviceIF *GetGPSComDevice(void);

extern const struct GPSDeviceIF *GetGPSDmdsDevice(void);

#endif