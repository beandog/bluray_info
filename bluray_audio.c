#include "bluray_audio.h"

void bluray_audio_lang(char *str, uint8_t lang[BLURAY_INFO_AUDIO_LANG_STRLEN]) {

	memset(str, '\0', BLURAY_INFO_AUDIO_LANG_STRLEN);
	memcpy(str, lang, BLURAY_INFO_AUDIO_LANG_STRLEN - 1);

}

void bluray_audio_codec(char *str, uint8_t coding_type) {

	memset(str, '\0', BLURAY_INFO_AUDIO_CODEC_STRLEN);

	switch(coding_type) {

		case BLURAY_STREAM_TYPE_AUDIO_MPEG1:
			strcpy(str, "mpeg1");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_MPEG2:
			strcpy(str, "mpeg2");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_LPCM:
			strcpy(str, "lpcm");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_AC3:
			strcpy(str, "ac3");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_DTS:
			strcpy(str, "dts");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_TRUHD:
			strcpy(str, "truhd");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_AC3PLUS:
		case BLURAY_STREAM_TYPE_AUDIO_AC3PLUS_SECONDARY:
			strcpy(str, "ac3plus");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_DTSHD:
		case BLURAY_STREAM_TYPE_AUDIO_DTSHD_SECONDARY:
			strcpy(str, "dtshd");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_DTSHD_MASTER:
			strcpy(str, "dtshd-ma");
			break;

	}

}

void bluray_audio_codec_name(char *str, uint8_t coding_type) {

	memset(str, '\0', BLURAY_INFO_AUDIO_CODEC_NAME_STRLEN);

	switch(coding_type) {

		case BLURAY_STREAM_TYPE_AUDIO_MPEG1:
			strcpy(str, "MPEG-1");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_MPEG2:
			strcpy(str, "MPEG-2");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_LPCM:
			strcpy(str, "LPCM");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_AC3:
			strcpy(str, "Dolby Digital");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_DTS:
			strcpy(str, "DTS");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_TRUHD:
			strcpy(str, "Dolby TrueHD");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_AC3PLUS:
		case BLURAY_STREAM_TYPE_AUDIO_AC3PLUS_SECONDARY:
			strcpy(str, "Dolby Digital Plus");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_DTSHD:
		case BLURAY_STREAM_TYPE_AUDIO_DTSHD_SECONDARY:
			strcpy(str, "DTS-HD");
			break;

		case BLURAY_STREAM_TYPE_AUDIO_DTSHD_MASTER:
			strcpy(str, "DTS-HD Master");
			break;

	}

}

bool bluray_audio_secondary_stream(uint8_t coding_type) {

	if(coding_type == BLURAY_STREAM_TYPE_AUDIO_AC3PLUS_SECONDARY || coding_type == BLURAY_STREAM_TYPE_AUDIO_DTSHD_SECONDARY)
		return true;
	else
		return false;

}

void bluray_audio_format(char *str, uint8_t format) {

	memset(str, '\0', BLURAY_INFO_AUDIO_FORMAT_STRLEN);

	switch(format) {

		case BLURAY_AUDIO_FORMAT_MONO:
			strcpy(str, "mono");
			break;

		case BLURAY_AUDIO_FORMAT_STEREO:
			strcpy(str, "stereo");
			break;

		case BLURAY_AUDIO_FORMAT_MULTI_CHAN:
			strcpy(str, "multi_chan");
			break;

		case BLURAY_AUDIO_FORMAT_COMBO:
			strcpy(str, "combo");
			break;

	}

}

void bluray_audio_rate(char *str, uint8_t rate) {

	memset(str, '\0', BLURAY_INFO_AUDIO_RATE_STRLEN);

	switch(rate) {

		case BLURAY_AUDIO_RATE_48:
			strcpy(str, "48");
			break;

		case BLURAY_AUDIO_RATE_96:
			strcpy(str, "96");
			break;

		case BLURAY_AUDIO_RATE_192:
			strcpy(str, "192");
			break;

		case BLURAY_AUDIO_RATE_192_COMBO:
			strcpy(str, "192_combo");
			break;

		case BLURAY_AUDIO_RATE_96_COMBO:
			strcpy(str, "96_combo");
			break;

	}

}
