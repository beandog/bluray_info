#include "bluray_audio.h"

const char *bluray_audio_codec(const uint8_t coding_type) {

	switch(coding_type) {
	
		case BLURAY_STREAM_TYPE_AUDIO_MPEG1:
			return "mpeg1";

		case BLURAY_STREAM_TYPE_AUDIO_MPEG2:
			return "mpeg2";

		case BLURAY_STREAM_TYPE_AUDIO_LPCM:
			return "lpcm";

		case BLURAY_STREAM_TYPE_AUDIO_AC3:
			return "ac3";

		case BLURAY_STREAM_TYPE_AUDIO_DTS:
			return "dts";

		case BLURAY_STREAM_TYPE_AUDIO_TRUHD:
			return "truhd";

		case BLURAY_STREAM_TYPE_AUDIO_AC3PLUS:
		case BLURAY_STREAM_TYPE_AUDIO_AC3PLUS_SECONDARY:
			return "ac3plus";

		case BLURAY_STREAM_TYPE_AUDIO_DTSHD:
		case BLURAY_STREAM_TYPE_AUDIO_DTSHD_SECONDARY:
		case BLURAY_STREAM_TYPE_AUDIO_DTSHD_MASTER:
			return "dtshd";

	}

	return "";

}

const char *bluray_audio_format(const uint8_t format) {

	switch(format) {

		case BLURAY_AUDIO_FORMAT_MONO:
			return "mono";

		case BLURAY_AUDIO_FORMAT_STEREO:
			return "stereo";

		case BLURAY_AUDIO_FORMAT_MULTI_CHAN:
			return "multi_chan";

		case BLURAY_AUDIO_FORMAT_COMBO:
			return "combo";

	}

	return "";

}

const char *bluray_audio_rate(const uint8_t rate) {

	switch(rate) {

		case BLURAY_AUDIO_RATE_48:
			return "48";

		case BLURAY_AUDIO_RATE_96:
			return "96";

		case BLURAY_AUDIO_RATE_192:
			return "192";

		case BLURAY_AUDIO_RATE_192_COMBO:
			return "192_combo";

		case BLURAY_AUDIO_RATE_96_COMBO:
			return "96_combo";
	
	}

	return "";

}
