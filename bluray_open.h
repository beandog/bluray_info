#ifndef BLURAY_INFO_OPEN_H
#define BLURAY_INFO_OPEN_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "config.h"
#include "libbluray/bluray.h"
#include "libbluray/meta_data.h"

#define BLURAY_INFO_DISC_ID_STRLEN 41
#define BLURAY_INFO_UDF_VOLUME_ID_STRLEN 33
#define BLURAY_INFO_PROVIDER_DATA_STRLEN 33
#define BLURAY_INFO_DISC_NAME_STRLEN 256

// Time format: 00:00:00.000
#define BLURAY_INFO_TIME_STRLEN 13

struct bluray_info {
	char disc_name[BLURAY_INFO_DISC_NAME_STRLEN];
	char udf_volume_id[BLURAY_INFO_UDF_VOLUME_ID_STRLEN];
	char disc_id[BLURAY_INFO_DISC_ID_STRLEN];
	uint32_t titles;
	uint32_t main_title;
	bool first_play_supported;
	bool top_menu_supported;
	uint32_t disc_num_titles;
	uint32_t hdmv_titles;
	uint32_t bdj_titles;
	uint32_t unsupported_titles;
	bool aacs;
	bool bdplus;
	bool bdj;
	bool content_exist_3D;
	char provider_data[BLURAY_INFO_PROVIDER_DATA_STRLEN];
	char initial_output_mode_preference[3];
};

struct bluray_title {
	uint32_t ix;
	uint32_t number;
	uint32_t playlist;
	uint64_t duration;
	uint64_t seconds;
	uint64_t minutes;
	uint64_t size;
	uint64_t size_mbs;
	uint32_t chapters;
	uint32_t clips;
	uint8_t angles;
	uint8_t video_streams;
	uint8_t audio_streams;
	uint8_t pg_streams;
	char length[BLURAY_INFO_TIME_STRLEN];
	BLURAY_CLIP_INFO *clip_info;
	BLURAY_TITLE_CHAPTER *title_chapters;
};

struct bluray_chapter {
	uint64_t duration;
	uint64_t start;
	char start_time[BLURAY_INFO_TIME_STRLEN];
	char length[BLURAY_INFO_TIME_STRLEN];
	int64_t range[2];
	uint64_t size;
	uint64_t size_mbs;
};

int bluray_info_init(struct bluray *bd, struct bluray_info *bluray_info, bool display_duplicates);

int bluray_title_init(struct bluray *bd, struct bluray_title *bluray_title, uint32_t title_ix, uint8_t angle_ix);

#endif
