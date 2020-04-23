#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <math.h>
#include <getopt.h>
#include "libbluray/bluray.h"
#include "libbluray/meta_data.h"
#include "bluray_device.h"
#include "bluray_open.h"
#include "bluray_audio.h"
#include "bluray_video.h"
#include "bluray_pgs.h"
#include "bluray_time.h"

/**
 *   _     _                           _        __
 *  | |__ | |_   _ _ __ __ _ _   _    (_)_ __  / _| ___
 *  | '_ \| | | | | '__/ _` | | | |   | | '_ \| |_ / _ \
 *  | |_) | | |_| | | | (_| | |_| |   | | | | |  _| (_) |
 *  |_.__/|_|\__,_|_|  \__,_|\__, |___|_|_| |_|_|  \___/
 *                           |___/_____|
 *
 * display information about a disc using libbluray
 *
 * If your Blu-ray is encrypted, you'll need libaacs as well as a modern
 * KEYDB.cfg file to access them.
 *
 * More information at https://dvds.beandog.org/
 *
 * Go crazy :)
 *
 */

int main(int argc, char **argv) {

	int retval = 0;

	// Parse options and arguments
	bool p_bluray_info = true;
	bool p_bluray_json = false;
	bool p_bluray_xchap = false;
	bool d_title_number = false;
	uint32_t arg_title_number = 0;
	bool d_playlist_number = false;
	uint32_t arg_playlist_number = 0;
	bool d_main_title = false;
	bool d_video = false;
	bool d_audio = false;
	bool d_subtitles = false;
	bool d_chapters = false;
	unsigned long int arg_number = 0;
	uint32_t d_min_seconds = 0;
	uint32_t d_min_minutes = 0;
	uint32_t d_min_audio_streams = 0;
	uint32_t d_min_pg_streams = 0;
	bool invalid_opt = false;
	const char *key_db_filename = NULL;
	int g_opt = 0;
	int g_ix = 0;
	struct option p_long_opts[] = {
		{ "audio", no_argument, NULL, 'a' },
		{ "chapters", no_argument, NULL, 'c' },
		{ "xchap", no_argument, NULL, 'g' },
		{ "help", no_argument, NULL, 'h' },
		{ "json", no_argument, NULL, 'j' },
		{ "keydb", required_argument, NULL, 'k' },
		{ "main", no_argument, NULL, 'm' },
		{ "playlist", required_argument, NULL, 'p' },
		{ "subtitles", no_argument, NULL, 's' },
		{ "title", required_argument, NULL, 't' },
		{ "video", no_argument, NULL, 'v' },
		{ "all", no_argument, NULL, 'x' },
		{ "has-audio", no_argument, NULL, 'A' },
		{ "seconds", required_argument, NULL, 'E' },
		{ "minutes", required_argument, NULL, 'M' },
		{ "has-subtitles", no_argument, NULL, 'S' },
		{ "version", no_argument, NULL, 'Z' },
		{ 0, 0, 0, 0 }
	};
	while((g_opt = getopt_long(argc, argv, "acghjk:mp:st:vxAE:M:SZ", p_long_opts, &g_ix)) != -1) {

		switch(g_opt) {

			case 'a':
				d_audio = true;
				break;

			case 'A':
				d_min_audio_streams = 1;
				break;

			case 'c':
				d_chapters = true;
				break;

			case 'E':
				arg_number = strtoul(optarg, NULL, 10);
				d_min_seconds = (uint32_t)arg_number;
				break;

			case 'g':
				p_bluray_info = false;
				p_bluray_xchap = true;
				break;

			case 'H':
				break;

			case 'j':
				p_bluray_info = false;
				p_bluray_json = true;
				break;

			case 'k':
				key_db_filename = optarg;
				break;

			case 'm':
				d_title_number = false;
				d_playlist_number = false;
				d_main_title = true;
				break;

			case 'M':
				arg_number = strtoul(optarg, NULL, 10);
				d_min_minutes = (uint32_t)arg_number;
				break;

			case 'p':
				d_title_number = false;
				d_playlist_number = true;
				d_main_title = false;
				arg_number = strtoul(optarg, NULL, 10);
				arg_playlist_number = (uint32_t)arg_number;
				break;

			case 's':
				d_subtitles = true;
				break;

			case 'S':
				d_min_pg_streams = 1;
				break;

			case 't':
				d_title_number = true;
				d_playlist_number = false;
				d_main_title = false;
				arg_number = strtoul(optarg, NULL, 10);
				if(arg_number < 2)
					arg_title_number = 1;
				else
					arg_title_number = (uint32_t)arg_number;
				break;

			case 'v':
				d_video = true;
				break;

			case 'x':
				d_video = true;
				d_audio = true;
				d_chapters = true;
				d_subtitles = true;
				break;

			case 'Z':
				printf("bluray_info %s\n", PACKAGE_VERSION);
				return 0;

			case '?':
				invalid_opt = true;
			case 'h':
				printf("bluray_info %s - display information about a Blu-ray\n", PACKAGE_VERSION);
				printf("\n");
				printf("Usage: bluray_info [path] [options]\n");
				printf("\n");
				printf("Options:\n");
				printf("  -m, --main  	 	   Limit to main title (default: all)\n");
				printf("  -t, --title <number>     Limit to selected title\n");
				printf("  -p, --playlist <number>  Limit to selected playlist\n");
				printf("  -j, --json               Display format as JSON\n");
				printf("\n");
				printf("Extra information:\n");
				printf("  -v, --video              Display video streams\n");
				printf("  -a, --audio              Display audio streams\n");
				printf("  -s, --subtitles          Display subtitles\n");
				printf("  -c, --chapters           Display chapters\n");
				printf("  -x, --all                Display all\n");
				printf("\n");
				printf("Narrow results:\n");
				printf("  -A, --has-audio          Title has audio\n");
				printf("  -S, --has-subtitles      Title has subtitles\n");
				printf("  -E, --seconds <number>   Title has minimum number of seconds\n");
				printf("  -M, --minutes <number>   Title has minimum number of minutes\n");
				printf("\n");
				printf("Other:\n");
				printf("  -g, --xchap		   Display title's chapter format for mkvmerge\n");
				printf("  -k, --keydb <filename>   Location to KEYDB.cfg (default: ~/.config/aacs/KEYDB.cfg)\n");
				printf("  -h, --help		   This output\n");
				printf("      --version		   Version information\n");
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

	const char *device_filename = NULL;

	if(argv[optind])
		device_filename = argv[optind];
	else
		device_filename = DEFAULT_BLURAY_DEVICE;

	// Open device
	BLURAY *bd = NULL;
	bd = bd_open(device_filename, key_db_filename);

	if(bd == NULL) {
		if(key_db_filename == NULL)
			printf("Could not open device %s\n", device_filename);
		else
			printf("Could not open device %s and key_db file %s\n", device_filename, key_db_filename);
		return 1;
	}

	// Blu-ray
	struct bluray_info bluray_info;
	retval = bluray_info_init(bd, &bluray_info);

	if(retval) {
		printf("* Couldn't open Blu-ray\n");
		return 1;
	}

	uint32_t d_num_titles = 0;
	d_num_titles = bluray_info.titles;

	uint32_t d_first_ix = 0;

	// Select track passed as an argument
	if(d_title_number) {
		if(arg_title_number > bluray_info.titles || arg_title_number < 1) {
			printf("Could not open title %" PRIu32 ", choose from 1 to %" PRIu32 "\n", arg_title_number, bluray_info.titles);
			return 1;
		}
		retval = bd_select_title(bd, arg_title_number - 1);
		if(retval == 0) {
			printf("Could not open title %" PRIu32 "\n", arg_title_number);
			return 1;
		}
		d_first_ix = arg_title_number - 1;
		d_num_titles = 1;
	}

	if(d_playlist_number) {
		retval = bd_select_playlist(bd, arg_playlist_number);
		if(retval == 0) {
			printf("Could not open playlist %" PRIu32 "\n", arg_playlist_number);
			return 1;
		}
		d_first_ix = bd_get_current_title(bd);
		d_num_titles = 1;
	}

	if(d_main_title) {
		d_first_ix = bluray_info.main_title;
		d_num_titles = 1;
	}

	uint32_t main_title_number;
	main_title_number = bluray_info.main_title + 1;

	if(p_bluray_info) {
		printf("Disc title: '%s', Volume name: '%s', Main title: %03" PRIu32 ", AACS: %s, BD-J: %s, BD+: %s\n", bluray_info.disc_name, bluray_info.udf_volume_id, main_title_number, (bluray_info.aacs ? "yes" : "no"), (bluray_info.bdj ? "yes" : "no"), (bluray_info.bdplus ? "yes": "no"));
	}

	uint32_t ix = 0;
	uint8_t angle_ix = 0;

	if(p_bluray_json) {

		// Find the longest title
		uint64_t max_duration = 0;
		uint32_t main_playlist = 0;
		uint32_t longest_title_number = 1;
		uint32_t longest_playlist = 0;
		BLURAY_TITLE_INFO *bd_title = NULL;
		for(ix = 0; ix < bluray_info.titles; ix++) {

			bd_title = bd_get_title_info(bd, ix, angle_ix);

			if(bd_title == NULL) {
				continue;
			}

			if(bd_title->idx == bluray_info.main_title)
				main_playlist = bd_title->playlist;

			if(bd_title->duration > max_duration) {
				longest_title_number = ix + 1;
				longest_playlist = bd_title->playlist;
				max_duration = bd_title->duration;
			}

			bd_free_title_info(bd_title);
			bd_title = NULL;

		}

		printf("{\n");
		printf(" \"bluray\": {\n");
		printf("  \"disc name\": \"%s\",\n", bluray_info.disc_name);
		printf("  \"udf title\": \"%s\",\n", bluray_info.udf_volume_id);
		printf("  \"disc id\": \"%s\",\n", bluray_info.disc_id);
		printf("  \"main title\": %" PRIu32 ",\n", main_title_number);
		printf("  \"main playlist\": %" PRIu32 ",\n", main_playlist);
		printf("  \"longest title\": %" PRIu32 ",\n", longest_title_number);
		printf("  \"longest playlist\": %" PRIu32 ",\n", longest_playlist);
		printf("  \"first play supported\": %s,\n", (bluray_info.first_play_supported ? "true" : "false"));
		printf("  \"top menu supported\": %s,\n", (bluray_info.top_menu_supported ? "true" : "false"));
		printf("  \"provider data\": \"%s\",\n", bluray_info.provider_data);
		printf("  \"3D content\": %s,\n", (bluray_info.content_exist_3D ? "true" : "false"));
		printf("  \"initial mode\": \"%s\",\n", bluray_info.initial_output_mode_preference);
		printf("  \"titles\": %" PRIu32 ",\n", bluray_info.titles);
		printf("  \"bdinfo titles\": %" PRIu32 ",\n", bluray_info.disc_num_titles);
		printf("  \"hdmv titles\": %" PRIu32 ",\n", bluray_info.hdmv_titles);
		printf("  \"bdj titles\": %" PRIu32 ",\n", bluray_info.bdj_titles);
		printf("  \"unsupported titles\": %" PRIu32 ",\n", bluray_info.unsupported_titles);
		printf("  \"aacs\": %s,\n", (bluray_info.aacs ? "true" : "false"));
		printf("  \"bdplus\": %s,\n", (bluray_info.bdplus ? "true" : "false"));
		printf("  \"bd-j\": %s\n", (bluray_info.bdj ? "true" : "false"));
		printf(" },\n");

	}

	BLURAY_STREAM_INFO *bd_stream = NULL;
	BLURAY_TITLE_CHAPTER *bd_chapter = NULL;

	struct bluray_title bluray_title;
	struct bluray_video bluray_video;
	struct bluray_audio bluray_audio;
	struct bluray_pgs bluray_pgs;
	struct bluray_chapter bluray_chapter;
	bluray_chapter.duration = 0;
	strcpy(bluray_chapter.length, "00:00:00.000");

	uint32_t bluray_highest_playlist = 0;

	if(p_bluray_json)
		printf(" \"titles\": [\n");

	uint8_t video_stream_ix = 0;
	uint8_t video_stream_number = 1;
	uint8_t audio_stream_ix = 0;
	uint8_t audio_stream_number = 1;
	uint8_t pg_stream_ix = 0;
	uint8_t pg_stream_number = 1;
	uint32_t chapter_ix = 0;
	uint32_t chapter_number = 1;
	uint64_t chapter_start = 0;
	uint32_t d_title_counter = 0;
	angle_ix = 0;

	for(ix = d_first_ix; d_title_counter < d_num_titles; ix++, d_title_counter++) {

		retval = bluray_title_init(bd, &bluray_title, ix, angle_ix);

		// Skip if there was a problem getting it
		if(retval)
			continue;

		bluray_highest_playlist = ((bluray_title.playlist > bluray_highest_playlist) ? bluray_title.playlist : bluray_highest_playlist);

		if(p_bluray_info) {

			if(!(bluray_title.seconds >= d_min_seconds && bluray_title.minutes >= d_min_minutes && bluray_title.audio_streams >= d_min_audio_streams && bluray_title.pg_streams >= d_min_pg_streams)) {
				bd_stream = NULL;
				continue;
			}

			printf("Title: %03" PRIu32 ", Playlist: %04" PRIu32 ", Length: %s, Chapters: %03"PRIu32 ", Video streams: %02" PRIu8 ", Audio streams: %02" PRIu8 ", Subtitles: %02" PRIu8 ", Angles: %02" PRIu8 ", Filesize: %05.0lf MBs\n", bluray_title.number, bluray_title.playlist, bluray_title.length, bluray_title.chapters, bluray_title.video_streams, bluray_title.audio_streams, bluray_title.pg_streams, bluray_title.angles, bluray_title.size_mbs);

		}

		if(p_bluray_json) {

			if(!(bluray_title.seconds >= d_min_seconds && bluray_title.minutes >= d_min_minutes && bluray_title.audio_streams >= d_min_audio_streams && bluray_title.pg_streams >= d_min_pg_streams)) {
				bd_stream = NULL;
				continue;
			}

			printf("  {\n");
			printf("   \"title\": %u,\n", bluray_title.number);
			printf("   \"playlist\": %" PRIu32 ",\n", bluray_title.playlist);
			printf("   \"length\": \"%s\",\n", bluray_title.length);
			printf("   \"msecs\": %" PRIu64 ",\n", bluray_title.duration / 900);
			printf("   \"angles\": %" PRIu8 ",\n", bluray_title.angles);
			printf("   \"filesize\": %" PRIu64 ",\n", bluray_title.size);

		}

		// Blu-ray video streams
		if((p_bluray_info && d_video) || p_bluray_json) {

			if(p_bluray_json)
				printf("   \"video\": [\n");

			for(video_stream_ix = 0; video_stream_ix < bluray_title.video_streams; video_stream_ix++) {

				video_stream_number = video_stream_ix + 1;
				// bd_stream = &bd_title->clips[0].video_streams[video_stream_ix];
				bd_stream = &bluray_title.clip_info[0].video_streams[video_stream_ix];

				if(bd_stream == NULL)
					continue;

				bluray_video_codec(bluray_video.codec, bd_stream->coding_type);
				bluray_video_codec_name(bluray_video.codec_name, bd_stream->coding_type);
				bluray_video_format(bluray_video.format, bd_stream->format);
				bluray_video.framerate = bluray_video_framerate(bd_stream->rate);
				bluray_video_aspect_ratio(bluray_video.aspect_ratio, bd_stream->aspect);

				if(p_bluray_info && d_video) {
					printf("	Video: %02u, Format: %s, Aspect ratio: %s, FPS: %.02f, Codec: %s\n", video_stream_number, bluray_video.format, bluray_video.aspect_ratio, bluray_video.framerate, bluray_video.codec);
				}

				if(p_bluray_json) {
					printf("    {\n");
					printf("     \"track\": %" PRIu8 ",\n", video_stream_number);
					printf("     \"stream\": \"0x%x\",\n", bd_stream->pid);
					printf("     \"format\": \"%s\",\n", bluray_video.format);
					printf("     \"aspect ratio\": \"%s\",\n", bluray_video.aspect_ratio);
					printf("     \"framerate\": %.02f,\n", bluray_video.framerate);
					printf("     \"codec\": \"%s\",\n", bluray_video.codec);
					printf("     \"codec name\": \"%s\"\n", bluray_video.codec_name);
					if(video_stream_number < bluray_title.video_streams)
						printf("    },\n");
					else
						printf("    }\n");
				}

			}

			bd_stream = NULL;

			if(p_bluray_json)
				printf("   ],\n");

		}

		// Blu-ray audio streams
		if((p_bluray_info && d_audio) || p_bluray_json) {

			if(p_bluray_json)
				printf("   \"audio\": [\n");

			for(audio_stream_ix = 0; audio_stream_ix < bluray_title.audio_streams; audio_stream_ix++) {

				audio_stream_number = audio_stream_ix + 1;
				// bd_stream = &bd_title->clips[0].audio_streams[audio_stream_ix];
				bd_stream = &bluray_title.clip_info[0].audio_streams[audio_stream_ix];

				if(bd_stream == NULL)
					continue;

				bluray_audio_lang(bluray_audio.lang, bd_stream->lang);
				bluray_audio_codec(bluray_audio.codec, bd_stream->coding_type);
				bluray_audio_codec_name(bluray_audio.codec_name, bd_stream->coding_type);
				bluray_audio_format(bluray_audio.format, bd_stream->format);
				bluray_audio_rate(bluray_audio.rate, bd_stream->rate);

				if(p_bluray_info && d_audio) {
					printf("	Audio: %02" PRIu8 ", Language: %s, Codec: %s, Format: %s, Rate: %s\n", audio_stream_number, bluray_audio.lang, bluray_audio.codec, bluray_audio.format, bluray_audio.rate);
				}

				if(p_bluray_json) {
					printf("    {\n");
					printf("     \"track\": %" PRIu8 ",\n", audio_stream_number);
					printf("     \"stream\": \"0x%x\",\n", bd_stream->pid);
					printf("     \"language\": \"%s\",\n", bluray_audio.lang);
					printf("     \"codec\": \"%s\",\n", bluray_audio.codec);
					printf("     \"codec name\": \"%s\",\n", bluray_audio.codec_name);
					printf("     \"format\": \"%s\",\n", bluray_audio.format);
					printf("     \"rate\": \"%s\"\n", bluray_audio.rate);
					if(audio_stream_number < bluray_title.audio_streams)
						printf("    },\n");
					else
						printf("    }\n");
				}

			}

			bd_stream = NULL;

			if(p_bluray_json)
				printf("   ],\n");

		}

		// Blu-ray PGS streams
		if((p_bluray_info && d_subtitles) || p_bluray_json) {

			if(p_bluray_json)
				printf("   \"subtitles\": [\n");

			for(pg_stream_ix = 0; pg_stream_ix < bluray_title.pg_streams; pg_stream_ix++) {

				pg_stream_number = pg_stream_ix + 1;
				// bd_stream = &bd_title->clips[0].pg_streams[pg_stream_ix];
				bd_stream = &bluray_title.clip_info[0].pg_streams[pg_stream_ix];

				bluray_pgs_lang(bluray_pgs.lang, bd_stream->lang);

				if(bd_stream == NULL)
					continue;

				if(p_bluray_info && d_subtitles) {
					printf("	Subtitle: %02" PRIu8 ", Language: %s\n", pg_stream_number, bluray_pgs.lang);
				}

				if(p_bluray_json) {
					printf("    {\n");
					printf("     \"track\": %" PRIu8 ",\n", pg_stream_number);
					printf("     \"stream\": \"0x%x\",\n", bd_stream->pid);
					printf("     \"language\": \"%s\"\n", bluray_pgs.lang);
					if(pg_stream_number < bluray_title.pg_streams)
						printf("    },\n");
					else
						printf("    }\n");
				}

			}

			bd_stream = NULL;

			if(p_bluray_json)
				printf("   ],\n");

		}

		// Blu-ray chapters
		if((p_bluray_info && d_chapters) || p_bluray_json || p_bluray_xchap) {

			if(p_bluray_json)
				printf("   \"chapters\": [\n");

			for(chapter_ix = 0; chapter_ix < bluray_title.chapters; chapter_ix++) {

				chapter_number = chapter_ix + 1;
				// bd_chapter = &bd_title->chapters[chapter_ix];
				bd_chapter = &bluray_title.title_chapters[chapter_ix];

				if(bd_chapter == NULL)
					continue;

				bluray_chapter.start = chapter_start;
				bluray_chapter.duration = bd_chapter->duration;
				bluray_duration_length(bluray_chapter.length, bluray_chapter.duration);
				bluray_duration_length(bluray_chapter.start_time, bluray_chapter.start);

				if(p_bluray_info && d_chapters) {
					printf("	Chapter: %03" PRIu32 ", Start: %s, Length: %s\n", chapter_number, bluray_chapter.start_time, bluray_chapter.length);
				}

				if(p_bluray_json) {
					printf("    {\n");
					printf("     \"chapter\": %" PRIu32 ",\n", chapter_number);
					printf("     \"start time\": \"%s\",\n", bluray_chapter.start_time);
					printf("     \"length\": \"%s\",\n", bluray_chapter.length);
					printf("     \"start\": %" PRIu64 ",\n", bluray_chapter.start / 900);
					printf("     \"duration\": %" PRIu64 "\n", bd_chapter->duration / 900);
					if(chapter_number < bluray_title.chapters)
						printf("    },\n");
					else
						printf("    }\n");
				}

				if(p_bluray_xchap && ix == d_first_ix) {
					printf("CHAPTER%03" PRIu32 "=%s\n", chapter_number, bluray_chapter.start_time);
					printf("CHAPTER%03" PRIu32 "NAME=Chapter %03" PRIu32 "\n", chapter_number, chapter_number);
				}

				chapter_start += bluray_chapter.duration;

			}

			bd_chapter = NULL;

			if(p_bluray_json)
				printf("   ]\n");

		}

		if(p_bluray_json) {
			if(d_title_counter + 1 < d_num_titles)
				printf("  },\n");
			else
				printf("  }\n");
		}

	}

	if(p_bluray_json) {
		printf(" ]\n");
		printf("}\n");
	}

	bd_close(bd);
	bd = NULL;

	return 0;

}
