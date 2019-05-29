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

	uint32_t d_num_titles = 0;

	bool opt_title_number = false;
	bool opt_playlist_number = false;
	bool opt_main_title = false;
	bool opt_chapter_start = false;
	bool opt_chapter_end = false;
	bool invalid_opt = false;
	long int arg_number = 0;
	uint32_t arg_title_number = 0;
	uint32_t arg_playlist_number = 0;
	uint32_t arg_first_chapter = 0;
	uint32_t arg_last_chapter = 0;
	const char *key_db_filename = NULL;
	const char *home_dir = getenv("HOME");
	int retval = 0;

	struct bluray_player bluray_player;
	snprintf(bluray_player.config_dir, BLURAY_CONFIG_DIR_STRLEN, "/.config/bluray_player");
	memset(bluray_player.mpv_config_dir, '\0', sizeof(bluray_player.mpv_config_dir));
	if(home_dir != NULL)
		snprintf(bluray_player.mpv_config_dir, BLURAY_PLAYER_PATH_MAX, "%s%s", home_dir, bluray_player.config_dir);

	struct bluray_playback bluray_playback;
	bluray_playback.title = 0;
	bluray_playback.fullscreen = false;
	bluray_playback.deinterlace = false;
	memset(bluray_playback.audio_lang, '\0', sizeof(bluray_playback.audio_lang));
	memset(bluray_playback.subtitles_lang, '\0', sizeof(bluray_playback.subtitles_lang));
	memset(bluray_playback.chapter_start, '\0', sizeof(bluray_playback.chapter_start));
	memset(bluray_playback.chapter_end, '\0', sizeof(bluray_playback.chapter_end));

	char bluray_mpv_args[9];
	memset(bluray_mpv_args, '\0', sizeof(bluray_mpv_args));
	mpv_handle *bluray_mpv = NULL;
	mpv_event *bluray_mpv_event = NULL;
	struct mpv_event_log_message *bluray_mpv_log_message = NULL;

	char *token = NULL;
	int g_opt = 0;
	int g_ix = 0;
	const char p_short_opts[] = "a:c:dfhk:l:mp:s:t:qVz";
	struct option p_long_opts[] = {
		{ "chapters", required_argument, NULL, 'c' },
		{ "deinterlace", no_argument, NULL, 'd' },
		{ "fullscreen", no_argument, NULL, 'f' },
		{ "help", no_argument, NULL, 'h' },
		{ "keydb", required_argument, NULL, 'k' },
		{ "main", no_argument, NULL, 'm' },
		{ "alang", required_argument, NULL, 'a' },
		{ "slang", required_argument, NULL, 's' },
		{ "playlist", required_argument, NULL, 'p' },
		{ "title", required_argument, NULL, 't' },
		{ "version", no_argument, NULL, 'V' },
		{ 0, 0, 0, 0 }
	};
	while((g_opt = getopt_long(argc, argv, p_short_opts, p_long_opts, &g_ix)) != -1) {

		switch(g_opt) {

			case 'a':
				strncpy(bluray_playback.audio_lang, optarg, 3);
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
					arg_number = strtol(optarg, NULL, 10);
					if(arg_number > 0) {
						arg_first_chapter = (uint8_t)arg_number;
					}
					arg_number = 0;
					break;
				}

				// passed -c -<chapter number>
				if(strncmp(optarg, "-", 1) == 0) {
					opt_chapter_start = false;
					opt_chapter_end = true;
					token = strtok(optarg, "-");
					arg_number = strtol(token, NULL, 10);
					if(arg_number > 0) {
						arg_last_chapter = (uint8_t)arg_number;
					}
					arg_number = 0;
					break;
				}

				// passed -c <chapter-number>-[|<chapter-number>]
				opt_chapter_start = true;
				token = strtok(optarg, "-");
				arg_number = strtol(token, NULL, 10);
				if(arg_number > 0) {
					arg_first_chapter = (uint8_t)arg_number;
				}
				arg_number = 0;

				token = strtok(NULL, "-");

				// passed -c <chapter-number>-<chapter-number>
				if(token != NULL) {
					opt_chapter_end = true;
					arg_number = strtol(token, NULL, 10);
					if(arg_number > 0) {
						arg_last_chapter = (uint8_t)arg_number;
					}
					arg_number = 0;
					break;
				}

				// at this point, only passed -c <chapter-number>- which is
				// handled the same as -c <chapter-number>
				opt_chapter_end = false;

				break;


			case 'k':
				key_db_filename = optarg;
				break;

			case 'm':
				opt_main_title = true;
				break;

			case 'p':
				opt_playlist_number = true;
				arg_number = strtol(optarg, NULL, 10);
				if(arg_number < 0) {
					arg_playlist_number = 0;
				} else {
					arg_playlist_number = (uint32_t)arg_number;
				}
				arg_number = 0;
				break;

			case 't':
				opt_title_number = true;
				arg_number = strtol(optarg, NULL, 10);
				if(arg_number < 1) {
					arg_title_number = 1;
				} else {
					arg_title_number = (uint32_t)arg_number;
				}
				arg_number = 0;
				break;

			case 's':
				strncpy(bluray_playback.subtitles_lang, optarg, 3);
				break;

			case 'V':
				printf("bluray_player %s\n", PACKAGE_VERSION);
				return 0;

			case '?':
				invalid_opt = true;
			case 'h':
				printf("bluray_player %s - Play back a Blu-ray using libmpv engine\n", PACKAGE_VERSION);
				printf("\n");
				printf("Usage: bluray_player [bluray path] [options]\n");
				printf("\n");
				printf("Title selection:\n");
				printf("  -m, --main               Play main title (default)\n");
				printf("  -t, --title <#>          Play title number\n");
				printf("  -p, --playlist <#>       Play playlist number\n");
				printf("  -c, --chapters <#>[-#]   Play chapter number(s)\n");
				printf("\n");
				printf("Languages - ISO 639-2 three-letter language codes (eng, deu, fra, spa, ...):\n");
				printf("  -a, --alang <language>   Audio language (default: first stream)\n");
				printf("  -s, --slang <language>   Subtitles language (default: first stream)\n");
				printf("\n");
				printf("Playback:\n");
				printf("  -f, --fullscreen	   Display fullscreen\n");
				printf("  -d, --deinterlace	   Deinterlace video\n");
				printf("\n");
				printf("Other:\n");
				printf("  -k, --keydb <filename>   Location to KEYDB.cfg (default: ~/.config/aacs/KEYDB.cfg)\n");
				printf("  -h, --help		   This output\n");
				printf("  -V, --version		   Version information\n");
				printf("\n");
				printf("Blu-ray path can be a device, a filename, or directory (default: %s)\n", DEFAULT_BLURAY_DEVICE);
				if(invalid_opt)
					return 1;
				return 0;

			case 0:
			default:
				break;

		}

	}

	if(!opt_title_number && !opt_playlist_number)
		opt_main_title = true;

	if((opt_main_title + opt_title_number + opt_playlist_number) > 1) {
		fprintf(stderr, "Select only one option of title, default title, or playlist\n");
		return 1;
	}

	const char *device_filename = NULL;

	if(argv[optind]) {
		device_filename = argv[optind];
	} else {
		device_filename = DEFAULT_BLURAY_DEVICE;
	}

	// Open device
	BLURAY *bd = NULL;
	bd = bd_open(device_filename, key_db_filename);

	if(bd == NULL) {
		if(key_db_filename == NULL)
			fprintf(stderr, "Could not open device %s\n", device_filename);
		else
			fprintf(stderr, "Could not open device %s and key_db file %s\n", device_filename, key_db_filename);
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
	retval = bluray_info_init(bd, &bluray_info);

	d_num_titles = bluray_info.titles;

	struct bluray_title bluray_title;

	// Select title passed as an argument
	if(opt_title_number) {
		bluray_title.ix = arg_title_number - 1;
		if(arg_title_number > d_num_titles) {
			fprintf(stderr, "Could not open title %" PRIu32 ", choose from 1 to %" PRIu32 "\n", arg_title_number, d_num_titles);
			bd_close(bd);
			bd = NULL;
			return 1;
		}
		if(bd_select_title(bd, bluray_title.ix) == 0) {
			fprintf(stderr, "Could not open title %" PRIu32 "\n", arg_title_number);
			bd_close(bd);
			bd = NULL;
			return 1;
		}
	} else if(opt_playlist_number) {
		if(bd_select_playlist(bd, bluray_title.playlist) == 0) {
			fprintf(stderr, "Could not open playlist %" PRIu32 "\n", arg_playlist_number);
			bd_close(bd);
			bd = NULL;
			return 1;
		}
		bluray_title.ix = bd_get_current_title(bd);
	} else {
		bluray_title.ix = bluray_info.main_title_ix;
		if(bd_select_title(bd, bluray_title.ix) == 0) {
			fprintf(stderr, "Could not open main title # %" PRIu32 "\n", bluray_info.main_title_ix + 1);
			bd_close(bd);
			bd = NULL;
			return 1;
		}
	}

	// Init bluray_title struct
	retval = bluray_title_init(bd, &bluray_title, bluray_title.ix);

	// MPV zero-indexes title numbers
	bluray_playback.title = bluray_title.ix;

	if(strlen(bluray_info.disc_name) && d_num_titles)
		printf("Disc title: %s\n", bluray_info.disc_name);

	printf("Title: %03" PRIu32 ", Playlist: %04" PRIu32 ", Length: %s, Chapters: %02" PRIu32 ", Video streams: %02" PRIu8 ", Audio streams: %02" PRIu8 ", Subtitles: %02" PRIu8 ", Filesize: %05.0lf MBs\n", bluray_title.number, bluray_title.playlist, bluray_title.length, bluray_title.chapters, bluray_title.video_streams, bluray_title.audio_streams, bluray_title.pg_streams, bluray_title.size_mbs);

	// Finished with libbluray
	bd_close(bd);
	bd = NULL;

	// Blu-ray playback using libmpv
	bluray_mpv = mpv_create();

	if(arg_last_chapter > 0 && arg_last_chapter > bluray_title.chapters)
		arg_last_chapter = bluray_title.chapters;
	if(arg_last_chapter > 0 && arg_first_chapter > arg_last_chapter)
		arg_first_chapter = arg_last_chapter;

	// When choosing a chapter range, mpv will add 1 to the last one requested
	snprintf(bluray_playback.chapter_start, 5, "#%03" PRIu32, arg_first_chapter);
	snprintf(bluray_playback.chapter_end, 5, "#%03" PRIu32, arg_last_chapter + 1);

	// Load user's mpv configuration in ~/.config/bluray_player/mpv.conf (and friends)
	if(strlen(bluray_player.mpv_config_dir) > 0) {
		mpv_set_option_string(bluray_mpv, "config-dir", bluray_player.mpv_config_dir);
		mpv_set_option_string(bluray_mpv, "config", "yes");
	}

	// Playback options and default configuration
	mpv_set_option_string(bluray_mpv, "bluray-device", device_filename);
	mpv_set_option_string(bluray_mpv, "terminal", "yes");
	mpv_set_option_string(bluray_mpv, "term-osd-bar", "yes");
	mpv_set_option_string(bluray_mpv, "input-default-bindings", "yes");
	mpv_set_option_string(bluray_mpv, "input-vo-keyboard", "yes");
	if(strlen(bluray_playback.audio_lang) > 0)
		mpv_set_option_string(bluray_mpv, "alang", bluray_playback.audio_lang);
	if(strlen(bluray_playback.subtitles_lang) > 0)
		mpv_set_option_string(bluray_mpv, "slang", bluray_playback.subtitles_lang);
	if(bluray_playback.fullscreen)
		mpv_set_option_string(bluray_mpv, "fullscreen", NULL);
	if(bluray_playback.deinterlace)
		mpv_set_option_string(bluray_mpv, "deinterlace", "yes");
	if(opt_chapter_start && arg_first_chapter > 0)
		mpv_set_option_string(bluray_mpv, "start", bluray_playback.chapter_start);
	if(opt_chapter_end && arg_last_chapter > 0)
		mpv_set_option_string(bluray_mpv, "end", bluray_playback.chapter_end);

	// mpv zero-indexes titles
	sprintf(bluray_mpv_args, "bd://%03" PRIu32, bluray_playback.title);

	// MPV uses zero-indexing for titles, bluray_info uses one instead
	const char *bluray_mpv_commands[] = {
		"loadfile",
		bluray_mpv_args,
		NULL
	};

	mpv_initialize(bluray_mpv);
	mpv_command(bluray_mpv, bluray_mpv_commands);

	while(true) {

		bluray_mpv_event = mpv_wait_event(bluray_mpv, -1);

		if(bluray_mpv_event->event_id == MPV_EVENT_SHUTDOWN || bluray_mpv_event->event_id == MPV_EVENT_END_FILE)
			break;

		if(bluray_mpv_event->event_id == MPV_EVENT_LOG_MESSAGE) {
			bluray_mpv_log_message = (struct mpv_event_log_message *)bluray_mpv_event->data;
			printf("mpv [%s]: %s", bluray_mpv_log_message->level, bluray_mpv_log_message->text);
		}
	}

	mpv_terminate_destroy(bluray_mpv);

	return 0;

}
