#ifndef BLURAY_INFO_H
#define BLURAY_INFO_H

struct bluray_info {
	char bluray_id[41];
	char bluray_title[33];
	uint32_t hdmv_titles;
	uint32_t bdj_titles;
	uint32_t unsupported_titles;
	uint32_t titles;
	uint32_t relevant_titles;
	uint32_t longest_title;
	uint32_t main_title;
};

struct bluray_info bluray_get_info(const BLURAY_DISC_INFO *bd_info);

#endif
