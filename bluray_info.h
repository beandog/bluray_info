#ifndef BLURAY_INFO_H
#define BLURAY_INFO_H

#define BLURAY_ID_STRLEN 41
#define BLURAY_TITLE_STRLEN 33
#define BLURAY_DISC_NAME_STRLEN 256

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

#endif
