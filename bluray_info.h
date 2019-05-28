#ifndef BLURAY_INFO_H
#define BLURAY_INFO_H

#define BLURAY_ID_STRLEN 41
#define BLURAY_TITLE_STRLEN 33
#define BLURAY_DISC_NAME_STRLEN 256
#define BLURAY_LENGTH_STRLEN 12

struct bluray_info {
	char bluray_id[BLURAY_ID_STRLEN];
	char bluray_title[BLURAY_TITLE_STRLEN];
	char disc_name[BLURAY_DISC_NAME_STRLEN];
	uint32_t titles;
	uint32_t main_title_ix;
	bool first_play_supported;
	bool top_menu_supported;
	uint32_t hdmv_titles;
	uint32_t bdj_titles;
	uint32_t unsupported_titles;
	bool aacs;
	bool bdplus;
	bool bdj;
};

struct bluray_title {
	uint32_t ix;
	uint32_t number;
	uint32_t playlist;
	uint64_t duration;
	uint64_t seconds;
	uint64_t minutes;
	uint64_t size;
	double size_mbs;
	uint32_t chapters;
	uint32_t clips;
	uint8_t angles;
	uint8_t video_streams;
	uint8_t audio_streams;
	uint8_t pg_streams;
	char length[BLURAY_LENGTH_STRLEN];
};

struct bluray_chapter {
	uint64_t duration;
	uint64_t start;
	char start_time[BLURAY_LENGTH_STRLEN];
	char length[BLURAY_LENGTH_STRLEN];
	int64_t range[2];
	int64_t size;
	double size_mbs;
};

#endif
