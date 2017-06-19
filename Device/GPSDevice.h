#ifndef _GPS_DEVICE_H
#define _GPS_DEVICE_H

#include "Common/GPSEvent.h"

#include <stddef.h>

struct GPSDeviceIF {
	int (*open)(void);
	int (*read)(void *buffer, size_t len);
	int (*write)(void *buffer, size_t len);
	int (*close)(void);
};

struct GPSDeviceBase {
	int (*OpenOpt)(void);
	struct GPSEvent event;
	enum GPSEventMode mode;
	struct GPSDeviceIF imp;
	int fd;
};

typedef int (*OpenDeviceOpt)(enum GPSEventMode mode);

extern void GPSDeviceInit(void);

extern struct GPSDeviceIF *GetGPSSNSDevice(void);

extern struct GPSDeviceIF *GetGPSComDevice(void);

extern struct GPSDeviceIF *GetGPSDmdsDevice(void);

#endif