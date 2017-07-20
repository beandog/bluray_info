#ifndef BLURAY_INFO_VIDEO_H
#define BLURAY_INFO_VIDEO_H

#include <string.h>
#include "libbluray/bluray.h"

const char *bluray_video_codec(const uint8_t coding_type);

const char *bluray_video_format(const uint8_t format);

const char *bluray_video_framerate(const uint8_t rate);

const char *bluray_video_aspect_ratio(const uint8_t aspect);

#endif
