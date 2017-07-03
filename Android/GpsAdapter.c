#include <hardware/gps.h>

#include <pthread.h>

#include "Engine/GpsEngine.h"

static GpsCallbacks callbacks;
static void GpsAdapterThreadEntry(void* arg);

void GpsAdapterInit(GpsCallbacks *cb)
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
	GpsEngineInit();
}

int GpsAdapterStart(void)
{
	callbacks.create_thread_cb("GpsAdapterThreadEntry", GpsAdapterThreadEntry, NULL);
}

static void GpsAdapterThreadEntry(void* arg)
{
	GpsEngineSetup();
	
}