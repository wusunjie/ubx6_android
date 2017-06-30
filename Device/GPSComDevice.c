#include "GPSDeviceIF.h"
#include "GPSDevice.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <termios.h>

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

static const char GPSCOM_DEVICE_PATH[] = "/dev/GPSCOM";

extern void GPSComDeviceInit(void)
{
	GPSDeviceSetBase(&ComDevice, GPS_EVENT_MODE_BLOCK);
}

extern const struct GPSDeviceIF *GetGPSComDevice(void)
{
	return &ComDeviceImp;
}

static int GPSComOpen(void)
{
	int fd = open(GPSCOM_DEVICE_PATH, O_RDWR);
	if (-1 == fd) {
		return -1;
	}

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
