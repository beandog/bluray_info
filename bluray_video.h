#ifndef BLURAY_INFO_VIDEO_H
#define BLURAY_INFO_VIDEO_H

#include <string.h>
#include "libbluray/bluray.h"
#include "libbluray/bluray-version.h"

#define BLURAY_UHD_MIN_VER 10002	// HEVC and 2160p support was added in libbluray 1.0.2

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

void bluray_video_codec(char *str, uint8_t coding_type);

void bluray_video_codec_name(char *str, uint8_t coding_type);

void bluray_video_format(char *str, uint8_t format);

double bluray_video_framerate(uint8_t rate);

void bluray_video_aspect_ratio(char *str, uint8_t aspect);

#endif
