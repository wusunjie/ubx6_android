#include "Device/GPSDeviceIF.h"
#include "Device/GPSDevice.h"
#include "Common/CommonDefs.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <termios.h>
#include <unistd.h>
#include <string.h>

extern void GPSDeviceSetBase(struct GPSDeviceBase *base, enum GPSEventMode mode);

static int GPSComOpen(void);
static int GPSComRead(void *buffer, size_t len);
static int GPSComWrite(void *buffer, size_t len);
static int GPSComClose(void);

static const struct GPSDeviceIF ComDeviceImp = {
    .open = GPSComOpen,
    .read = GPSComRead,
    .write = GPSComWrite,
    .close = GPSComClose
};

static struct GPSDeviceBase ComDevice = {
    .fd = -1
};

static const char GPSCOM_DEVICE_PATH[] = "/dev/ttymxc1";

MERBOK_GPS_LOCAL void GPSComDeviceInit(void)
{
    GPSDeviceSetBase(&ComDevice, GPS_EVENT_MODE_BLOCK);
}

MERBOK_GPS_LOCAL const struct GPSDeviceIF *GetGPSComDevice(void)
{
    return &ComDeviceImp;
}

static int GPSComOpen(void)
{
    int fd = open(GPSCOM_DEVICE_PATH, O_RDWR);
    if (-1 == fd) {
        return -1;
    }

    //set com attribute if open success
    struct termios attr;
    memset(&attr, 0, sizeof(attr));

    //set contro/input/output/local flags
    attr.c_cflag |=CLOCAL | CREAD;
    attr.c_cflag &= ~PARENB;
    attr.c_iflag &= ~IXON;
    attr.c_iflag &= ~INPCK;
    attr.c_cflag &= ~CSTOPB;
    attr.c_cflag &= ~CSIZE;
    attr.c_cflag |= CS8;
    attr.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    attr.c_oflag &= ~OPOST;
    attr.c_cc[VTIME] = 0;
    attr.c_cc[VMIN] = 1;

    cfsetispeed(&attr, 9600);
    cfsetospeed(&attr, 9600);
    tcsetattr(fd, TCSANOW, &attr);

    tcflush(fd, TCIOFLUSH);

    return ComDevice.imp.open(&ComDevice, fd);
}

static int GPSComRead(void *buffer, size_t len)
{
    return ComDevice.imp.read(&ComDevice, buffer, len);
}

static int GPSComWrite(void *buffer, size_t len)
{
    return ComDevice.imp.write(&ComDevice, buffer, len);
}

static int GPSComClose(void)
{
    return ComDevice.imp.close(&ComDevice);
}
