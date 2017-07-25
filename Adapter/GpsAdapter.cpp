#include <hardware/gps.h>

#include <new>

#include <string.h>

#include "Engine/GpsEngine.h"

#include "Common/CommonDefs.h"
#include "Common/MsgQueue.h"
#include "Common/GPSEvent.h"
#include "Common/GPSLog.h"

#include "Adapter/Configuration.h"

static void GpsAdapterThreadEntry(void* arg);
static void GpsDataRMCCB(struct GPS_NMEA_RMC_DATA *data);
static void GpsDataGGACB(struct GPS_NMEA_GGA_DATA *data);
static void GpsDataUTCTimeCB(uint64_t utc);
static void GpsDataSVInfoCB(int num, struct GPS_SV_INFO *infos);
static void GpsDataNMEACB(char *nmea, int length);

static GnssSvFlags ConvertToGnssSvFlags(struct GPS_SV_INFO *info);

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
    GnssSystemInfo sysInfo;
    sysInfo.size = sizeof(GnssSystemInfo);
    sysInfo.year_of_hw = GetConfigYearOfHW();
    callbacks.set_system_info_cb(&sysInfo);
    callbacks.set_capabilities_cb(GetConfigCapabilities());
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
    int ret = -1;
    GPSLOGD("GpsAdapterStop");
    callbacks.acquire_wakelock_cb();
    if (msgQueue) {
        MsgHandlerStop *stop = new (std::nothrow) MsgHandlerStop(&context);
        if (stop) {
            MsgQueueSend(msgQueue, stop);
            ret = 0;
        }
    }
    callbacks.release_wakelock_cb();

    if (!ret) {
        GPSEventWait(mainLoopEvent);
        MsgQueueFlush(msgQueue);
    }

    return ret;
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
    int ret = -1;
    callbacks.acquire_wakelock_cb();
    if (msgQueue) {
        MsgHandlerInjectLocation *injectLocation = new (std::nothrow) MsgHandlerInjectLocation(&locationInfo, latitude, longitude, accuracy);
        if (injectLocation) {
            MsgQueueSend(msgQueue, injectLocation);
            ret = 0;
        }
    }
    callbacks.release_wakelock_cb();
    return ret;
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
    if (!strcmp(GNSS_CONFIGURATION_INTERFACE, name)) {
        return GetConfigurationInterface();
    }

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
    status.num_svs = num > GNSS_MAX_SVS ? GNSS_MAX_SVS:num;
    GPSLOGD("GpsDataSVInfoCB:num %d", num);

    for (i = 0; i < status.num_svs; i++) {
        status.gnss_sv_list[i].size = sizeof(GnssSvInfo);
        status.gnss_sv_list[i].svid = infos[i].svid;
        status.gnss_sv_list[i].c_n0_dbhz = infos[i].cno;
        status.gnss_sv_list[i].elevation = infos[i].elev;
        status.gnss_sv_list[i].azimuth = infos[i].azim;
        status.gnss_sv_list[i].flags = ConvertToGnssSvFlags(infos + i);
        status.gnss_sv_list[i].constellation = GNSS_CONSTELLATION_GPS;
    }

    callbacks.gnss_sv_status_cb(&status);
}

static GnssSvFlags ConvertToGnssSvFlags(struct GPS_SV_INFO *info)
{
    GnssSvFlags flags = GNSS_SV_FLAGS_NONE;
    if (info->flags.bits.svUsed) {
        flags |= GNSS_SV_FLAGS_USED_IN_FIX;
    }
    if (info->flags.bits.orbitAvail) {
        if (info->flags.bits.orbitEph) {
            flags |= GNSS_SV_FLAGS_HAS_EPHEMERIS_DATA;
        }
        if (info->flags.bits.orbitAlm) {
            flags |= GNSS_SV_FLAGS_HAS_ALMANAC_DATA;
        }
    }

    return flags;
}

static void GpsDataNMEACB(char *nmea, int length)
{
    GPSLOGD("GpsDataNMEACB:%s", nmea);
    callbacks.nmea_cb(utcTime, nmea, length);
}
