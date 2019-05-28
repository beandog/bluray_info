#include "bluray_audio.h"

void bluray_audio_lang(char *str, const uint8_t lang[BLURAY_INFO_AUDIO_LANG_STRLEN]) {

	memset(str, '\0', BLURAY_INFO_AUDIO_LANG_STRLEN);
	memcpy(str, lang, BLURAY_INFO_AUDIO_LANG_STRLEN - 1);

}

void bluray_audio_codec(char *str, const uint8_t coding_type) {

	memset(str, '\0', BLURAY_INFO_AUDIO_CODEC_STRLEN);

	switch(coding_type) {

		case BLURAY_STREAM_TYPE_AUDIO_MPEG1:
			strncpy(str, "mpeg1", BLURAY_INFO_AUDIO_CODEC_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_MPEG2:
			strncpy(str, "mpeg2", BLURAY_INFO_AUDIO_CODEC_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_LPCM:
			strncpy(str, "lpcm", BLURAY_INFO_AUDIO_CODEC_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_AC3:
			strncpy(str, "ac3", BLURAY_INFO_AUDIO_CODEC_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_DTS:
			strncpy(str, "dts", BLURAY_INFO_AUDIO_CODEC_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_TRUHD:
			strncpy(str, "truhd", BLURAY_INFO_AUDIO_CODEC_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_AC3PLUS:
		case BLURAY_STREAM_TYPE_AUDIO_AC3PLUS_SECONDARY:
			strncpy(str, "ac3plus", BLURAY_INFO_AUDIO_CODEC_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_DTSHD:
		case BLURAY_STREAM_TYPE_AUDIO_DTSHD_SECONDARY:
			strncpy(str, "dtshd", BLURAY_INFO_AUDIO_CODEC_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_DTSHD_MASTER:
			strncpy(str, "dtshd-ma", BLURAY_INFO_AUDIO_CODEC_STRLEN);
			break;

	}

}

void bluray_audio_codec_name(char *str, const uint8_t coding_type) {

	memset(str, '\0', BLURAY_INFO_AUDIO_CODEC_NAME_STRLEN);

	switch(coding_type) {

		case BLURAY_STREAM_TYPE_AUDIO_MPEG1:
			strncpy(str, "MPEG-1", BLURAY_INFO_AUDIO_CODEC_NAME_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_MPEG2:
			strncpy(str, "MPEG-2", BLURAY_INFO_AUDIO_CODEC_NAME_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_LPCM:
			strncpy(str, "LPCM", BLURAY_INFO_AUDIO_CODEC_NAME_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_AC3:
			strncpy(str, "Dolby Digital", BLURAY_INFO_AUDIO_CODEC_NAME_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_DTS:
			strncpy(str, "DTS", BLURAY_INFO_AUDIO_CODEC_NAME_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_TRUHD:
			strncpy(str, "Dolby TrueHD", BLURAY_INFO_AUDIO_CODEC_NAME_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_AC3PLUS:
		case BLURAY_STREAM_TYPE_AUDIO_AC3PLUS_SECONDARY:
			strncpy(str, "Dolby Digital Plus", BLURAY_INFO_AUDIO_CODEC_NAME_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_DTSHD:
		case BLURAY_STREAM_TYPE_AUDIO_DTSHD_SECONDARY:
			strncpy(str, "DTS-HD", BLURAY_INFO_AUDIO_CODEC_NAME_STRLEN);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_DTSHD_MASTER:
			strncpy(str, "DTS-HD Master", BLURAY_INFO_AUDIO_CODEC_NAME_STRLEN);
			break;

	}

}

bool bluray_audio_secondary_stream(const uint8_t coding_type) {

	if(coding_type == BLURAY_STREAM_TYPE_AUDIO_AC3PLUS_SECONDARY || coding_type == BLURAY_STREAM_TYPE_AUDIO_DTSHD_SECONDARY)
		return true;
	else
		return false;

}

void bluray_audio_format(char *str, const uint8_t format) {

	memset(str, '\0', BLURAY_INFO_AUDIO_FORMAT + 1);

	switch(format) {

		case BLURAY_AUDIO_FORMAT_MONO:
			strncpy(str, "mono", BLURAY_INFO_AUDIO_FORMAT + 1);
			break;

		case BLURAY_AUDIO_FORMAT_STEREO:
			strncpy(str, "stereo", BLURAY_INFO_AUDIO_FORMAT + 1);
			break;

		case BLURAY_AUDIO_FORMAT_MULTI_CHAN:
			strncpy(str, "multi_chan", BLURAY_INFO_AUDIO_FORMAT + 1);
			break;

		case BLURAY_AUDIO_FORMAT_COMBO:
			strncpy(str, "combo", BLURAY_INFO_AUDIO_FORMAT + 1);
			break;

	}

}

void bluray_audio_rate(char *str, const uint8_t rate) {

	memset(str, '\0', BLURAY_INFO_AUDIO_RATE_STRLEN);

	switch(rate) {

		case BLURAY_AUDIO_RATE_48:
			strncpy(str, "48", BLURAY_INFO_AUDIO_RATE_STRLEN);
			break;

		case BLURAY_AUDIO_RATE_96:
			strncpy(str, "96", BLURAY_INFO_AUDIO_RATE_STRLEN);
			break;

		case BLURAY_AUDIO_RATE_192:
			strncpy(str, "192", BLURAY_INFO_AUDIO_RATE_STRLEN);
			break;

		case BLURAY_AUDIO_RATE_192_COMBO:
			strncpy(str, "192_combo", BLURAY_INFO_AUDIO_RATE_STRLEN);
			break;

		case BLURAY_AUDIO_RATE_96_COMBO:
			strncpy(str, "96_combo", BLURAY_INFO_AUDIO_RATE_STRLEN);
			break;

	}

}
