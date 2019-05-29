#ifndef BLURAY_PLAYER_H
#define BLURAY_PLAYER_H

#include <limits.h>
#if defined (__linux__)
#include <linux/limits.h>
#endif

#define BLURAY_PLAYER_CONFIG_DIR_STRLEN 23
#define BLURAY_PLAYER_PATH_MAX ( PATH_MAX - 1 )
#define BLURAY_PLAYER_LANG_STRLEN 4
#define BLURAY_PLAYER_CHAPTER_STRLEN 5

struct bluray_player {
	char config_dir[BLURAY_PLAYER_CONFIG_DIR_STRLEN];
	char mpv_config_dir[BLURAY_PLAYER_PATH_MAX];
};

struct bluray_playback {
	uint32_t title;
	bool fullscreen;
	bool deinterlace;
	char audio_lang[BLURAY_PLAYER_LANG_STRLEN];
	char subtitles_lang[BLURAY_PLAYER_LANG_STRLEN];
	char chapter_start[BLURAY_PLAYER_CHAPTER_STRLEN];
	char chapter_end[BLURAY_PLAYER_CHAPTER_STRLEN];
	uint8_t video_stream_id;
	uint8_t audio_stream_id;
	uint8_t subtitle_stream_id;
};

#endif
