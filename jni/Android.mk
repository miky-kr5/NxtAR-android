LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OPENCV_CAMERA_MODULES:=off
OPENCV_LIB_TYPE:=STATIC #SHARED
include C:\Users\miguel.astor\Documents\OpenCV-2.4.8-android-sdk\sdk\native\jni\OpenCV.mk
#include C:\NVPACK\OpenCV-2.4.5-Tegra-sdk-r2\sdk\native\jni\OpenCV-tegra3.mk

LOCAL_MODULE    := cvproc
LOCAL_SRC_FILES := cv_proc.cpp marker.cpp
LOCAL_LDLIBS +=  -llog -ldl

include $(BUILD_SHARED_LIBRARY)

# Add prebuilt libraries
include $(CLEAR_VARS)

LOCAL_MODULE := gdx
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libgdx.so

include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := gdx-freetype
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libgdx-freetype.so

include $(PREBUILT_SHARED_LIBRARY)