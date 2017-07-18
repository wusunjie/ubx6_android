#ifndef _GPS_DEVICE_H
#define _GPS_DEVICE_H

#include <stddef.h>

struct GPSDeviceBase;

struct GPSDeviceImp {
    int (*open)(struct GPSDeviceBase *device, int fd);
    int (*read)(struct GPSDeviceBase *device, void *buffer, size_t len);
    int (*write)(struct GPSDeviceBase *device, void *buffer, size_t len);
    int (*close)(struct GPSDeviceBase *device);
};

struct GPSDeviceBase {
    struct GPSDeviceImp imp;
    int fd;
};

#endif
