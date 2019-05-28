#include "bluray_open.h"

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
	memset(bluray_info->bluray_title, '\0', BLURAY_INFO_TITLE_STRLEN);
	if(bd_disc_info->udf_volume_id)
		strncpy(bluray_info->bluray_title, bd_disc_info->udf_volume_id, BLURAY_INFO_TITLE_STRLEN - 1);

	// Set the disc ID if AACS is present
	memset(bluray_info->bluray_id, '\0', BLURAY_INFO_ID_STRLEN);
	uint32_t ix = 0;
	if(bd_disc_info->libaacs_detected) {
		for(ix = 0; ix < 20; ix++) {
			sprintf(bluray_info->bluray_id + 2 * ix, "%02X", bd_disc_info->disc_id[ix]);
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
	snprintf(bluray_info->provider_data, BLURAY_INFO_PROVIDER_DATA_STRLEN, "%s", bd_disc_info->provider_data);
	memset(bluray_info->initial_output_mode_preference, '\0', 3);
	strcpy(bluray_info->initial_output_mode_preference, (bd_disc_info->initial_output_mode_preference ? "3D" : "2D"));

	return 0;

}
