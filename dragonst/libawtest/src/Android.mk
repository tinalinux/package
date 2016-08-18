# Copyright 2013 The Android Open Source Project
# awstcrl.cpp main.cpp tinystr.cpp tinyxml.cpp tinyxmlerror.cpp tinyxmlparser.cpp

LOCAL_PATH:= $(call my-dir)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	AwsDevPlugin.cpp \
	awserdev.cpp \
	awsfostream.cpp \
	awshelper.cpp \
	awsostream.cpp \
	GeneralFuncs.cpp \
	GenSerialPortStream.cpp \
	tinystr.cpp \
	tinyxml.cpp \
	tinyxmlerror.cpp \
	tinyxmlparser.cpp \
	main.cpp

LOCAL_MODULE := AwsTest
LOCAL_SHARED_LIBRARIES := libstlport libstdc++ libcutils
LOCAL_C_INCLUDES := $(TOP)/frameworks/base/include

LOCAL_CFLAGS := -Wall -Werror
include $(BUILD_EXECUTABLE)