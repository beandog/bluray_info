#ifndef BLURAY_INFO_AUDIO_H
#define BLURAY_INFO_AUDIO_H

#include <string.h>
#include "libbluray/bluray.h"

const char *bluray_audio_codec(const uint8_t coding_type);

const char *bluray_audio_format(const uint8_t aspect);

const char *bluray_audio_rate(const uint8_t rate);

#endif
