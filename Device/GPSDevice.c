#include "Device/GPSDevice.h"

static int GPSDeviceOpen(struct GPSDeviceBase *device);
static int GPSDeviceRead(struct GPSDeviceBase *device, void *buffer, size_t len, size_t *result);
static int GPSDeviceWrite(struct GPSDeviceBase *device, void *buffer, size_t len, size_t *result);
static int GPSDeviceClose(struct GPSDeviceBase *device);

extern void GPSSNSDeviceInit(void);
extern void GPSComDeviceInit(void);

extern void GPSDeviceInit(void)
{
	GPSSNSDeviceInit();
	GPSComDeviceInit();
}

extern void GPSDeviceSetOpt(struct GPSDeviceBase *device, enum GPSEventMode mode, OpenDeviceOpt OpenDevice)
{
	device->mode = mode;
	device->OpenOpt = OpenDevice;
	device->imp.open = OpenDeviceOpen;
	device->imp.read = GPSDeviceRead;
	device->imp.write = GPSDeviceWrite;
	device->imp.close = GPSDeviceClose;	
}

static int GPSDeviceOpen(struct GPSDeviceBase *device)
{
	int fd = device->OpenOpt(device, device->mode);
	if (-1 == fd) {
		return -1;
	}

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