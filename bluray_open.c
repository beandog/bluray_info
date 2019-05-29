#include "bluray_open.h"
#include "bluray_time.h"

/**
 * Get main Blu-ray metadata from disc
 */
int bluray_info_init(struct bluray *bd, struct bluray_info *bluray_info) {

	// Get main disc information
	const BLURAY_DISC_INFO *bd_disc_info = NULL;
	bd_disc_info = bd_get_disc_info(bd);

	// Quit if couldn't open disc
	if(bd_disc_info == NULL)
		return 1;

	// Set Blu-ray disc name
	memset(bluray_info->disc_name, '\0', BLURAY_INFO_DISC_NAME_STRLEN);
	const struct meta_dl *bd_meta = NULL;
	bd_meta = bd_get_meta(bd);
	if(bd_meta != NULL)
		strncpy(bluray_info->disc_name, bd_meta->di_name, BLURAY_INFO_DISC_NAME_STRLEN - 1);

	// Use the UDF volume name as disc title; will only work if input file
	// is an image or disc.
	memset(bluray_info->udf_volume_id, '\0', BLURAY_INFO_UDF_VOLUME_ID_STRLEN);
	if(bd_disc_info->udf_volume_id)
		strncpy(bluray_info->udf_volume_id, bd_disc_info->udf_volume_id, BLURAY_INFO_UDF_VOLUME_ID_STRLEN - 1);

	// Set the disc ID if AACS is present
	memset(bluray_info->disc_id, '\0', BLURAY_INFO_DISC_ID_STRLEN);
	uint32_t ix = 0;
	if(bd_disc_info->libaacs_detected) {
		for(ix = 0; ix < 20; ix++) {
			sprintf(bluray_info->disc_id + 2 * ix, "%02X", bd_disc_info->disc_id[ix]);
		}
	}

	// Titles, Indexes and Playlists
	//
	// libbluray has a "title" which is a really an index it uses to list the
	// playlists based on the type queried. It has stuck as the "title index" for
	// media players (mplayer, mpv).
	//
	// The de facto title index can cause problems if using another application
	// that prefers another index method for accessing the playlists (if such
	// a thing exists). bdpslice (part of libbluray) takes both a title number
	// or a playlist number as an argument, and passing the playlist number
	// is more certain.
	//
	// libbluray indexes titles starting at 0, but for human-readable, bluray_info
	// starts at 1. Playlists start at 0, because they are indexed as such on the
	// filesystem.
	bluray_info->titles = bd_get_titles(bd, TITLES_RELEVANT, 0);
	bluray_info->main_title_ix = 0;

	int bd_main_title = bd_get_main_title(bd);
	if(bd_main_title >= 0)
		bluray_info->main_title_ix = (uint32_t)bd_main_title;

	// These are going to change depending on if you have the JVM installed or not
	bluray_info->first_play_supported = (bd_disc_info->first_play_supported ? true : false);
	bluray_info->top_menu_supported = (bd_disc_info->top_menu_supported ? true : false);
	bluray_info->disc_num_titles = bd_disc_info->num_titles;
	bluray_info->hdmv_titles = bd_disc_info->num_hdmv_titles;
	bluray_info->bdj_titles = bd_disc_info->num_bdj_titles;
	bluray_info->unsupported_titles = bd_disc_info->num_unsupported_titles;
	bluray_info->aacs = (bd_disc_info->aacs_detected ? true : false);
	bluray_info->bdplus = (bd_disc_info->bdplus_detected ? true : false);
	bluray_info->bdj = (bd_disc_info->bdj_detected ? true : false);
	bluray_info->content_exist_3D = (bd_disc_info->content_exist_3D ? true : false);
	memset(bluray_info->provider_data, '\0', BLURAY_INFO_PROVIDER_DATA_STRLEN);
	snprintf(bluray_info->provider_data, BLURAY_INFO_PROVIDER_DATA_STRLEN - 1, "%s", bd_disc_info->provider_data);
	memset(bluray_info->initial_output_mode_preference, '\0', 3);
	strcpy(bluray_info->initial_output_mode_preference, (bd_disc_info->initial_output_mode_preference ? "3D" : "2D"));

	return 0;

}

/**
 * Initialize and populate a bluray_title struct
 */
int bluray_title_init(struct bluray *bd, struct bluray_title *bluray_title, uint32_t title_ix) {

	// Initialize to safe values
	bluray_title->ix = title_ix;
	bluray_title->number = title_ix + 1;
	bluray_title->playlist = 0;
	bluray_title->duration = 0;
	bluray_title->seconds = 0;
	bluray_title->minutes = 0;
	bluray_title->size = 0;
	bluray_title->size_mbs = 0;
	bluray_title->chapters = 0;
	bluray_title->clips = 0;
	bluray_title->angles = 0;
	bluray_title->video_streams = 0;
	bluray_title->audio_streams = 0;
	bluray_title->pg_streams = 0;
	strcpy(bluray_title->length, "00:00:00.00");

	int retval = 0;

	// Quit if couldn't open title
	retval = bd_select_title(bd, title_ix);
	if(retval == 0)
		return 1;

	// Quit if couldn't get title info
	BLURAY_TITLE_INFO *bd_title = NULL;
	uint32_t angle = 0;
	bd_title = bd_get_title_info(bd, title_ix, angle);
	if(bd_title == NULL)
		return 2;

	// Populate data
	bluray_title->playlist = bd_title->playlist;
	bluray_title->duration = bd_title->duration;
	bluray_title->seconds = bluray_duration_seconds(bluray_title->duration);
	bluray_title->minutes = bluray_duration_minutes(bluray_title->duration);
	bluray_duration_length(bluray_title->length, bluray_title->duration);
	bluray_title->size = bd_get_title_size(bd);
	bluray_title->size_mbs = ceil((double)bluray_title->size / 1048576);
	bluray_title->chapters = bd_title->chapter_count;
	bluray_title->clips = bd_title->clip_count;
	bluray_title->angles = bd_title->angle_count;
	if(bluray_title->clips) {
		bluray_title->video_streams = bd_title->clips[0].video_stream_count;
		bluray_title->audio_streams = bd_title->clips[0].audio_stream_count;
		bluray_title->pg_streams = bd_title->clips[0].pg_stream_count;
	}

	bluray_title->clip_info = bd_title->clips;
	bluray_title->title_chapters = bd_title->chapters;

	return 0;

}
