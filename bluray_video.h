#ifndef BLURAY_INFO_VIDEO_H
#define BLURAY_INFO_VIDEO_H

#include <string.h>
#include "libbluray/bluray.h"

#define BLURAY_INFO_VIDEO_CODEC_STRLEN 6
#define BLURAY_INFO_VIDEO_CODEC_NAME_STRLEN 7
#define BLURAY_INFO_VIDEO_FORMAT_STRLEN 7
#define BLURAY_INFO_VIDEO_ASPECT_RATIO_STRLEN 7

struct bluray_video {
	char codec[BLURAY_INFO_VIDEO_CODEC_STRLEN];
	char codec_name[BLURAY_INFO_VIDEO_CODEC_NAME_STRLEN];
	char format[BLURAY_INFO_VIDEO_FORMAT_STRLEN];
	double framerate;
	char aspect_ratio[BLURAY_INFO_VIDEO_ASPECT_RATIO_STRLEN];
};

void bluray_video_codec(char *str, const uint8_t coding_type);

void bluray_video_codec_name(char *str, const uint8_t coding_type);

void bluray_video_format(char *str, const uint8_t format);

double bluray_video_framerate(const uint8_t rate);

void bluray_video_aspect_ratio(char *str, const uint8_t aspect);

#endif
