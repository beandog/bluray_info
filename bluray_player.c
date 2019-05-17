#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#if defined (__linux__)
#include <linux/limits.h>
#endif
#include <libbluray/bluray.h>
#include "bluray_device.h"
#include "bluray_info.h"
#include "bluray_time.h"
#include <mpv/client.h>

struct bluray_info {
	char bluray_id[BLURAY_ID + 1];
	char bluray_title[BLURAY_TITLE + 1];
	uint32_t hdmv_titles;
	uint32_t bdj_titles;
	uint32_t unsupported_titles;
	uint32_t titles;
	uint32_t relevant_titles;
	uint32_t longest_title;
	int main_title;
};

struct bluray_player {
	char config_dir[23];
	char mpv_config_dir[PATH_MAX - 1];
};

struct bluray_playback {
	uint32_t title;
	uint32_t playlist;
	bool fullscreen;
	bool deinterlace;
	char audio_lang[4];
	char subtitles_lang[4];
	char chapter_start[5];
	char chapter_end[5];
	char loop[4];
};

struct bluray_title {
	uint32_t ix;
	uint32_t playlist;
	uint64_t duration;
	uint64_t size;
	uint64_t size_mbs;
	uint32_t chapters;
	uint32_t clips;
	uint8_t angles;
	uint8_t video_streams;
	uint8_t audio_streams;
	uint8_t pg_streams;
	char length[BLURAY_DURATION + 1];
};

