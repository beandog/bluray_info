#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <math.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#include <libbluray/bluray.h>
#include "bluray_device.h"
#include "bluray_open.h"
#include "bluray_time.h"
#include "bluray_player.h"
#include <mpv/client.h>

/**
 *   _     _                                  _
 *  | |__ | |_   _ _ __ __ _ _   _      _ __ | | __ _ _   _  ___ _ __
 *  | '_ \| | | | | '__/ _` | | | |    | '_ \| |/ _` | | | |/ _ \ '__|
 *  | |_) | | |_| | | | (_| | |_| |    | |_) | | (_| | |_| |  __/ |
 *  |_.__/|_|\__,_|_|  \__,_|\__, |____| .__/|_|\__,_|\__, |\___|_|
 *                           |___/_____|_|            |___/
 *
 * bluray video player using libbluray and libmpv
 *
 * If your Blu-ray is encrypted, you'll need libaacs as well as a modern
 * KEYDB.cfg file to access them.
 *
 * More information at https://dvds.beandog.org/
 *
 * Go watch tt0067992 :)
 *
 */

int main(int argc, char **argv) {

	char device_filename[PATH_MAX] = {'\0'};
	bool opt_playlist_number = false;
	bool opt_main_title = false;
	bool opt_chapter_start = false;
	bool opt_chapter_end = false;
	bool exit_help = false;
	bool opt_video_stream = false;
	bool opt_audio_stream = false;
	bool opt_subtitle_stream = false;
	char stream_id[4];
	unsigned long int arg_number = 0;
	uint32_t arg_playlist_number = 0;
	uint32_t arg_first_chapter = 0;
	uint32_t arg_last_chapter = 0;
	char key_db_filename[PATH_MAX] = {'\0'};
	const char *home_dir = getenv("HOME");
	int retval = 0;

	struct bluray_player bluray_player;
	snprintf(bluray_player.config_dir, BLURAY_PLAYER_CONFIG_DIR_STRLEN, "/.config/bluray_player");
	memset(bluray_player.mpv_config_dir, '\0', sizeof(bluray_player.mpv_config_dir));
	if(home_dir != NULL)
		snprintf(bluray_player.mpv_config_dir, BLURAY_PLAYER_PATH_MAX, "%s%s", home_dir, bluray_player.config_dir);

	struct bluray_playback bluray_playback;
	bluray_playback.title = 0;
	bluray_playback.fullscreen = false;
	bluray_playback.detelecine = false;
	memset(bluray_playback.audio_lang, '\0', BLURAY_PLAYER_LANG_STRLEN);
	memset(bluray_playback.subtitles_lang, '\0', BLURAY_PLAYER_LANG_STRLEN);
	memset(bluray_playback.chapter_start, '\0', BLURAY_PLAYER_CHAPTER_STRLEN);
	memset(bluray_playback.chapter_end, '\0', BLURAY_PLAYER_CHAPTER_STRLEN);

	char *token = NULL;
	int g_opt = 0;
	int g_ix = 0;
	struct option p_long_opts[] = {
		{ "alang", required_argument, NULL, 'a' },
		{ "aid", required_argument, NULL, 'A' },
		{ "chapters", required_argument, NULL, 'c' },
		{ "detelecine", no_argument, NULL, 'd' },
		{ "fullscreen", no_argument, NULL, 'f' },
		{ "help", no_argument, NULL, 'h' },
		{ "keydb", required_argument, NULL, 'k' },
		{ "main", no_argument, NULL, 'm' },
		{ "playlist", required_argument, NULL, 'p' },
		{ "slang", required_argument, NULL, 's' },
		{ "sid", required_argument, NULL, 'S' },
		{ "vid", required_argument, NULL, 'V' },
		{ "version", no_argument, NULL, 'Z' },
		{ 0, 0, 0, 0 }
	};
	while((g_opt = getopt_long(argc, argv, "a:A:c:dfhk:mp:s:S:V:Z", p_long_opts, &g_ix)) != -1) {

		switch(g_opt) {

			case 'a':
				strncpy(bluray_playback.audio_lang, optarg, BLURAY_PLAYER_LANG_STRLEN - 1);
				break;

			case 'A':
				opt_audio_stream = true;
				arg_number = strtoul(optarg, NULL, 10);
				bluray_playback.audio_stream_id = (uint8_t)arg_number;
				break;

			case 'c':

				// passed -c -
				if(strlen(optarg) == 1 && strncmp(optarg, "-", 1) == 0) {
					printf("invalid syntax: - only, erroring out\n");
					return 1;
				}

				// passed -c <chapter number>
				if(strchr(optarg, '-') == NULL) {
					opt_chapter_start = true;
					opt_chapter_end = false;
					arg_number = strtoul(optarg, NULL, 10);
					if(arg_number < 2)
						arg_first_chapter = 1;
					else
						arg_first_chapter = (uint8_t)arg_number;
					break;
				}

				// passed -c -<chapter number>
				if(strncmp(optarg, "-", 1) == 0) {
					opt_chapter_start = false;
					opt_chapter_end = true;
					token = strtok(optarg, "-");
					arg_number = strtoul(token, NULL, 10);
					if(arg_number < 2)
						arg_last_chapter = 1;
					else
						arg_last_chapter = (uint8_t)arg_number;
					break;
				}

				// passed -c <chapter-number>-[|<chapter-number>]
				opt_chapter_start = true;
				token = strtok(optarg, "-");
				arg_number = strtoul(token, NULL, 10);
				if(arg_number < 2)
					arg_first_chapter = 1;
				else
					arg_first_chapter = (uint8_t)arg_number;

				token = strtok(NULL, "-");

				// passed -c <chapter-number>-<chapter-number>
				if(token != NULL) {
					opt_chapter_end = true;
					arg_number = strtoul(token, NULL, 10);
					if(arg_number < 2)
						arg_last_chapter = 1;
					else
						arg_last_chapter = (uint8_t)arg_number;
					break;
				}

				// at this point, only passed -c <chapter-number>- which is
				// handled the same as -c <chapter-number>
				opt_chapter_end = false;

				break;

			case 'd':
				bluray_playback.detelecine = true;
				break;

			case 'f':
				bluray_playback.fullscreen = true;
				break;

			case 'k':
				memset(key_db_filename, '\0', PATH_MAX);
				strncpy(key_db_filename, optarg, PATH_MAX - 1);
				break;

			case 'm':
				opt_main_title = true;
				break;

			case 'p':
				opt_playlist_number = true;
				arg_number = strtoul(optarg, NULL, 10);
				arg_playlist_number = (uint32_t)arg_number;
				break;

			case 's':
				strncpy(bluray_playback.subtitles_lang, optarg, BLURAY_PLAYER_LANG_STRLEN - 1);
				break;

			case 'S':
				opt_subtitle_stream = true;
				arg_number = strtoul(optarg, NULL, 10);
				bluray_playback.subtitle_stream_id = (uint8_t)arg_number;
				break;

			case 'V':
				opt_video_stream = true;
				arg_number = strtoul(optarg, NULL, 10);
				bluray_playback.video_stream_id = (uint8_t)arg_number;
				break;

			case 'Z':
				printf("bluray_player %s\n", PACKAGE_VERSION);
				exit_help = true;
				break;

			case 'h':
				printf("bluray_player %s - Play back a Blu-ray using libmpv engine\n", PACKAGE_VERSION);
				printf("\n");
				printf("Usage: bluray_player [bluray path] [options]\n");
				printf("\n");
				printf("Title selection:\n");
				printf("  -m, --main               Play main playlist (default)\n");
				printf("  -p, --playlist <#>       Play playlist number\n");
				printf("  -c, --chapters <#>[-#]   Play chapter number(s)\n");
				printf("\n");
				printf("Languages - ISO 639-2 three-letter language codes (eng, deu, fra, spa, ...):\n");
				printf("  -a, --alang <language>   Audio language (default: first stream)\n");
				printf("  -s, --slang <language>   Subtitles language (default: first stream)\n");
				printf("\n");
				printf("Stream IDs:\n");
				printf("  -V, --vid <number>       Play video stream id\n");
				printf("  -A, --aid <number>       Play audio stream id\n");
				printf("  -S, --sid <number>       Play subtitle stream id\n");
				printf("\n");
				printf("Playback:\n");
				printf("  -f, --fullscreen         Display fullscreen\n");
				printf("  -d, --detelecine         Detelecine video\n");
				printf("\n");
				printf("Other:\n");
				printf("  -k, --keydb <filename>   Location to KEYDB.cfg (default: ~/.config/aacs/KEYDB.cfg)\n");
				printf("  -h, --help               This output\n");
				printf("      --version	           Display version\n");
				printf("\n");
				printf("MPV configuration is read from ~/.config/bluray_player/mpv.conf\n");
				printf("\n");
				printf("Blu-ray path can be a device, a filename, or directory (default: %s)\n", DEFAULT_BLURAY_DEVICE);
				exit_help = true;
				break;

			case 0:
			default:
				break;

		}

	}

	if(exit_help)
		return 0;

	if(!opt_playlist_number)
		opt_main_title = true;

	if(opt_main_title && opt_playlist_number) {
		fprintf(stderr, "Select only one option of a playlist or default playlist\n");
		return 1;
	}

	memset(device_filename, '\0', PATH_MAX);
	if (argv[optind])
		strncpy(device_filename, argv[optind], PATH_MAX - 1);
	else
		strncpy(device_filename, DEFAULT_BLURAY_DEVICE, PATH_MAX - 1);

	// Open device
	BLURAY *bd = NULL;
	if(strlen(key_db_filename))
		bd = bd_open(device_filename, key_db_filename);
	else
		bd = bd_open(device_filename, NULL);

	if(bd == NULL) {
		if(strlen(key_db_filename))
			fprintf(stderr, "Could not open Blu-ray %s and KEYDB file %s\n", device_filename, key_db_filename);
		else
			fprintf(stderr, "Could not open Blu-ray %s\n", device_filename);
		return 1;
	}

	// Fetch info
	const BLURAY_DISC_INFO *bd_info = NULL;

	bd_info = bd_get_disc_info(bd);

	if(bd_info == NULL) {
		fprintf(stderr, "Could not get Blu-ray disc info\n");
		bd_close(bd);
		bd = NULL;
		return 1;
	}

	// Blu-ray
	struct bluray_info bluray_info;
	// MPV does not list duplicate titles, so ignore them
	retval = bluray_info_init(bd, &bluray_info, false);

	struct bluray_title bluray_title;

	// Select title passed as an argument
	if(opt_playlist_number) {
		retval = bd_select_playlist(bd, arg_playlist_number);
		if(retval == 0) {
			fprintf(stderr, "Could not open playlist %" PRIu32 "\n", arg_playlist_number);
			bd_close(bd);
			bd = NULL;
			return 1;
		}
		bluray_title.ix = bd_get_current_title(bd);
		bluray_title.playlist = arg_playlist_number;
	} else {
		bluray_title.ix = bluray_info.main_title;
		retval = bd_select_title(bd, bluray_title.ix);
		if(retval == 0) {
			fprintf(stderr, "Could not open main playlist");
			bd_close(bd);
			bd = NULL;
			return 1;
		}
	}

	// Init bluray_title struct
	uint8_t angle_ix = 0;
	retval = bluray_title_init(bd, &bluray_title, bluray_title.ix, angle_ix, false);

	if(retval) {
		fprintf(stderr, "Could not open playlist %" PRIu32 ", quitting\n", bluray_title.playlist);
		return 1;
	}

	// Silently check and fix chapter boundaries for playback
	if(arg_last_chapter > 0 && arg_last_chapter > bluray_title.chapters)
		arg_last_chapter = bluray_title.chapters;
	if(arg_last_chapter > 0 && arg_first_chapter > arg_last_chapter)
		arg_first_chapter = arg_last_chapter;

	// When choosing a chapter range, mpv will add 1 to the last one requested
	// For example, to play just chapter 3, send playback of 3-4
	memset(bluray_playback.chapter_start, '\0', BLURAY_PLAYER_CHAPTER_STRLEN);
	snprintf(bluray_playback.chapter_start, BLURAY_PLAYER_CHAPTER_STRLEN, "#%" PRIu32, arg_first_chapter);
	memset(bluray_playback.chapter_end, '\0', BLURAY_PLAYER_CHAPTER_STRLEN);
	snprintf(bluray_playback.chapter_end, BLURAY_PLAYER_CHAPTER_STRLEN, "#%" PRIu32, arg_last_chapter + 1);

	// MPV zero-indexes title numbers
	bluray_playback.title = bluray_title.ix;

	if(strlen(bluray_info.disc_name))
		printf("[bluray_player] %s\n", bluray_info.disc_name);

	// libmpv doesn't support Blu-ray angle selection (as of latest stable, 0.29.1)
	printf("[bluray_player] Playlist: %*" PRIu32 ", Length: %s, Chapters: %*" PRIu32 ", Video streams: %*" PRIu8 ", Audio streams: %*" PRIu8 ", Subtitles: %*" PRIu8 ", Angles: %*" PRIu8 ", Filesize: %*" PRIu64 " MBs\n", 5, bluray_title.playlist, bluray_title.length, 2, bluray_title.chapters, 2, bluray_title.video_streams, 2, bluray_title.audio_streams, 2, bluray_title.pg_streams, 2, bluray_title.angles, 5, bluray_title.size_mbs);

	// Finished with libbluray
	bd_close(bd);
	bd = NULL;

	// Note that the order and location of setting mpv configuration is important,
	// especially if you want to override mpv.conf in ~/.config/bluray_player/

	// Blu-ray playback using libmpv
	mpv_handle *bluray_mpv = NULL;
	bluray_mpv = mpv_create();
	if(bluray_mpv == NULL) {
		fprintf(stderr, "[bluray_player] Could not create an MPV instance, quitting\n");
		return 1;
	}

	// Load user's mpv configuration in ~/.config/bluray_player/mpv.conf (and friends)
	if(strlen(bluray_player.mpv_config_dir) > 0) {
		mpv_set_option_string(bluray_mpv, "config-dir", bluray_player.mpv_config_dir);
		mpv_set_option_string(bluray_mpv, "config", "yes");
	}

	// MPV doesn't need much before initializing, and config-dir is the only relevant one here
	retval = mpv_initialize(bluray_mpv);
	if(retval) {
		fprintf(stderr, "[bluray_player] Could not initalize MPV, quiting\n");
		return 1;
	}

	// Playback options and default configuration
	mpv_set_option_string(bluray_mpv, "bluray-device", device_filename);
	mpv_set_option_string(bluray_mpv, "terminal", "yes");
	mpv_set_option_string(bluray_mpv, "term-osd-bar", "yes");
	mpv_set_option_string(bluray_mpv, "input-default-bindings", "yes");
	mpv_set_option_string(bluray_mpv, "input-vo-keyboard", "yes");
	if(bluray_playback.fullscreen)
		mpv_set_option_string(bluray_mpv, "fullscreen", "yes");
	if(bluray_playback.detelecine)
		mpv_set_option_string(bluray_mpv, "vf", "pullup,dejudder");
	if(opt_chapter_start && arg_first_chapter > 0)
		mpv_set_option_string(bluray_mpv, "start", bluray_playback.chapter_start);
	if(opt_chapter_end && arg_last_chapter > 0)
		mpv_set_option_string(bluray_mpv, "end", bluray_playback.chapter_end);

	// mpv zero-indexes titles
	char bluray_mpv_args[BLURAY_PLAYER_TITLE_STRLEN];
	memset(bluray_mpv_args, '\0', BLURAY_PLAYER_TITLE_STRLEN);
	snprintf(bluray_mpv_args, BLURAY_PLAYER_TITLE_STRLEN, "bd://%" PRIu32, bluray_playback.title);

	// MPV uses zero-indexing for titles, bluray_info uses one instead
	const char *bluray_mpv_commands[] = {
		"loadfile",
		bluray_mpv_args,
		NULL
	};

	retval = mpv_command(bluray_mpv, bluray_mpv_commands);
	if(retval) {
		fprintf(stderr, "[bluray_player] Could not send MPV arguments: %s; using defaults\n", bluray_mpv_args);
	}

	// Based on the decoder used in libmpv, it may or may not find other streams.
	// bluray_player intentionally doesn't check for out of bounds or non-existing
	// audio and subtitle languages and tracks, and lets the player handle user input.

	// Any arguments after this part will override mpv.conf

	// Set playback languages in order of language code, then stream IDs
	// When selecting a lanugage with --alang or --slang, it will choose the first of
	// any streams with that language. Setting --aid, or --sid will choose the specific one.
	if(strlen(bluray_playback.audio_lang) > 0)
		retval = mpv_set_option_string(bluray_mpv, "alang", bluray_playback.audio_lang);
	if(strlen(bluray_playback.subtitles_lang) > 0)
		mpv_set_option_string(bluray_mpv, "slang", bluray_playback.subtitles_lang);

	// I vaguely recall seeing a Blu-ray with two video streams before
	if(opt_video_stream) {
		memset(stream_id, '\0', 4);
		snprintf(stream_id, 4, "%" PRIu8, bluray_playback.video_stream_id);
		retval = mpv_set_option_string(bluray_mpv, "vid", stream_id);
	}
	if(opt_audio_stream) {
		memset(stream_id, '\0', 4);
		snprintf(stream_id, 4, "%" PRIu8, bluray_playback.audio_stream_id);
		retval = mpv_set_option_string(bluray_mpv, "aid", stream_id);
	}
	if(opt_subtitle_stream) {
		memset(stream_id, '\0', 4);
		snprintf(stream_id, 4, "%" PRIu8, bluray_playback.subtitle_stream_id);
		retval = mpv_set_option_string(bluray_mpv, "sid", stream_id);
	}

	mpv_event *bluray_mpv_event = NULL;
	struct mpv_event_log_message *bluray_mpv_log_message = NULL;

	while(true) {

		bluray_mpv_event = mpv_wait_event(bluray_mpv, -1);

		if(bluray_mpv_event->event_id == MPV_EVENT_SHUTDOWN || bluray_mpv_event->event_id == MPV_EVENT_END_FILE)
			break;

		if(bluray_mpv_event->event_id == MPV_EVENT_LOG_MESSAGE) {
			bluray_mpv_log_message = (struct mpv_event_log_message *)bluray_mpv_event->data;
			fprintf(stderr, "[bluray_player] mpv [%s]: %s", bluray_mpv_log_message->level, bluray_mpv_log_message->text);
		}

	}

	mpv_terminate_destroy(bluray_mpv);

	return 0;

}
