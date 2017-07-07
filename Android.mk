LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

DEVICE_SRC_FILES := \
    Device/GPSComDevice.c \
    Device/GPSDevice.c

COMMON_SRC_FILES := \
    Common/GPSEvent.c

ENGINE_SRC_FILES := \
    Engine/UBXCtrlHandler.c \
    Engine/UBXParser.c \
    Engine/GpsEngine.c \
    Engine/minmea.c

ADAPTER_SRC_FILE := \
    Adapter/GpsAdapter.c \
    Adapter/GpsModule.c


LOCAL_MODULE := gps.default
LOCAL_MODULE_PATH := $(TARGET_OUT)/system/lib/hw
LOCAL_SRC_FILES := $(DEVICE_SRC_FILES) $(COMMON_SRC_FILES) $(ENGINE_SRC_FILES) $(ADAPTER_SRC_FILE)
LOCAL_CFLAGS := -DMERBOK_GPS_DLL -DMERBOK_GPS_DLL_EXPORTS -g -I.
LOCAL_LDLIBS := 

include $(BUILD_SHARED_LIBRARY)
