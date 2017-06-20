#include "GPSDeviceIF.h"
#include "GPSDevice.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>

extern void GPSDeviceSetBase(struct GPSDeviceBase *base, enum GPSEventMode mode);

static int GPSComOpen(void);
static int GPSComRead(void *buffer, size_t len);
static int GPSComWrite(void *buffer, size_t len);
static int GPSComClose(void);

static struct GPSDeviceBase ComDevice = {
	.fd = -1
};

static const char GPSCOM_DEVICE_PATH[] = "/dev/GPSCOM";

extern void GPSComDeviceInit(void)
{
	GPSDeviceSetBase(&ComDevice, GPS_EVENT_MODE_NONBLOCK);
}

extern struct GPSDeviceIF *GetGPSComDevice(void)
{
	return NULL;
}

static int GPSComOpen(void)
{
	int fd = open(GPSCOM_DEVICE_PATH, O_RDWR);
	if (-1 == fd) {
		return -1;
	}

	if (GPS_EVENT_MODE_NONBLOCK == ComDevice.mode) {
		int flags = fcntl(fd, F_GETFL, 0);
		flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
		if ((-1 == flags) || (O_NONBLOCK & flags != flags)) {
			/* roll back to BLOCK mode */
			ComDevice.mode = GPS_EVENT_MODE_BLOCK;
		}
	}

	/* set serial port parameters. */

	if (GPS_EVENT_MODE_NONBLOCK == ComDevice.mode) {
		if (-1 == GPSEventInit(&ComDevice.event, fd, ComDevice.mode)) {
			close(fd);
			return -1;
		}
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
