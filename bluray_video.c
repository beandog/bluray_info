#include "bluray_video.h"

void bluray_video_codec(char *str, const uint8_t coding_type) {

	switch(coding_type) {

		case BLURAY_STREAM_TYPE_VIDEO_H264:
			strcpy(str, "h264");
			break;

#ifdef BLURAY_STREAM_TYPE_VIDEO_HEVC
		case BLURAY_STREAM_TYPE_VIDEO_HEVC:
			strcpy(str, "h265");
			break;
#endif

		case BLURAY_STREAM_TYPE_VIDEO_MPEG1:
			strcpy(str, "mpeg1");
			break;

		case BLURAY_STREAM_TYPE_VIDEO_MPEG2:
			strcpy(str, "mpeg2");
			break;

		case BLURAY_STREAM_TYPE_VIDEO_VC1:
			strcpy(str, "vc1");
			break;

	}

}

void bluray_video_codec_name(char *str, const uint8_t coding_type) {

	switch(coding_type) {

		case BLURAY_STREAM_TYPE_VIDEO_H264:
			strcpy(str, "AVC");
			break;

#ifdef BLURAY_STREAM_TYPE_VIDEO_HEVC
		case BLURAY_STREAM_TYPE_VIDEO_HEVC:
			strcpy(str, "HEVC");
			break;
#endif

		case BLURAY_STREAM_TYPE_VIDEO_MPEG1:
			strcpy(str, "MPEG-1");
			break;

		case BLURAY_STREAM_TYPE_VIDEO_MPEG2:
			strcpy(str, "MPEG-2");
			break;

		case BLURAY_STREAM_TYPE_VIDEO_VC1:
			strcpy(str, "VC-1");
			break;

	}

}

void bluray_video_format(char *str, const uint8_t format) {

	switch(format) {

		case BLURAY_VIDEO_FORMAT_480I:
			strcpy(str, "480i");
			break;

		case BLURAY_VIDEO_FORMAT_480P:
			strcpy(str, "480p");
			break;

		case BLURAY_VIDEO_FORMAT_576I:
			strcpy(str, "576i");
			break;

		case BLURAY_VIDEO_FORMAT_576P:
			strcpy(str, "576p");
			break;

		case BLURAY_VIDEO_FORMAT_720P:
			strcpy(str, "720p");
			break;

		case BLURAY_VIDEO_FORMAT_1080I:
			strcpy(str, "1080i");
			break;

		case BLURAY_VIDEO_FORMAT_1080P:
			strcpy(str, "1080p");
			break;

#ifdef BLURAY_VIDEO_FORMAT_2160P
		case BLURAY_VIDEO_FORMAT_2160P:
			strcpy(str, "2160p");
			break;
#endif

		default:
			strcpy(str, "");
			break;

	}

}

double bluray_video_framerate(const uint8_t rate) {

	switch(rate) {

		case BLURAY_VIDEO_RATE_24000_1001:
			return 23.97;

		case BLURAY_VIDEO_RATE_24:
			return 24;

		case BLURAY_VIDEO_RATE_25:
			return 25;

		case BLURAY_VIDEO_RATE_30000_1001:
			return 29.97;

		case BLURAY_VIDEO_RATE_50:
			return 50;

		case BLURAY_VIDEO_RATE_60000_1001:
			return 59.94;

		default:
			return 0;

	}

}

void bluray_video_aspect_ratio(char *str, const uint8_t aspect) {

	switch(aspect) {

		case BLURAY_ASPECT_RATIO_4_3:
			strncpy(str, "4:3", BLURAY_VIDEO_ASPECT_RATIO + 1);
			break;

		case BLURAY_ASPECT_RATIO_16_9:
			strncpy(str, "16:9", BLURAY_VIDEO_ASPECT_RATIO + 1);
			break;

		default:
			strncpy(str, "", BLURAY_VIDEO_ASPECT_RATIO + 1);
			break;

	}

}
