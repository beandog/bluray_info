#include "bluray_time.h"

uint64_t bluray_duration_seconds(uint64_t duration) {

	uint64_t seconds = duration / 90000;

	return seconds;

}

uint64_t bluray_duration_minutes(uint64_t duration) {

	uint64_t seconds = duration / 90000;
	uint64_t minutes = seconds / 60;

	return minutes;

}

void bluray_duration_length(char *str, uint64_t duration) {

	uint64_t rem = round( ((double)duration) / 90 );
	uint64_t d_msecs = rem % 1000; rem /= 1000;
	uint64_t d_secs = rem % 60; rem /= 60;
	uint64_t d_mins = rem % 60; rem /= 60;
	uint64_t d_hours = rem % 100;	// Avoid printing more than 2 characters, wraps at 99

	sprintf(str, "%02" PRIu64 ":%02" PRIu64 ":%02" PRIu64 ".%03" PRIu64, d_hours, d_mins, d_secs, d_msecs);

}

uint64_t bluray_chapter_duration(struct bluray *bd, uint32_t title_ix, uint32_t chapter_ix, uint8_t angle_ix) {

	int retval = 0;
	retval = bd_select_title(bd, title_ix);

	if(retval == 0)
		return 0;

	retval = bd_select_angle(bd, angle_ix);

	if(retval == 0)
		return 0;

	struct bd_title_info *bluray_title_info = NULL;
	bluray_title_info = bd_get_title_info(bd, title_ix, angle_ix);

	if(bluray_title_info == NULL)
		return 0;

	uint32_t chapter_number;
	chapter_number = chapter_ix + 1;

	if(chapter_number > bluray_title_info->chapter_count)
		return 0;

	struct bd_chapter *bd_chapter = NULL;
	bd_chapter = &bluray_title_info->chapters[chapter_ix];

	return bd_chapter->duration;

}

void bluray_chapter_length(char *dest_str, struct bluray *bd, uint32_t title_ix, uint32_t chapter_ix, uint8_t angle_ix) {

	uint64_t duration = 0;

	duration = bluray_chapter_duration(bd, title_ix, chapter_ix, angle_ix);

	char duration_str[BLURAY_INFO_TIME_STRLEN];

	bluray_duration_length(duration_str, duration);

	strcpy(dest_str, duration_str);

}
