#include "Device/GPSDevice.h"
#include "Common/CommonDefs.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

static int GPSDeviceOpen(struct GPSDeviceBase *device, int fd);
static int GPSDeviceRead(struct GPSDeviceBase *device, void *buffer, size_t len);
static int GPSDeviceWrite(struct GPSDeviceBase *device, void *buffer, size_t len);
static int GPSDeviceClose(struct GPSDeviceBase *device);

MERBOK_EXTERN_C void GPSComDeviceInit(void);

MERBOK_EXTERN_C MERBOK_GPS_LOCAL void GPSDeviceInit(void)
{
    GPSComDeviceInit();
}

MERBOK_EXTERN_C MERBOK_GPS_LOCAL void GPSDeviceSetBase(struct GPSDeviceBase *base, enum GPSEventMode mode)
{
    base->imp.open = GPSDeviceOpen;
    base->imp.read = GPSDeviceRead;
    base->imp.write = GPSDeviceWrite;
    base->imp.close = GPSDeviceClose;
    base->mode = mode;
}

static int GPSDeviceOpen(struct GPSDeviceBase *device, int fd)
{
    device->fd = fd;

    if (GPS_EVENT_MODE_NONBLOCK == device->mode) {
        int flags = fcntl(fd, F_GETFL, 0);
        flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        if ((-1 == flags) || ((O_NONBLOCK & flags) != flags)) {
            /* roll back to BLOCK mode */
            device->mode = GPS_EVENT_MODE_BLOCK;
        }
    }

    /* set serial port parameters. */

    GPSEventInit(&(device->event), fd, device->mode);

    return 0;
}

static int GPSDeviceRead(struct GPSDeviceBase *device, void *buffer, size_t len)
{
    if (-1 == device->fd) {
        return -1;
    }
    
    return GPSEventRead(&(device->event), buffer, len);
}

static int GPSDeviceWrite(struct GPSDeviceBase *device, void *buffer, size_t len)
{
    if (-1 == device->fd) {
        return -1;
    }
    
    return GPSEventWrite(&(device->event), buffer, len);
}

static int GPSDeviceClose(struct GPSDeviceBase *device)
{
    int ret = -1;

    if (-1 == device->fd) {
        return 0;
    }

    ret = GPSEventClose(&(device->event));
    if (-1 == ret) {
        close(device->fd);
        device->fd = -1;
        return -1;
    }

    ret = close(device->fd);
    device->fd = -1;

    return ret;
}
