#ifndef BLURAY_INFO_VIDEO_H
#define BLURAY_INFO_VIDEO_H

#include <string.h>
#include "libbluray/bluray.h"

struct bluray_video {
	char codec[6];
	char codec_name[7];
	char format[6];
	double framerate;
	char aspect_ratio[5];
};

void bluray_video_codec(char *str, const uint8_t coding_type);

void bluray_video_codec_name(char *str, const uint8_t coding_type);

void bluray_video_format(char *str, const uint8_t format);

double bluray_video_framerate(const uint8_t rate);

void bluray_video_aspect_ratio(char *str, const uint8_t aspect);

#endif
