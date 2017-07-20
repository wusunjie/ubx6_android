#include <hardware/gps.h>

#include <new>

#include <string.h>

#include "Engine/GpsEngine.h"

#include "Common/CommonDefs.h"
#include "Common/MsgQueue.h"
#include "Common/GPSEvent.h"
#include "Common/GPSLog.h"

static void GpsAdapterThreadEntry(void* arg);
static void GpsDataRMCCB(struct GPS_NMEA_RMC_DATA *data);
static void GpsDataGGACB(struct GPS_NMEA_GGA_DATA *data);
static void GpsDataUTCTimeCB(uint64_t utc);
static void GpsDataSVInfoCB(int num, struct GPS_SV_INFO *infos);
static void GpsDataNMEACB(char *nmea, int length);

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
static GpsUtcTime utcTime = 0;
static MsgQueue *msgQueue = NULL;
static GPSEvent *mainLoopEvent = NULL;

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
    .time_func = GpsDataUTCTimeCB,
    .svinfo_func = GpsDataSVInfoCB,
    .nmea_func = GpsDataNMEACB
};

struct AdapterThreadCtx {
    int running;
};

static struct AdapterThreadCtx context;

class MsgHandlerStop: public MsgHandler {
private:
    struct AdapterThreadCtx *context;
public:

    MsgHandlerStop(struct AdapterThreadCtx *ctx)
    :context(ctx)
    {

    }

    void proc(void)
    {
        GpsStatus status;

        context->running = 0;
        status.size = sizeof(GpsStatus);
        status.status = GPS_STATUS_ENGINE_ON;
        callbacks.status_cb(&status);
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

    memcpy(&callbacks, cb, sizeof(GpsCallbacks));

    msgQueue = MsgQueueCreate();
    mainLoopEvent = GPSEventCreate();
    utcTime = 0;

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

    GpsStatus status;
    status.size = sizeof(GpsStatus);
    status.status = GPS_STATUS_SESSION_BEGIN;

    callbacks.status_cb(&status);

    while (context.running) {
        MsgHandler *handler = NULL;
        callbacks.acquire_wakelock_cb();
        while ((handler = MsgQueueRecv(msgQueue))) {
            callbacks.release_wakelock_cb();
            handler->proc();
            delete handler;
            callbacks.acquire_wakelock_cb();
        }
        callbacks.release_wakelock_cb();
        GpsEnginePollEvent();
    }
    GPSEventSignal(mainLoopEvent);
}

static int GpsAdapterStop(void)
{
    GPSLOGD("GpsAdapterStop");
    MsgHandlerStop *stop = new (std::nothrow) MsgHandlerStop(&context);
    if (stop) {
        callbacks.acquire_wakelock_cb();
        MsgQueueSend(msgQueue, stop);
        callbacks.release_wakelock_cb();
        return GPSEventWait(mainLoopEvent);
    }
    return -1;
}

static void GpsAdapterCleanup(void)
{
    GPSLOGD("GpsAdapterCleanup");
    GPSEventDestory(mainLoopEvent);
    MsgQueueDestory(msgQueue);
    msgQueue = NULL;
    mainLoopEvent = NULL;
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
        MsgQueueSend(msgQueue, injectLocation);
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

static void GpsDataUTCTimeCB(uint64_t utc)
{
    GPSLOGD("GpsDataUTCTimeCB:time %llu", utc);
    utcTime = (GpsUtcTime)utc;
}

static void GpsDataSVInfoCB(int num, struct GPS_SV_INFO *infos)
{
    int i;
    GnssSvStatus status;
    status.size = sizeof(GnssSvStatus);
    status.num_svs = num;
    GPSLOGD("GpsDataSVInfoCB:num %d", num);
    /* TODO:
        1. fill flags field.
        2. fill constellation field */
    for (i = 0; i < num; i++) {
        status.gnss_sv_list[i].size = sizeof(GnssSvInfo);
        status.gnss_sv_list[i].svid = infos[i].svid;
        status.gnss_sv_list[i].c_n0_dbhz = infos[i].cno;
        status.gnss_sv_list[i].elevation = infos[i].elev;
        status.gnss_sv_list[i].azimuth = infos[i].azim;
    }

    callbacks.gnss_sv_status_cb(&status);
}

static void GpsDataNMEACB(char *nmea, int length)
{
    GPSLOGD("GpsDataNMEACB:%s", nmea);
    callbacks.nmea_cb(utcTime, nmea, length);
}