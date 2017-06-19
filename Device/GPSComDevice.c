#include "GPSDevice.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>

extern void GPSDeviceSetOpt(struct GPSDeviceBase *device, enum GPSEventMode mode, OpenDeviceOpt OpenDevice);

static struct GPSDeviceBase ComDevice = {
	.fd = -1;
};

static int GPSComOpen(enum GPSEventMode mode);

static const char GPSCOM_DEVICE_PATH[] = "/dev/GPSCOM";

extern void GPSComDeviceInit(void)
{
	GPSDeviceSetOpt(&ComDevice, GPS_EVENT_MODE_NONBLOCK, GPSComOpen);
}

extern struct GPSDeviceIF *GetGPSComDevice(void)
{
	return &(ComDevice.imp);
}

static int GPSComOpen(void)
{
	if (-1 != ComDevice.fd) {
		return 0;
	}

	int fd = open(GPSCOM_DEVICE_PATH, O_RDWR);
	if (-1 == fd) {
		return -1;
	}

	if (GPS_EVENT_MODE_NONBLOCK == device->mode) {
		int flags = fcntl(fd, F_GETFL, 0);
		flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
		if ((-1 == flags) || (O_NONBLOCK & flags != flags)) {
			/* roll back to BLOCK mode */
			device->mode = GPS_EVENT_MODE_BLOCK;
		}
	}

	/* set serial port parameters. */

	if (GPS_EVENT_MODE_NONBLOCK == device->mode) {
		if (-1 == GPSEventInit(&ComDevice.event, fd, device->mode)) {
			close(fd);
			return -1;
		}
	}

	ComDevice.fd = fd;
	return 0;
}