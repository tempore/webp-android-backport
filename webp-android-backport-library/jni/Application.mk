#APP_ABI := all
#dont need arch introdiced after android 4.2.1
APP_ABI := armeabi armeabi-v7a x86 mips

APP_CPPFLAGS := -fno-rtti -fno-exceptions
APP_PLATFORM := android-8

ifndef WEBP_BACKPORT_DEBUG_NATIVE
# Force release compilation in release optimizations, even if application is debuggable by manifest
APP_OPTIM := release
endif