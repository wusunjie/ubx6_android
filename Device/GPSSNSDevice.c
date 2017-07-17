#include "Device/GPSDeviceAPI.h"

#include "Device/GpsDevice.h"
#include "Common/CommonDefs.h"

#include "Pdrd_Def.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>


extern void GPSDeviceSetBase(struct GPSDeviceBase *base);

static int GPSSNSOpen(void);
static int GPSSNSRead(void *buffer, size_t len);
static int GPSSNSWrite(void *buffer, size_t len);
static int GPSSNSClose(void);

static const struct GPSDeviceIF SNSDeviceImp = {
    .open = GPSSNSOpen,
    .read = GPSSNSRead,
    .write = GPSSNSWrite,
    .close = GPSSNSClose
};

static struct GPSDeviceBase SNSDevice = {
    .fd = -1
};

static const char GPSSNS_DEVICE_PATH[] = "/dev/pdrdctl";

MERBOK_GPS_LOCAL void GPSSNSDeviceInit(void)
{
    GPSDeviceSetBase(&SNSDevice);
}

MERBOK_GPS_LOCAL const struct GPSDeviceIF *GetGPSSNSDevice(void)
{
    return &SNSDeviceImp;
}

static int GPSSNSOpen(void)
{
    int fd = open(GPSSNS_DEVICE_PATH, O_RDWR);
    if (-1 == fd) {
        return -1;
    }

    PDRD_SAMPLEGAPCTL GapCtrl;
    GapCtrl.SensorGap = 10;
    GapCtrl.TemperatureGap = 50;
    GapCtrl.TotalGap = 200;

    int ret = ioctl(fd, IOC_PDRD_SETSAMPLERATE, &GapCtrl);
    if (-1 == ret) {
        close(fd);
        return -1;
    }

    return SNSDevice.imp.open(&SNSDevice, fd);
}

static int GPSSNSRead(void *buffer, size_t len)
{
    return SNSDevice.imp.read(&SNSDevice, buffer, len);
}

static int GPSSNSWrite(void *buffer, size_t len)
{
    return SNSDevice.imp.write(&SNSDevice, buffer, len);
}

static int GPSSNSClose(void)
{
    return SNSDevice.imp.close(&SNSDevice);
}
