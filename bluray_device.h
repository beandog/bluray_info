#ifndef BLURAY_DEVICE_H
#define BLURAY_DEVICE_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <mntent.h>

#if defined (__linux__)
#define DEFAULT_BLURAY_DEVICE "/dev/sr0"
#elif defined (__DragonFly__)
#define DEFAULT_BLURAY_DEVICE "/dev/cd0"
#elif defined (__FreeBSD__)
#define DEFAULT_BLURAY_DEVICE "/dev/cd0"
#elif defined (__NetBSD__)
#define DEFAULT_BLURAY_DEVICE "/dev/cd0d"
#elif defined (__OpenBSD__)
#define DEFAULT_BLURAY_DEVICE "/dev/rcd0c"
#elif defined (__APPLE__) && defined (__MACH__)
#define DEFAULT_BLURAY_DEVICE "/dev/disk1"
#else
#define DEFAULT_BLURAY_DEVICE "/dev/sr0"
#endif

bool bluray_optical_drive(const char *device_filename);

#endif
