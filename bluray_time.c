#include "bluray_time.h"

/**
 * As is with all timing here, it's impossible to say for certain what the
 * actual length is down to the millisecond, so I have to come as close to it
 * as possible, or as close to as possible as I believe it is.
 *
 * The code here for getting the number of seconds here is pulled from
 * list_titles (libbluray). I'm calculating the milliseconds on my own from there.
 *
 * Using avprobe on a stream copied by bdsplice (libbluray) or bluray_copy, I'm
 * getting the same amount of milliseconds, so I'm going to call it good.
 *
 * Also note that I'm getting the duration for the chapters and the titles in
 * separate locations, and they *add up* to be the same amount, even though the
 * libbluray apps or libav will disagree with me. It is those cases that make me
 * uncertain that it may be right at all, or there is the added possibilty that
 * mine is more accurate. Either way, it's as good as I can get. :)
 *
 */

void bluray_duration_length(char *str, const uint64_t duration) {

	uint64_t msecs = duration / 90000;
	uint64_t d_hours = msecs / 3600;
	uint64_t d_mins = (msecs % 3600) / 60;
	uint64_t d_secs = msecs % 60;
	uint64_t d_msecs = (duration / 900) % 100;

	snprintf(str, BLURAY_DURATION + 1, "%02lu:%02lu:%02lu.%03lu", d_hours, d_mins, d_secs, d_msecs);

}
