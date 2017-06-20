#include "Device/GPSDevice.h"

#include <unistd.h>

static int GPSDeviceOpen(struct GPSDeviceBase *device, int fd);
static int GPSDeviceRead(struct GPSDeviceBase *device, void *buffer, size_t len);
static int GPSDeviceWrite(struct GPSDeviceBase *device, void *buffer, size_t len);
static int GPSDeviceClose(struct GPSDeviceBase *device);

extern void GPSSNSDeviceInit(void);
extern void GPSComDeviceInit(void);

extern void GPSDeviceInit(void)
{
	GPSSNSDeviceInit();
	GPSComDeviceInit();
}

extern void GPSDeviceSetBase(struct GPSDeviceBase *base, enum GPSEventMode mode)
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

	if (-1 == GPSEventInit(&(device->event), device->fd, device->mode)) {
		close(fd);
		device->fd = -1;
		return -1;
	}

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