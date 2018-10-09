#include "bluray_audio.h"

void bluray_audio_lang(char *str, const uint8_t lang[4]) {

	memcpy(str, lang, BLURAY_AUDIO_LANG + 1);

}

void bluray_audio_codec(char *str, const uint8_t coding_type) {

	switch(coding_type) {

		case BLURAY_STREAM_TYPE_AUDIO_MPEG1:
			strncpy(str, "mpeg1", BLURAY_AUDIO_CODEC + 1);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_MPEG2:
			strncpy(str, "mpeg2", BLURAY_AUDIO_CODEC + 1);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_LPCM:
			strncpy(str, "lpcm", BLURAY_AUDIO_CODEC + 1);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_AC3:
			strncpy(str, "ac3", BLURAY_AUDIO_CODEC + 1);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_DTS:
			strncpy(str, "dts", BLURAY_AUDIO_CODEC + 1);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_TRUHD:
			strncpy(str, "truhd", BLURAY_AUDIO_CODEC + 1);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_AC3PLUS:
		case BLURAY_STREAM_TYPE_AUDIO_AC3PLUS_SECONDARY:
			strncpy(str, "ac3plus", BLURAY_AUDIO_CODEC + 1);
			break;

		case BLURAY_STREAM_TYPE_AUDIO_DTSHD:
		case BLURAY_STREAM_TYPE_AUDIO_DTSHD_SECONDARY:
		case BLURAY_STREAM_TYPE_AUDIO_DTSHD_MASTER:
			strncpy(str, "dtshd", BLURAY_AUDIO_CODEC + 1);
			break;

		default:
			strncpy(str, "", BLURAY_AUDIO_CODEC + 1);
			break;

	}

}

void bluray_audio_format(char *str, const uint8_t format) {

	switch(format) {

		case BLURAY_AUDIO_FORMAT_MONO:
			strncpy(str, "mono", BLURAY_AUDIO_FORMAT + 1);
			break;

		case BLURAY_AUDIO_FORMAT_STEREO:
			strncpy(str, "stereo", BLURAY_AUDIO_FORMAT + 1);
			break;

		case BLURAY_AUDIO_FORMAT_MULTI_CHAN:
			strncpy(str, "multi_chan", BLURAY_AUDIO_FORMAT + 1);
			break;

		case BLURAY_AUDIO_FORMAT_COMBO:
			strncpy(str, "combo", BLURAY_AUDIO_FORMAT + 1);
			break;

		default:
			strncpy(str, "", BLURAY_AUDIO_FORMAT + 1);
			break;

	}

}

void bluray_audio_rate(char *str, const uint8_t rate) {

	switch(rate) {

		case BLURAY_AUDIO_RATE_48:
			strncpy(str, "48", BLURAY_AUDIO_RATE + 1);
			break;

		case BLURAY_AUDIO_RATE_96:
			strncpy(str, "96", BLURAY_AUDIO_RATE + 1);
			break;

		case BLURAY_AUDIO_RATE_192:
			strncpy(str, "192", BLURAY_AUDIO_RATE + 1);
			break;

		case BLURAY_AUDIO_RATE_192_COMBO:
			strncpy(str, "192_combo", BLURAY_AUDIO_RATE + 1);
			break;

		case BLURAY_AUDIO_RATE_96_COMBO:
			strncpy(str, "96_combo", BLURAY_AUDIO_RATE + 1);
			break;

		default:
			strncpy(str, "", BLURAY_AUDIO_RATE + 1);
			break;

	}

}
