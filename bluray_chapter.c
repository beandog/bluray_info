#include "bluray_chapter.h"

uint64_t bluray_chapter_first_position(struct bluray *bd, const uint32_t title_ix, const uint32_t chapter_ix) {

	int32_t retval = 0;
	retval = bd_select_playlist(bd, title_ix);

	if(retval == 0)
		return 0;

	struct bd_title_info *bluray_title_info = NULL;
	bluray_title_info = bd_get_playlist_info(bd, title_ix, 0);

	if(bluray_title_info == NULL)
		return 0;

	uint32_t chapter_number;
	chapter_number = chapter_ix + 1;

	if(chapter_number > bluray_title_info->chapter_count)
		return 0;

	// libbluray.h has two functions to jump to a chapter and return a seek position.
	// The first one, bd_seek_chapter returns the seek position after jumping to it,
	// while the second one specifically is documented to return its start position.
	// To be safe, jump to the chapter first, although it may not be needed.
	bd_seek_chapter(bd, chapter_ix);

	uint64_t position;
	position = (uint64_t)bd_chapter_pos(bd, chapter_ix);

	return position;

}

uint64_t bluray_chapter_last_position(struct bluray *bd, const uint32_t title_ix, const uint32_t chapter_ix) {

	// Start with the first position. It's not safe to assume that it's
	// already been checked before this.
	uint64_t first_position = 0;
	first_position = bluray_chapter_first_position(bd, title_ix, chapter_ix);

	int32_t retval = 0;
	retval = bd_select_playlist(bd, title_ix);

	if(retval == 0)
		return 0;

	// Selecting other than the first angle is not supported right now
	uint32_t angle = 0;

	struct bd_title_info *bluray_title_info = NULL;
	bluray_title_info = bd_get_playlist_info(bd, title_ix, angle);

	if(bluray_title_info == NULL)
		return 0;

	uint32_t chapter_number;
	chapter_number = chapter_ix + 1;

	if(chapter_number > bluray_title_info->chapter_count)
		return 0;

	uint64_t last_position = 0;

	// If only one chapter, or the final one, return the title size as the last position
	if(bluray_title_info->chapter_count == 1 || chapter_number == bluray_title_info->chapter_count) {
		// Casting this here makes me nervous, even though the highest a position
		last_position = bd_get_title_size(bd);
	}

	// If this not the final chapter, simply calculate the position against the
	// next chapter's position.
	if(chapter_number != bluray_title_info->chapter_count) {
		last_position = bluray_chapter_first_position(bd, title_ix, chapter_ix + 1);
	}

	// This shouldn't happen
	if(last_position < first_position)
		last_position = first_position;

	return last_position;

}

/**
 * In libbluray, number of bytes is the same value as the distance between positions.
 *
 * Each title has a padding of 768 bytes at its front, add it to the first chapter.
 */

uint64_t bluray_chapter_size(struct bluray *bd, const uint32_t title_ix, const uint32_t chapter_ix) {

	uint64_t size;
	uint64_t first_position;
	uint64_t last_position;

	first_position = bluray_chapter_first_position(bd, title_ix, chapter_ix);
	last_position = bluray_chapter_last_position(bd, title_ix, chapter_ix);

	if(last_position < first_position)
		return 0;

	size = last_position - first_position;

	if(chapter_ix == 0)
		size += 768;

	return size;

}
