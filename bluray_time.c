#include "bluray_time.h"

uint64_t bluray_duration_seconds(const uint64_t duration) {

	uint64_t seconds = duration / 90000;

	return seconds;

}

uint64_t bluray_duration_minutes(const uint64_t duration) {

	uint64_t seconds = duration / 90000;
	uint64_t minutes = seconds / 60;

	return minutes;

}

void bluray_duration_length(char *str, const uint64_t duration) {

	uint64_t msecs = duration / 90000;
	uint64_t d_hours = msecs / 3600;
	uint64_t d_mins = (msecs % 3600) / 60;
	uint64_t d_secs = msecs % 60;
	uint64_t d_msecs = (duration / 900) % 100;

	snprintf(str, BLURAY_DURATION + 1, "%02lu:%02lu:%02lu.%02lu", d_hours, d_mins, d_secs, d_msecs);

}
