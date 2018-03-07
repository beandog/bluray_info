#ifndef BLURAY_INFO_AUDIO_H
#define BLURAY_INFO_AUDIO_H
#define BLURAY_AUDIO_CODEC 8
#define BLURAY_AUDIO_FORMAT 10
#define BLURAY_AUDIO_RATE 9

#include <string.h>
#include "libbluray/bluray.h"

void bluray_audio_codec(char *str, const uint8_t coding_type);

void bluray_audio_format(char *str, const uint8_t aspect);

void bluray_audio_rate(char *str, const uint8_t rate);

#endif
