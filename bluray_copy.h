#ifndef BLURAY_COPY_H
#define BLURAY_COPY_H

#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif

/**
 * For packet size, use the same as libbluray. This makes doing math much
 * simpler when calculating where start and end points of chapters are.
 * Plus, it makes checking for one bad fetch possible, and can skip over bad
 * blocks.
 *
 * See libbluray/src/examples/bdsplice.c for their packet sizes
 */
#define BLURAY_COPY_BUFFER_SIZE 192

/**
 * This is for documentation only right now. From the Blu-ray specification,
 * an MPEG-2 Transport Stream is constructed of "aligned units," where one is
 * 6144 bytes, or 32 source packets.
 */
#define BLURAY_M2TS_UNIT_SIZE 6144

struct bluray_copy {
	char filename[PATH_MAX];
	int fd;
	int64_t size;
	double size_mbs;
};

#endif
