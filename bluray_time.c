#include "bluray_time.h"

const char *bluray_duration_length(const uint64_t duration) {

	uint64_t msecs = duration / 90000;
	uint64_t d_hours = msecs / 3600;
	uint64_t d_mins = (msecs % 3600) / 60;
	uint64_t d_secs = msecs % 60;
	uint64_t d_msecs = (duration / 900) % 100;

	char length[13] = {'\0'};

	snprintf(length, 13, "%02lu:%02lu:%02lu.%02lu", d_hours, d_mins, d_secs, d_msecs);

	return strndup(length, 13);

}
