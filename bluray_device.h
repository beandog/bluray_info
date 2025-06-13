#ifndef BLURAY_DEVICE_H
#define BLURAY_DEVICE_H

#if defined(__linux__)
#define DEFAULT_BLURAY_DEVICE "/dev/sr0"
#elif defined(__DragonFly__)
#define DEFAULT_BLURAY_DEVICE "/dev/cd0"
#elif defined(__FreeBSD__)
#define DEFAULT_BLURAY_DEVICE "/dev/cd0"
#elif defined(__NetBSD__)
#define DEFAULT_BLURAY_DEVICE "/dev/cd0d"
#elif defined(__OpenBSD__)
#define DEFAULT_BLURAY_DEVICE "/dev/rcd0c"
#elif defined(__APPLE__) && defined(__MACH__)
#define DEFAULT_BLURAY_DEVICE "/dev/disk1"
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(__MSYS__)
#define DEFAULT_BLURAY_DEVICE "D:\\"
#else
#define DEFAULT_BLURAY_DEVICE "/dev/sr0"
#endif

#endif
