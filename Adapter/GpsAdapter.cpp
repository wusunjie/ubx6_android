#include <hardware/gps.h>

#include <new>

#include <string.h>

#include "Engine/GpsEngine.h"

#include "Common/CommonDefs.h"
#include "Common/MsgQueue.h"
#include "Common/GPSLog.h"

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

static GpsLocation locationInfo;

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

MERBOK_EXTERN_C_BEGIN

MERBOK_GPS_LOCAL const GpsInterface* GetGpsInterfaceInst(void)
{
    return &GpsInterfaceInst;
}

MERBOK_EXTERN_C_END

static GpsCallbacks callbacks;

static struct GpsDataCallbacks cbs = {
    .rmc_func = GpsDataRMCCB,
    .gga_func = GpsDataGGACB,
};

struct AdapterThreadCtx {
    int running;
};

static struct AdapterThreadCtx context;

class MsgHandlerStop: public MsgHandler {
private:
    struct AdapterThreadCtx *context;
    int running;
public:

    MsgHandlerStop(struct AdapterThreadCtx *ctx, int r)
    :context(ctx)
    ,running(r)
    {

    }

    void proc(void)
    {
        context->running = running;
    }
};

class MsgHandlerInjectLocation: public MsgHandler {
private:
    GpsLocation *context;
    double latitude;
    double longitude;
    float accuracy;
public:

    MsgHandlerInjectLocation(GpsLocation *ctx, double lati, double longi, float a)
    :context(ctx)
    ,latitude(lati)
    ,longitude(longi)
    ,accuracy(a)
    {

    }

    void proc(void)
    {
        context->latitude = latitude;
        context->longitude = longitude;
        context->accuracy = accuracy;
        callbacks.location_cb(context);
    }
};

static int GpsAdapterInit(GpsCallbacks *cb)
{
    GnssSystemInfo sysInfo = {
        .size = sizeof(GnssSystemInfo),
        .year_of_hw = 2017
    };

    memset(&locationInfo, 0, sizeof(GpsLocation));
    locationInfo = {
        .size = sizeof(GpsLocation),
        .flags =
        GPS_LOCATION_HAS_LAT_LONG | GPS_LOCATION_HAS_ALTITUDE
        | GPS_LOCATION_HAS_SPEED | GPS_LOCATION_HAS_BEARING |
        GPS_LOCATION_HAS_ACCURACY,
    };
    memset(&context, 0, sizeof(AdapterThreadCtx));

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
    callbacks.acquire_wakelock_cb();
    MsgQueueFlush();
    callbacks.release_wakelock_cb();
    GpsEngineInit(&cbs);
    callbacks.set_system_info_cb(&sysInfo);
    callbacks.set_capabilities_cb(GPS_CAPABILITY_SCHEDULING | GPS_CAPABILITY_SINGLE_SHOT);

    return 0;
}

static int GpsAdapterStart(void)
{
    GPSLOGD("GpsAdapterStart");
    context.running = 1;
    callbacks.create_thread_cb("GpsAdapterThreadEntry", GpsAdapterThreadEntry, NULL);
    return 0;
}

static void GpsAdapterThreadEntry(void *arg)
{
    (void)arg;
    GPSLOGD("GpsAdapterThreadEntry");
    GpsEngineSetup();

    while (context.running) {
        MsgHandler *handler = NULL;
        callbacks.acquire_wakelock_cb();
        while ((handler = MsgQueueRecv())) {
            callbacks.release_wakelock_cb();
            handler->proc();
            delete handler;
            callbacks.acquire_wakelock_cb();
        }
        callbacks.release_wakelock_cb();
        GpsEnginePollEvent();
    }
}

static int GpsAdapterStop(void)
{
    GPSLOGD("GpsAdapterStop");
    MsgHandlerStop *stop = new (std::nothrow) MsgHandlerStop(&context, 0);
    if (stop) {
        callbacks.acquire_wakelock_cb();
        MsgQueueSend(stop);
        callbacks.release_wakelock_cb();
        return 0;
    }
    return -1;
}

static void GpsAdapterCleanup(void)
{
    GPSLOGD("GpsAdapterCleanup");
}

static int GpsAdapterInjectTime(GpsUtcTime time, int64_t timeReference, int uncertainty)
{
    GPSLOGD("GpsAdapterInjectTime");
    (void)time;
    (void)timeReference;
    (void)uncertainty;
    return 0;
}

static int GpsAdapterInjectLocation(double latitude, double longitude, float accuracy)
{
    GPSLOGD("GpsAdapterInjectLocation");
    MsgHandlerInjectLocation *injectLocation = new (std::nothrow) MsgHandlerInjectLocation(&locationInfo, latitude, longitude, accuracy);
    if (injectLocation) {
        callbacks.acquire_wakelock_cb();
        MsgQueueSend(injectLocation);
        callbacks.release_wakelock_cb();
        return 0;
    }
    return -1;
}

static void GpsAdapterDeleteAidingData(GpsAidingData flags)
{
    GPSLOGD("GpsAdapterDeleteAidingData: flags 0x%x", flags);
    (void)flags;
}

static int GpsAdapterSetPositionMode(GpsPositionMode mode, GpsPositionRecurrence recurrence,
    uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time)
{
    GPSLOGD("GpsAdapterSetPositionMode: mode %d", mode);
    (void)mode;
    (void)recurrence;
    (void)min_interval;
    (void)preferred_accuracy;
    (void)preferred_time;
    return 0;
}

static const void *GpsAdapterGetExtension(const char *name)
{
    GPSLOGD("GpsAdapterGetExtension: %s", name);
    (void)name;
    return NULL;
}

static void GpsDataRMCCB(struct GPS_NMEA_RMC_DATA *data)
{
    GPSLOGD("GpsDataRMCCB:latitude %f, longitude %f, speed %f, bearing %f",
        data->latitude, data->longitude, data->speed, data->course);
    locationInfo.latitude = data->latitude;
    locationInfo.longitude = data->longitude;
    locationInfo.speed = data->speed;
    locationInfo.bearing = data->course;
    callbacks.location_cb(&locationInfo);
}

static void GpsDataGGACB(struct GPS_NMEA_GGA_DATA *data)
{
    GPSLOGD("GpsDataGGACB:altitude %f, accuracy %f", data->altitude, data->hdop);
    locationInfo.altitude = data->altitude;
    locationInfo.accuracy = data->hdop;
    callbacks.location_cb(&locationInfo);
}
