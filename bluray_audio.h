#ifndef BLURAY_INFO_AUDIO_H
#define BLURAY_INFO_AUDIO_H
#define BLURAY_INFO_AUDIO_CODEC_STRLEN 9
#define BLURAY_INFO_AUDIO_CODEC_NAME_STRLEN 18
#define BLURAY_INFO_AUDIO_FORMAT_STRLEN 12
#define BLURAY_INFO_AUDIO_RATE_STRLEN 9
#define BLURAY_INFO_AUDIO_LANG_STRLEN 4

#include <stdbool.h>
#include <string.h>
#include "libbluray/bluray.h"

struct bluray_audio {
	char lang[BLURAY_INFO_AUDIO_LANG_STRLEN];
	char codec[BLURAY_INFO_AUDIO_CODEC_STRLEN];
	char codec_name[BLURAY_INFO_AUDIO_CODEC_NAME_STRLEN];
	char format[BLURAY_INFO_AUDIO_FORMAT_STRLEN];
	char rate[BLURAY_INFO_AUDIO_RATE_STRLEN];
};

void bluray_audio_lang(char *str, const uint8_t lang[4]);

void bluray_audio_codec(char *str, const uint8_t coding_type);

void bluray_audio_codec_name(char *str, const uint8_t coding_type);

bool bluray_audio_secondary_stream(const uint8_t coding_type);

void bluray_audio_format(char *str, const uint8_t aspect);

void bluray_audio_rate(char *str, const uint8_t rate);

#endif
