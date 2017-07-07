#include <hardware/gps.h>

#include <stdlib.h>
#include <string.h>

#include "Common/CommonDefs.h"

MERBOK_GPS_LOCAL const GpsInterface* GetGpsInterfaceInst(void);

static int merbok_gps_module_open(const struct hw_module_t* module, const char* id,
            struct hw_device_t** device);
static const GpsInterface* merbok_gps_get_interface(struct gps_device_t* dev);
static int merbok_gps_device_close(struct hw_device_t* device);

static hw_module_methods_t merbok_gps_module_method = {
	.open = merbok_gps_module_open
};

MERBOK_GPS_API struct hw_module_t HAL_MODULE_INFO_SYM = {
	.tag = HARDWARE_MODULE_TAG,
	.module_api_version = 0,
	.hal_api_version = 0,
	.id = GPS_HARDWARE_MODULE_ID,
	.name = "Merbok GPS Module",
	.author = "Sunjie Wu",
	.methods = &merbok_gps_module_method,
};

static int merbok_gps_module_open(const struct hw_module_t *module, const char *id,
            struct hw_device_t **device)
{
    (void)id;

    struct gps_device_t *dev = (struct gps_device_t *)malloc(sizeof(*dev));

    if(dev == NULL)
        return -1;

    memset(dev, 0, sizeof(*dev));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t *)module;
    dev->common.close = merbok_gps_device_close;
    dev->get_gps_interface = merbok_gps_get_interface;

    *device = (struct hw_device_t*)dev;
    return 0;
}

static const GpsInterface *merbok_gps_get_interface(struct gps_device_t* dev)
{
    (void)dev;

    return GetGpsInterfaceInst();
}

static int merbok_gps_device_close(struct hw_device_t* device)
{
    free(device);
    return 0;
}