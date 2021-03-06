LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

DEVICE_SRC_FILES := \
    Device/GPSComDevice.c \
    Device/GPSDevice.c

COMMON_SRC_FILES := \
    Common/GPSSafeIO.c \
    Common/GPSEvent.c \
    Common/MsgQueue.cpp

ENGINE_SRC_FILES := \
    Engine/UBXCtrlHandler.c \
    Engine/UBXParser.c \
    Engine/GpsEngine.c \
    Engine/minmea.c

ADAPTER_SRC_FILE := \
    Adapter/GpsAdapter.cpp \
    Adapter/GpsModule.c \
    Adapter/Configuration.c

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := gps.default
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/hw
LOCAL_SRC_FILES := $(DEVICE_SRC_FILES) $(COMMON_SRC_FILES) $(ENGINE_SRC_FILES) $(ADAPTER_SRC_FILE)
LOCAL_CFLAGS := -DMERBOK_GPS_DLL -DMERBOK_GPS_DLL_EXPORTS -g
LOCAL_CPPFLAGS := -DMERBOK_GPS_DLL -DMERBOK_GPS_DLL_EXPORTS -g -fno-rtti -fno-exceptions

LOCAL_C_INCLUDES := . system/core/include
LOCAL_SHARED_LIBRARIES := libcutils

LOCAL_LDLIBS := 

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := gps.conf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)
LOCAL_SRC_FILES := Config/gps.conf

include $(BUILD_PREBUILT)
