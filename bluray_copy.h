#ifndef BLURAY_COPY_H
#define BLURAY_COPY_H

/**
 * For packet size, use the same as libbluray. This makes doing math much
 * simpler when calculating where start and end points of chapters are.
 * Plus, it makes checking for one bad fetch possible, and can skip over bad
 * blocks.
 */
#define BLURAY_COPY_BUFFER_SIZE 192

struct bluray_copy {
	char *filename;
	int fd;
	int64_t size;
	double size_mbs;
};

#endif
