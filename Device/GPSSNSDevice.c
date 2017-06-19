#include "GPSDevice.h"

#include "Pdrd_Def.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

extern void GPSDeviceSetOpt(struct GPSDeviceBase *device, enum GPSEventMode mode, OpenDeviceOpt OpenDevice);

static struct GPSDeviceBase SNSDevice = {
	.fd = -1;
};

static int GPSSNSOpen(enum GPSEventMode mode);

static const char GPSSNS_DEVICE_PATH[] = "/dev/pdrdctl";

extern void GPSSNSDeviceInit(void)
{
	GPSDeviceSetOpt(&SNSDevice, GPS_EVENT_MODE_NONBLOCK, GPSSNSOpen);
}

extern struct GPSDeviceIF *GetGPSSNSDevice(void)
{
	return &(SNSDevice.imp);
}

static int GPSSNSOpen(void)
{
	if (-1 != SNSDevice.fd) {
		return 0;
	}

	int fd = open(GPSSNS_DEVICE_PATH, O_RDWR);
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

	PDRD_SAMPLEGAPCTL GapCtrl;
	GapCtrl.SensorGap = 10;
	GapCtrl.TemperatureGap = 50;
	GapCtrl.TotalGap = 200;

	int ret = ioctl(fd, IOC_PDRD_SETSAMPLERATE, &GapCtrl);
	if (-1 == ret) {
		close(fd);
		return -1;
	}

	if (device->mode == GPS_EVENT_MODE_NONBLOCK) {
		if (-1 == GPSEventInit(&SNSDevice.event, fd, device->mode)) {
			close(fd);
			return -1;
		}
	}

	SNSDevice.fd = fd;
	return 0;
}