int main(int argc, char **argv) {

	uint32_t ix = 0;
	uint32_t d_num_titles = 0;

	bool opt_title_number = false;
	bool opt_playlist_number = false;
	bool opt_main_title = false;
	bool opt_chapter_number = false;
	bool opt_chapter_start = false;
	bool opt_chapter_end = false;
	bool opt_chapter_range = false;
	bool invalid_opt = false;
	bool debug = false;
	uint32_t arg_title_number = 0;
	uint32_t arg_playlist_number = 0;
	uint32_t arg_first_chapter = 0;
	uint32_t arg_last_chapter = 0;
	const char *key_db_filename = NULL;
	const char *subtitles_filename = NULL;
	const char *home_dir = getenv("HOME");

	struct bluray_player bluray_player;
	snprintf(bluray_player.config_dir, 23, "/.config/bluray_player");
	memset(bluray_player.mpv_config_dir, '\0', sizeof(bluray_player.mpv_config_dir));
	if(home_dir != NULL)
		snprintf(bluray_player.mpv_config_dir, PATH_MAX - 1, "%s%s", home_dir, bluray_player.config_dir);

	struct bluray_playback bluray_playback;
	bluray_playback.title = 1;
	bluray_playback.playlist = 0;
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

	memset(bluray_playback.loop, '\0', sizeof(bluray_playback.loop));

	char *token = NULL;
	int g_opt = 0;
	int g_ix = 0;
	opterr = 1;
	const char p_short_opts[] = "a:c:dfhk:l:mp:S:s:t:qVz";
	struct option p_long_opts[] = {
		{ "chapters", required_argument, NULL, 'c' },
		{ "deinterlace", no_argument, NULL, 'd' },
		{ "fullscreen", no_argument, NULL, 'f' },
		{ "help", no_argument, NULL, 'h' },
		{ "keydb", required_argument, NULL, 'k' },
		{ "loop", no_argument, NULL, 'l' },
		{ "main", no_argument, NULL, 'm' },
		{ "alang", required_argument, NULL, 'a' },
		{ "slang", required_argument, NULL, 's' },
		{ "playlist", required_argument, NULL, 'p' },
		{ "title", required_argument, NULL, 't' },
		{ "version", no_argument, NULL, 'V' },
		{ "debug", no_argument, NULL, 'z' },
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
					opt_chapter_number = true;
					opt_chapter_start = true;
					opt_chapter_end = false;
					opt_chapter_range = false;
					arg_first_chapter = (uint8_t)strtoumax(optarg, NULL, 0);
					break;
				}

				// passed -c -<chapter number>
				if(strncmp(optarg, "-", 1) == 0) {
					opt_chapter_number = true;
					opt_chapter_start = false;
					opt_chapter_end = true;
					opt_chapter_range = true;
					token = strtok(optarg, "-");
					arg_last_chapter = (uint8_t)strtoumax(token, NULL, 0);
					break;
				}

				// passed -c <chapter-number>-[|<chapter-number>]
				opt_chapter_number = true;
				opt_chapter_start = true;
				token = strtok(optarg, "-");
				arg_first_chapter = (uint8_t)strtoumax(token, NULL, 0);

				token = strtok(NULL, "-");

				// passed -c <chapter-number>-<chapter-number>
				if(token != NULL) {
					opt_chapter_end = true;
					opt_chapter_range = true;
					arg_last_chapter = (uint8_t)strtoumax(token, NULL, 0);
					break;
				}

				// at this point, only passed -c <chapter-number>- which is
				// handled the same as -c <chapter-number>
				opt_chapter_end = false;
				opt_chapter_range = false;

				break;


			case 'k':
				key_db_filename = optarg;
				break;

			case 'l':
				strncpy(bluray_playback.loop, optarg, 3);
				break;

			case 'm':
				opt_main_title = true;
				break;

			case 'p':
				opt_playlist_number = true;
				arg_playlist_number = (unsigned int)strtoumax(optarg, NULL, 0);
				break;

			case 't':
				opt_title_number = true;
				arg_title_number = (unsigned int)strtoumax(optarg, NULL, 0);
				break;

			case 'S':
				subtitles_filename = optarg;
				break;

			case 's':
				strncpy(bluray_playback.subtitles_lang, optarg, 3);
				break;

			case 'V':
				printf("bluray_player %s\n", BLURAY_INFO_VERSION);
				return 0;

			case 'z':
				debug = true;
				break;

			case '?':
				invalid_opt = true;
			case 'h':
				printf("bluray_player %s\n", BLURAY_INFO_VERSION);
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
				printf("  -l, --loop <#|inf>	   Loop playback number of times, or infinitely\n");
				printf("\n");
				printf("Other:\n");
				printf("  -k, --keydb <filename>   Location to KEYDB.CFG (default: ~/.config/aacs/KEYDB.cfg)\n");
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

	BLURAY_TITLE_INFO *bd_title = NULL;

	// Blu-ray
	struct bluray_info bluray_info;
	memset(bluray_info.bluray_id, '\0', sizeof(bluray_info.bluray_id));
	memset(bluray_info.bluray_title, '\0', sizeof(bluray_info.bluray_title));
	bluray_info.hdmv_titles = 0;
	bluray_info.bdj_titles = 0;
	bluray_info.unsupported_titles = 0;
	bluray_info.longest_title = 0;
	bluray_info.main_title = 1;

	if(bd_info->udf_volume_id)
		strncpy(bluray_info.bluray_title, bd_info->udf_volume_id, BLURAY_TITLE + 1);
	bluray_info.titles = bd_info->num_titles;
	if(bd_info->libaacs_detected) {
		for(ix = 0; ix < 20; ix++) {
			sprintf(bluray_info.bluray_id + 2 * ix, "%02X", bd_info->disc_id[ix]);
		}
	}

	// Use "relevant titles" as index / reference
	bluray_info.relevant_titles = bd_get_titles(bd, TITLES_RELEVANT, 0);
	d_num_titles = (uint32_t)bluray_info.relevant_titles;
	bluray_info.main_title = bd_get_main_title(bd);

	struct bluray_title bluray_title;
	bluray_title.ix = 0;
	bluray_title.playlist = 0;
	bluray_title.duration = 0;
	bluray_title.size = 0;
	bluray_title.size_mbs = 0;
	bluray_title.chapters = 0;
	bluray_title.clips = 0;
	bluray_title.angles = 0;
	bluray_title.video_streams = 0;
	bluray_title.audio_streams = 0;
	bluray_title.pg_streams = 0;
	snprintf(bluray_title.length, BLURAY_DURATION + 1, "%s", "00:00:00.00");

	// Select title passed as an argument
	if(opt_title_number) {
		bluray_title.ix = arg_title_number - 1;
		if(bluray_title.ix > (d_num_titles - 1)) {
			fprintf(stderr, "Could not open title %u, choose from 1 to %u\n", bluray_title.ix + 1, d_num_titles);
			bd_close(bd);
			bd = NULL;
			return 1;
		}
		if(bd_select_title(bd, bluray_title.ix) == 0) {
			fprintf(stderr, "Could not open title %u\n", bluray_title.ix + 1);
			bd_close(bd);
			bd = NULL;
			return 1;
		}
		bd_title = bd_get_title_info(bd, bluray_title.ix, 0);
		bluray_title.playlist = bd_title->playlist;
	} else if(opt_playlist_number) {
		bluray_title.playlist = arg_playlist_number;
		if(bd_select_playlist(bd, bluray_title.playlist) == 0) {
			fprintf(stderr, "Could not open playlist %u\n", bluray_title.playlist);
			bd_close(bd);
			bd = NULL;
			return 1;
		}
		bluray_title.ix = bd_get_current_title(bd);
		bd_title = bd_get_title_info(bd, bluray_title.ix, 0);
	} else {
		bluray_title.ix = (uint32_t)bluray_info.main_title;
		if(bd_select_title(bd, bluray_title.ix) == 0) {
			fprintf(stderr, "Could not open title %u\n", bluray_title.ix + 1);
			bd_close(bd);
			bd = NULL;
			return 1;
		}
		bd_title = bd_get_title_info(bd, bluray_title.ix, 0);
	}

	if(bd_title == NULL) {
		fprintf(stderr, "Could not get info for title %u\n", bluray_title.ix + 1);
		bd_close(bd);
		bd = NULL;
		return 1;
	}

	bluray_playback.title = bluray_title.ix + 1;

	if(bd_info->udf_volume_id && d_num_titles)
		printf("Disc Title: %s\n", bluray_info.bluray_title);

	bluray_title.duration = bd_title->duration;
	bluray_title.size = bd_get_title_size(bd);
	bluray_title.size_mbs = bluray_title.size / 1024 / 1024;
	bluray_title.chapters = bd_title->chapter_count;
	bluray_title.clips = bd_title->clip_count;
	bluray_title.angles = bd_title->angle_count;
	bluray_duration_length(bluray_title.length, bluray_title.duration);

	if(bluray_title.clips) {
		bluray_title.video_streams = bd_title->clips[0].video_stream_count;
		bluray_title.audio_streams = bd_title->clips[0].audio_stream_count;
		bluray_title.pg_streams = bd_title->clips[0].pg_stream_count;
	}

	printf("Title: %03u, Playlist: %04u, Length: %s, Chapters: %02u, Video streams: %02u, Audio streams: %02u, Subtitles: %02u, Filesize: %05lu MBs\n", bluray_title.ix + 1, bluray_title.playlist, bluray_title.length, bluray_title.chapters, bluray_title.video_streams, bluray_title.audio_streams, bluray_title.pg_streams, bluray_title.size_mbs);

	// Finished with libbluray
	bd_free_title_info(bd_title);
	bd_title = NULL;

	bd_close(bd);
	bd = NULL;

	// Blu-ray playback using libmpv
	bluray_mpv = mpv_create();

	if(arg_last_chapter > 0 && arg_last_chapter > bluray_title.chapters)
		arg_last_chapter = bluray_title.chapters;
	if(arg_last_chapter > 0 && arg_first_chapter > arg_last_chapter)
		arg_first_chapter = arg_last_chapter;

	// When choosing a chapter range, mpv will add 1 to the last one requested
	snprintf(bluray_playback.chapter_start, 5, "#%03d", arg_first_chapter);
	snprintf(bluray_playback.chapter_end, 5, "#%03d", arg_last_chapter + 1);

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
	if(strlen(bluray_playback.loop) > 0)
		mpv_set_option_string(bluray_mpv, "loop", bluray_playback.loop);

	// mpv zero-indexes titles
	sprintf(bluray_mpv_args, "bd://%03u", bluray_playback.title - 1);

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
