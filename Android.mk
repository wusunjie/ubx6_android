LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

DEVICE_SRC_FILES := \
	Device/GPSComDevice.c \
	Device/GPSDevice.c

COMMON_SRC_FILES := \
	Common/GPSEvent.c

ENGINE_SRC_FILES := \
	Engine/UBXCtrlHandler.c \
	Engine/UBXParser.c

TEST_SRC_FILES := \
	GpsTester.c

LOCAL_MODULE := GPSTester
LOCAL_SRC_FILES := $(DEVICE_SRC_FILES) $(COMMON_SRC_FILES) $(ENGINE_SRC_FILES) $(TEST_SRC_FILES)
LOCAL_CPPFLAGS := -g -Wall -I.
LOCAL_LDLIBS := 

include $(BUILD_EXECUTABLE)