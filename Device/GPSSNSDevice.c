#include "GPSDeviceIF.h"

#include "GPSDevice.h"

#include "Pdrd_Def.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>


extern void GPSDeviceSetBase(struct GPSDeviceBase *base, enum GPSEventMode mode);

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

extern void GPSSNSDeviceInit(void)
{
	GPSDeviceSetBase(&SNSDevice, GPS_EVENT_MODE_NONBLOCK);
}

extern const struct GPSDeviceIF *GetGPSSNSDevice(void)
{
	return &SNSDeviceImp;
}

static int GPSSNSOpen(void)
{
	int fd = open(GPSSNS_DEVICE_PATH, O_RDWR);
	if (-1 == fd) {
		return -1;
	}

	if (GPS_EVENT_MODE_NONBLOCK == SNSDevice.mode) {
		int flags = fcntl(fd, F_GETFL, 0);
		flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
		if ((-1 == flags) || ((O_NONBLOCK & flags) != flags)) {
			/* roll back to BLOCK mode */
			SNSDevice.mode = GPS_EVENT_MODE_BLOCK;
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

	if (SNSDevice.mode == GPS_EVENT_MODE_NONBLOCK) {
		if (-1 == GPSEventInit(&SNSDevice.event, fd, SNSDevice.mode)) {
			close(fd);
			return -1;
		}
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
