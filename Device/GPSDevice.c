#include "Device/GPSDevice.h"
#include "Common/CommonDefs.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

static int GPSDeviceOpen(struct GPSDeviceBase *device, int fd);
static int GPSDeviceRead(struct GPSDeviceBase *device, void *buffer, size_t len);
static int GPSDeviceWrite(struct GPSDeviceBase *device, void *buffer, size_t len);
static int GPSDeviceClose(struct GPSDeviceBase *device);

extern void GPSComDeviceInit(void);

MERBOK_GPS_LOCAL void GPSDeviceInit(void)
{
    GPSComDeviceInit();
}

MERBOK_GPS_LOCAL void GPSDeviceSetBase(struct GPSDeviceBase *base)
{
    base->imp.open = GPSDeviceOpen;
    base->imp.read = GPSDeviceRead;
    base->imp.write = GPSDeviceWrite;
    base->imp.close = GPSDeviceClose;
}

static int GPSDeviceOpen(struct GPSDeviceBase *device, int fd)
{
    device->fd = fd;

    return 0;
}

static int GPSDeviceRead(struct GPSDeviceBase *device, void *buffer, size_t len)
{
    if (-1 == device->fd) {
        return -1;
    }
    
    return GPSEventRead(device->fd, buffer, len);
}

static int GPSDeviceWrite(struct GPSDeviceBase *device, void *buffer, size_t len)
{
    if (-1 == device->fd) {
        return -1;
    }
    
    return GPSEventWrite(device->fd, buffer, len);
}

static int GPSDeviceClose(struct GPSDeviceBase *device)
{
    int ret = -1;

    if (-1 == device->fd) {
        return 0;
    }

    ret = GPSEventClose(device->fd);
    device->fd = -1;
    return ret;
}
