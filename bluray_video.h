#ifndef BLURAY_INFO_VIDEO_H
#define BLURAY_INFO_VIDEO_H
#define BLURAY_VIDEO_ASPECT_RATIO 4

#include <string.h>
#include "libbluray/bluray.h"

void bluray_video_codec(char *str, const uint8_t coding_type);

void bluray_video_codec_name(char *str, const uint8_t coding_type);

void bluray_video_format(char *str, const uint8_t format);

double bluray_video_framerate(const uint8_t rate);

void bluray_video_aspect_ratio(char *str, const uint8_t aspect);

#endif
