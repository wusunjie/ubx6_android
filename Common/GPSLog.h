#ifndef _GPS_LOG_H
#define _GPS_LOG_H

#include "cutils/log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "hgps"

#define GPSLOGE(...) ALOGE(__VA_ARGS__)
#define GPSLOGD(...) ALOGD(__VA_ARGS__)
#define GPSASSERT(cond, ...) ALOG_ASSERT(cond, ## __VA_ARGS__)
#define GPSAOALOGD(...) ALOGD(__VA_ARGS__)

#endif