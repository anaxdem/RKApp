LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#subdirs := $(LOCAL_PATH)/libusb/Android.mk
#subdirs += $(LOCAL_PATH)/librkapp/Android.mk


LOCAL_MODULE    := librkapp

LOCAL_SRC_FILES := \
	librkapp/librkapp.c

LOCAL_SRC_FILES+= \
	libusb/libusb/core.c \
	libusb/libusb/descriptor.c \
	libusb/libusb/io.c \
	libusb/libusb/sync.c \
	libusb/libusb/os/linux_usbfs.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)/libusb/android \
	$(LOCAL_PATH)/libusb/libusb \
	$(LOCAL_PATH)/libusb/libusb/os 

LOCAL_LDLIBS := -llog 


LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
include $(BUILD_SHARED_LIBRARY)
