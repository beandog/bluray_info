#include "bluray_video.h"

const char *bluray_video_codec(const uint8_t coding_type) {

	switch(coding_type) {

		case BLURAY_STREAM_TYPE_VIDEO_H264:
			return "h264";

		case BLURAY_STREAM_TYPE_VIDEO_MPEG1:
			return "mpeg1";

		case BLURAY_STREAM_TYPE_VIDEO_MPEG2:
			return "mpeg2";

		case BLURAY_STREAM_TYPE_VIDEO_VC1:
			return "vc1";

	}

	return "";

}

const char *bluray_video_format(const uint8_t format) {

	switch(format) {

		case BLURAY_VIDEO_FORMAT_480I:
			return "480i";

		case BLURAY_VIDEO_FORMAT_480P:
			return "480p";

		case BLURAY_VIDEO_FORMAT_576I:
			return "576i";

		case BLURAY_VIDEO_FORMAT_576P:
			return "576p";

		case BLURAY_VIDEO_FORMAT_720P:
			return "720p";

		case BLURAY_VIDEO_FORMAT_1080I:
			return "1080i";

		case BLURAY_VIDEO_FORMAT_1080P:
			return "1080p";
	
	}

	return "";

}

const char *bluray_video_framerate(const uint8_t rate) {

	switch(rate) {

		case BLURAY_VIDEO_RATE_24000_1001:
			return "23.97";

		case BLURAY_VIDEO_RATE_24:
			return "24";

		case BLURAY_VIDEO_RATE_25:
			return "25";
	
		case BLURAY_VIDEO_RATE_30000_1001:
			return "29.97";

		case BLURAY_VIDEO_RATE_50:
			return "50";

		case BLURAY_VIDEO_RATE_60000_1001:
			return "59.94";

	}

	return "";

}

const char *bluray_video_aspect_ratio(const uint8_t aspect) {

	switch(aspect) {

		case BLURAY_ASPECT_RATIO_4_3:
			return "4:3";

		case BLURAY_ASPECT_RATIO_16_9:
			return "16:9";


	}

	return "";

}
