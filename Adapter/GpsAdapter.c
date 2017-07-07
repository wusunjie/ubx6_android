#include <hardware/gps.h>

#include <pthread.h>

#include "Engine/GpsEngine.h"
#include "Common/CommonDefs.h"

static void GpsAdapterThreadEntry(void* arg);
static void GpsDataRMCCB(struct GPS_NMEA_RMC_DATA *data);
static void GpsDataGGACB(struct GPS_NMEA_GGA_DATA *data);

static int GpsAdapterInit(GpsCallbacks *cb);
static int GpsAdapterStart(void);
static int GpsAdapterStop(void);
static void GpsAdapterCleanup(void);
static int GpsAdapterInjectTime(GpsUtcTime time, int64_t timeReference, int uncertainty);
static int GpsAdapterInjectLocation(double latitude, double longitude, float accuracy);
static void GpsAdapterDeleteAidingData(GpsAidingData flags);
static int GpsAdapterSetPositionMode(GpsPositionMode mode, GpsPositionRecurrence recurrence,
	uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time);
static const void *GpsAdapterGetExtension(const char* name);

static GpsCallbacks callbacks;

static GpsLocation locationInfo = {
	.size = sizeof(GpsLocation),
	.flags =
	GPS_LOCATION_HAS_LAT_LONG | GPS_LOCATION_HAS_ALTITUDE
	| GPS_LOCATION_HAS_SPEED | GPS_LOCATION_HAS_BEARING |
	GPS_LOCATION_HAS_ACCURACY,
};

static const GpsInterface GpsInterfaceInst = {
	.size = sizeof(GpsInterface),
	.init = GpsAdapterInit,
	.start = GpsAdapterStart,
	.stop = GpsAdapterStop,
	.cleanup = GpsAdapterCleanup,
	.inject_time = GpsAdapterInjectTime,
	.inject_location = GpsAdapterInjectLocation,
	.delete_aiding_data = GpsAdapterDeleteAidingData,
	.set_position_mode = GpsAdapterSetPositionMode,
	.get_extension = GpsAdapterGetExtension
};

static struct GpsDataCallbacks cbs = {
	.rmc_func = GpsDataRMCCB,
	.gga_func = GpsDataGGACB,
};

MERBOK_GPS_LOCAL const GpsInterface* GetGpsInterfaceInst(void)
{
	return &GpsInterfaceInst;
}

static int GpsAdapterInit(GpsCallbacks *cb)
{
	callbacks.location_cb = cb->location_cb;
	callbacks.status_cb = cb->status_cb;
	callbacks.sv_status_cb = cb->sv_status_cb;
	callbacks.nmea_cb = cb->nmea_cb;
	callbacks.set_capabilities_cb = cb->set_capabilities_cb;
	callbacks.acquire_wakelock_cb = cb->acquire_wakelock_cb;
	callbacks.release_wakelock_cb = cb->release_wakelock_cb;
	callbacks.create_thread_cb = cb->create_thread_cb;
	callbacks.request_utc_time_cb = cb->request_utc_time_cb;
	callbacks.set_system_info_cb = cb->set_system_info_cb;
	callbacks.gnss_sv_status_cb = cb->gnss_sv_status_cb;
	callbacks.size = sizeof(GpsCallbacks);
	GpsEngineInit(&cbs);
	return 0;
}

static int GpsAdapterStart(void)
{
	callbacks.create_thread_cb("GpsAdapterThreadEntry", GpsAdapterThreadEntry, NULL);
	return 0;
}

static void GpsAdapterThreadEntry(void *arg)
{
	(void)arg;
	GpsEngineSetup();

	while (1) {
		GpsEnginePollEvent();
	}
}

static int GpsAdapterStop(void)
{
	return 0;
}

static void GpsAdapterCleanup(void)
{

}

static int GpsAdapterInjectTime(GpsUtcTime time, int64_t timeReference, int uncertainty)
{
	(void)time;
	(void)timeReference;
	(void)uncertainty;
	return 0;
}

static int GpsAdapterInjectLocation(double latitude, double longitude, float accuracy)
{
	(void)latitude;
	(void)longitude;
	(void)accuracy;
	return 0;
}

static void GpsAdapterDeleteAidingData(GpsAidingData flags)
{
	(void)flags;
}

static int GpsAdapterSetPositionMode(GpsPositionMode mode, GpsPositionRecurrence recurrence,
	uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time)
{
	(void)mode;
	(void)recurrence;
	(void)min_interval;
	(void)preferred_accuracy;
	(void)preferred_time;
	return 0;
}

static const void *GpsAdapterGetExtension(const char *name)
{
	(void)name;
	return NULL;
}

static void GpsDataRMCCB(struct GPS_NMEA_RMC_DATA *data)
{
	locationInfo.latitude = data->latitude;
	locationInfo.longitude = data->longitude;
	locationInfo.speed = data->speed;
	locationInfo.bearing = data->course;
	callbacks.location_cb(&locationInfo);
}

static void GpsDataGGACB(struct GPS_NMEA_GGA_DATA *data)
{
	locationInfo.altitude = data->altitude;
	locationInfo.accuracy = data->hdop;
	callbacks.location_cb(&locationInfo);
}


