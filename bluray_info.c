#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <getopt.h>
#include "libbluray/bluray.h"
#include "libbluray/meta_data.h"
#include "bluray_info.h"
#include "bluray_device.h"
#include "bluray_audio.h"
#include "bluray_video.h"
#include "bluray_pgs.h"
#include "bluray_time.h"

struct bluray_info {
	char bluray_id[BLURAY_ID + 1];
	char bluray_title[BLURAY_TITLE + 1];
	char disc_name[BLURAY_DISC_NAME + 1];
	uint32_t titles;
	uint32_t relevant_titles;
	uint32_t bdinfo_titles;
	uint32_t bdj_titles;
	uint32_t hdmv_titles;
	uint32_t unsupported_titles;
	uint32_t longest_title;
	int main_title;
};

struct bluray_title {
	uint32_t ix;
	uint32_t number;
	uint32_t playlist;
	uint64_t duration;
	uint64_t seconds;
	uint64_t minutes;
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

struct bluray_video {
	char codec[BLURAY_VIDEO_CODEC + 1];
	char codec_name[BLURAY_VIDEO_CODEC_NAME + 1];
	char format[BLURAY_VIDEO_FORMAT + 1];
	double framerate;
	char aspect_ratio[BLURAY_VIDEO_ASPECT_RATIO + 1];
};

struct bluray_audio {
	char lang[BLURAY_AUDIO_LANG + 1];
	char codec[BLURAY_AUDIO_CODEC + 1];
	char codec_name[BLURAY_AUDIO_CODEC_NAME + 1];
	char format[BLURAY_AUDIO_FORMAT + 1];
	char rate[BLURAY_AUDIO_RATE + 1];
};

struct bluray_pgs {
	char lang[BLURAY_PGS_LANG + 1];
};

struct bluray_chapter {
	uint64_t start;
	uint64_t duration;
	char start_time[BLURAY_DURATION + 1];
	char length[BLURAY_DURATION + 1];
};

int main(int argc, char **argv) {

	uint32_t ix = 0;
	uint32_t stream_ix = 0;
	uint64_t chapter_ix = 0;
	uint64_t chapter_start = 0;
	uint32_t d_num_titles = 0;
	uint32_t d_first_ix = 0;
	uint32_t d_first_title = d_first_ix + 1;
	uint32_t d_title_counter = 0;
	int retval = 0;

	// Parse options and arguments
	bool p_bluray_info = true;
	bool p_bluray_json = false;
	bool p_bluray_id = false;
	bool p_bluray_disc_name = false;
	bool p_bluray_udf = false;
	bool p_bluray_xchap = false;
	bool d_title_number = false;
	uint32_t a_title_number = 0;
	bool d_playlist_number = false;
	uint32_t a_playlist_number = 0;
	bool d_main_title = false;
	bool d_video = false;
	bool d_audio = false;
	bool d_subtitles = false;
	bool d_chapters = false;
	bool d_duplicate_titles = false;
	uint32_t d_min_seconds = 0;
	uint32_t d_min_minutes = 0;
	uint32_t d_min_audio_streams = 0;
	uint32_t d_min_pg_streams = 0;
	bool d_quiet = false;
	bool invalid_opt = false;
	const char *key_db_filename = NULL;
	int g_opt = 0;
	int g_ix = 0;
	opterr = 1;
	const char p_short_opts[] = "acdghijk:mnp:qst:uvxAHSE:M:V";
	struct option p_long_opts[] = {
		{ "audio", no_argument, NULL, 'a' },
		{ "chapters", no_argument, NULL, 'c' },
		{ "duplicate", no_argument, NULL, 'd' },
		{ "help", no_argument, NULL, 'h' },
		{ "human", no_argument, NULL, 'H' },
		{ "id", no_argument, NULL, 'i' },
		{ "json", no_argument, NULL, 'j' },
		{ "keydb", required_argument, NULL, 'k' },
		{ "main", no_argument, NULL, 'm' },
		{ "name", no_argument, NULL, 'n' },
		{ "xchap", no_argument, NULL, 'g' },
		{ "playlist", required_argument, NULL, 'p' },
		{ "quiet", no_argument, NULL, 'q' },
		{ "subtitles", no_argument, NULL, 's' },
		{ "title", required_argument, NULL, 't' },
		{ "volname", no_argument, NULL, 'u' },
		{ "video", no_argument, NULL, 'v' },
		{ "all", no_argument, NULL, 'x' },
		{ "minutes", required_argument, NULL, 'M' },
		{ "seconds", required_argument, NULL, 'E' },
		{ "has-audio", no_argument, NULL, 'A' },
		{ "has-subtitles", no_argument, NULL, 'S' },
		{ "version", no_argument, NULL, 'V' },
		{ 0, 0, 0, 0 }
	};
	while((g_opt = getopt_long(argc, argv, p_short_opts, p_long_opts, &g_ix)) != -1) {

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

			case 'd':
				d_duplicate_titles = true;
				break;

			case 'E':
				d_min_seconds = (unsigned int)strtoumax(optarg, NULL, 0);
				break;

			case 'g':
				p_bluray_info = false;
				p_bluray_xchap = true;
				break;

			case 'H':
				break;

			case 'i':
				p_bluray_info = false;
				p_bluray_id = true;
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
				d_min_minutes = (unsigned int)strtoumax(optarg, NULL, 0);
				break;

			case 'n':
				p_bluray_info = false;
				p_bluray_disc_name = true;
				break;

			case 'p':
				d_title_number = false;
				d_playlist_number = true;
				d_main_title = false;
				a_playlist_number = (unsigned int)strtoumax(optarg, NULL, 0);
				break;

			case 'q':
				d_quiet = true;
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
				a_title_number = (unsigned int)strtoumax(optarg, NULL, 0);
				break;

			case 'u':
				p_bluray_info = false;
				p_bluray_udf = true;
				break;

			case 'V':
				printf("bluray_info %s - (c) 2018 Steve Dibb <steve.dibb@gmail.com>, licensed under GPL-2\n", BLURAY_INFO_VERSION);
				return 0;

			case 'v':
				d_video = true;
				break;

			case 'x':
				d_video = true;
				d_audio = true;
				d_chapters = true;
				d_subtitles = true;
				break;

			case '?':
				invalid_opt = true;
			case 'h':
				printf("bluray_info %s - display information about a Blu-ray disc\n", BLURAY_INFO_VERSION);
				printf("\n");
				printf("Usage: bluray_info [path] [options]\n");
				printf("\n");
				printf("Options:\n");
				printf("  -m, --main  	 	   Limit to main title (default: all)\n");
				printf("  -t, --title <number>     Limit to selected title\n");
				printf("  -p, --playlist <number>  Limit to selected playlist\n");
				printf("  -k, --keydb <filename>   Location to KEYDB.CFG (default: ~/.config/aacs/KEYDB.cfg)\n");
				printf("\n");
				printf("Extra information:\n");
				printf("  -v, --video              Display video streams\n");
				printf("  -a, --audio              Display audio streams\n");
				printf("  -s, --subtitles          Display subtitles\n");
				printf("  -c, --chapters           Display chapters\n");
				printf("  -x, --all                Display all\n");
				printf("\n");
				printf("Formatting:\n");
				printf("  -H, --human		   Output as human readable (default)\n");
				printf("  -j, --json               Output as JSON\n");
				printf("\n");
				printf("Narrow results:\n");
				printf("  -A, --has-audio          Title has audio\n");
				printf("  -S, --has-subtitles      Title has subtitles\n");
				printf("  -E, --seconds <number>   Title has minimum number of seconds\n");
				printf("  -M, --minutes <number>   Title has minimum number of minutes\n");
				printf("\n");
				printf("Limited information:\n");
				printf("  -i, --id		   Display ID only\n");
				printf("  -g, --xchap		   Display title chapters in export format for ogmerge or mkvmerge\n");
				printf("  -n, --name		   Display disc name only\n");
				printf("  -u, --volname		   Display UDF volume name only (path must be device or filename)\n");
				printf("  -q, --quiet		   Do not display disc title header and main title footer\n");
				printf("\n");
				printf("Other:\n");
				printf("  -d, --duplicate	   Include duplicate titles and clips\n");
				printf("  -h, --help		   This output\n");
				printf("  -V, --version		   Version information\n");
				printf("\n");
				printf("Blu-ray path can be a device, a filename, or directory (default: %s)\n", DEFAULT_BLURAY_DEVICE);
				printf("\n");
				printf("Examples:\n");
				printf("  bluray_info\n");
				printf("  bluray_info /dev/sr0\n");
				printf("  bluray_info bluray.iso\n");
				printf("  bluray_info Blu-ray/\n");
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

	// Fetch info
	const BLURAY_DISC_INFO *bd_info = NULL;

	bd_info = bd_get_disc_info(bd);

	if(bd_info == NULL) {
		printf("Could not get Blu-ray disc info\n");
		return 1;
	}

	BLURAY_TITLE_INFO *bd_title = NULL;
	BLURAY_STREAM_INFO *bd_stream = NULL;
	BLURAY_TITLE_CHAPTER *bd_chapter = NULL;

	// Blu-ray
	struct bluray_info bluray_info;
	memset(bluray_info.bluray_id, '\0', sizeof(bluray_info.bluray_id));
	memset(bluray_info.bluray_title, '\0', sizeof(bluray_info.bluray_title));
	memset(bluray_info.disc_name, '\0', sizeof(bluray_info.disc_name));
	bluray_info.titles = 0;
	bluray_info.relevant_titles = 0;
	bluray_info.bdinfo_titles = 0;
	bluray_info.bdj_titles = 0;
	bluray_info.hdmv_titles = 0;
	bluray_info.unsupported_titles = 0;
	bluray_info.longest_title = 0;
	bluray_info.main_title = 1;

	const struct meta_dl *bluray_meta = NULL;
	bluray_meta = bd_get_meta(bd);

	if(bluray_meta != NULL)
		strncpy(bluray_info.disc_name, bluray_meta->di_name, BLURAY_DISC_NAME + 1);

	if(bd_info->udf_volume_id)
		strncpy(bluray_info.bluray_title, bd_info->udf_volume_id, BLURAY_TITLE + 1);

	if(p_bluray_disc_name) {
		printf("%s\n", bluray_info.disc_name);
		return 0;
	}

	if(p_bluray_udf) {
		printf("%s\n", bluray_info.bluray_title);
		return 0;
	}

	if(bd_info->libaacs_detected) {
		for(ix = 0; ix < 20; ix++) {
			sprintf(bluray_info.bluray_id + 2 * ix, "%02X", bd_info->disc_id[ix]);
		}
	}

	if(p_bluray_id) {
		printf("%s\n", bluray_info.bluray_id);
		return 0;
	}

	// Titles, Indexes and Playlists
	//
	// libbluray has a "title" which is a really an index it uses to list the
	// playlists based on the type queried. It has stuck as the "title index" for
	// media players (mplayer, mpv). There are more titles than just the relevant
	// ones, so fetch all of them, then map the "relevant" ones, so we can display
	// both (or keep track of them).
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

	bluray_info.relevant_titles = bd_get_titles(bd, TITLES_RELEVANT, 0);
	bluray_info.titles = bluray_info.relevant_titles;
	if(d_duplicate_titles) {
		bluray_info.titles = bd_get_titles(bd, TITLES_ALL, 0);
	}
	d_num_titles = (uint32_t)bluray_info.titles;

	// libbluray has 'num_titles' and 'bd_get_titles()' both of which return different
	// numbers. Keep track of them both.
	bluray_info.bdinfo_titles = bd_info->num_titles;
	bluray_info.bdj_titles = bd_info->num_bdj_titles;
	bluray_info.hdmv_titles = bd_info->num_hdmv_titles;
	bluray_info.unsupported_titles = bd_info->num_unsupported_titles;

	// Select track passed as an argument
	if(d_title_number) {
		if(a_title_number > d_num_titles) {
			printf("Could not open title %u, choose from 0 to %u\n", a_title_number, d_num_titles - 1);
			return 1;
		}
		retval = bd_select_title(bd, a_title_number - 1);
		if(retval == 0) {
			printf("Could not open title %u\n", a_title_number);
			return 1;
		}
		d_first_ix = a_title_number - 1;
		d_first_title = d_first_ix + 1;
		d_num_titles = 1;
	}

	if(d_playlist_number) {
		retval = bd_select_playlist(bd, a_playlist_number);
		if(retval == 0) {
			printf("Could not open playlist %u\n", a_playlist_number);
			return 1;
		}
		d_first_ix = bd_get_current_title(bd);
		d_first_title = d_first_ix + 1;
		d_num_titles = 1;
	}

	bluray_info.main_title = bd_get_main_title(bd);

	if(d_main_title) {
		d_first_ix = (uint32_t)bluray_info.main_title;
		d_first_title = d_first_ix + 1;
		d_num_titles = 1;
	}

	if(p_bluray_xchap) {
		if(d_title_number)
			d_first_ix = a_title_number - 1;
		 else
			d_first_ix = (uint32_t)bluray_info.main_title;
		d_first_title = d_first_ix + 1;
		d_num_titles = 1;
	}

	if(p_bluray_info && d_quiet == false)
		printf("Disc title: %s\n", bluray_info.disc_name);

	if(p_bluray_json) {

		// Find the longest title
		uint64_t max_duration = 0;
		uint32_t main_playlist = 0;
		uint32_t longest_title = 1;
		uint32_t longest_playlist = 0;
		for(ix = 0; ix < bluray_info.relevant_titles; ix++) {
			bd_title = bd_get_title_info(bd, ix, 0);
			if(bd_title == NULL) {
				continue;
			}

			if(bd_title->idx == (uint32_t)bluray_info.main_title)
				main_playlist = bd_title->playlist;

			if(bd_title->duration > max_duration) {
				longest_title = ix;
				longest_playlist = bd_title->playlist;
				max_duration = bd_title->duration;
			}
		}

		printf("{\n");
		printf(" \"bluray\": {\n");
		printf("  \"disc title\": \"%s\",\n", bluray_info.disc_name);
		printf("  \"disc id\": \"%s\",\n", bluray_info.bluray_id);
		printf("  \"udf title\": \"%s\",\n", bluray_info.bluray_title);
		printf("  \"first play supported\": %s,\n", bd_info->first_play_supported ? "true" : "false");
		printf("  \"top menu supported\": %s,\n", bd_info->first_play_supported ? "true" : "false");
		printf("  \"provider data\": \"%s\",\n", bd_info->provider_data);
		printf("  \"3D content\": %s,\n", bd_info->content_exist_3D ? "true" : "false");
		printf("  \"initial mode\": \"%s\",\n", bd_info->initial_output_mode_preference ? "3D" : "2D");
		printf("  \"titles\": %u,\n", bluray_info.titles);
		printf("  \"relevant titles\": %u,\n", bluray_info.relevant_titles);
		printf("  \"bdinfo titles\": %u,\n", bd_info->num_titles);
		printf("  \"bdj titles\": %u,\n", bd_info->num_bdj_titles);
		printf("  \"hdmv titles\": %u,\n", bd_info->num_hdmv_titles);
		printf("  \"unsupported titles\": %u,\n", bd_info->num_unsupported_titles);
		printf("  \"main title\": %u,\n", bluray_info.main_title + 1);
		printf("  \"main playlist\": %u,\n", main_playlist);
		printf("  \"longest title\": %u,\n", longest_title + 1);
		printf("  \"longest playlist\": %u\n", longest_playlist);
		printf(" },\n");

	}

	struct bluray_title bluray_title;
	bluray_title.ix = 0;
	bluray_title.number = bluray_title.ix + 1;
	bluray_title.playlist = 0;
	bluray_title.duration = 0;
	bluray_title.seconds = 0;
	bluray_title.minutes = 0;
	bluray_title.size = 0;
	bluray_title.size_mbs = 0;
	bluray_title.chapters = 0;
	bluray_title.clips = 0;
	bluray_title.angles = 0;
	bluray_title.video_streams = 0;
	bluray_title.audio_streams = 0;
	bluray_title.pg_streams = 0;
	snprintf(bluray_title.length, BLURAY_DURATION + 1, "%s", "00:00:00.00");

	uint32_t bluray_highest_playlist = 0;

	struct bluray_video bluray_video;
	memset(bluray_video.codec, '\0', sizeof(bluray_video.codec));
	memset(bluray_video.codec_name, '\0', sizeof(bluray_video.codec_name));
	memset(bluray_video.format, '\0', sizeof(bluray_video.format));
	bluray_video.framerate = 0;
	memset(bluray_video.aspect_ratio, '\0', sizeof(bluray_video.aspect_ratio));

	struct bluray_audio bluray_audio;
	memset(bluray_audio.lang, '\0', sizeof(bluray_audio.lang));
	memset(bluray_audio.codec, '\0', sizeof(bluray_audio.codec));
	memset(bluray_audio.codec_name, '\0', sizeof(bluray_audio.codec_name));
	memset(bluray_audio.format, '\0', sizeof(bluray_audio.format));
	memset(bluray_audio.rate, '\0', sizeof(bluray_audio.rate));

	struct bluray_pgs bluray_pgs;
	memset(bluray_pgs.lang, '\0', sizeof(bluray_pgs.lang));

	struct bluray_chapter bluray_chapter;
	bluray_chapter.duration = 0;
	snprintf(bluray_chapter.length, BLURAY_DURATION + 1, "%s", "00:00:00.00");

	if(p_bluray_json)
		printf(" \"titles\": [\n");

	for(ix = d_first_ix; d_title_counter < d_num_titles; ix++, d_title_counter++) {

		retval = bd_select_title(bd, ix);
		if(retval == 0)
			continue;

		bd_title = bd_get_title_info(bd, ix, 0);

		if(bd_title == NULL)
			continue;

		bluray_title.ix = ix;
		bluray_title.number = bluray_title.ix + 1;
		bluray_title.playlist = bd_title->playlist;
		bluray_title.duration = bd_title->duration;
		bluray_title.seconds = bluray_duration_seconds(bluray_title.duration);
		bluray_title.minutes = bluray_duration_minutes(bluray_title.duration);
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
		} else {
			bluray_title.video_streams = 0;
			bluray_title.audio_streams = 0;
			bluray_title.pg_streams = 0;
		}

		bluray_highest_playlist = ((bluray_title.playlist > bluray_highest_playlist) ? bluray_title.playlist : bluray_highest_playlist);

		if(p_bluray_info) {

			if(!(bluray_title.seconds >= d_min_seconds && bluray_title.minutes >= d_min_minutes && bluray_title.audio_streams >= d_min_audio_streams && bluray_title.pg_streams >= d_min_pg_streams)) {
				bd_stream = NULL;
				continue;
			}

			printf("Title: %03u, Playlist: %04u, Length: %s, Chapters: %03u, Video streams: %02u, Audio streams: %02u, Subtitles: %02u, Filesize: %05lu MB\n", bluray_title.number, bluray_title.playlist, bluray_title.length, bluray_title.chapters, bluray_title.video_streams, bluray_title.audio_streams, bluray_title.pg_streams, bluray_title.size_mbs);

		}

		if(p_bluray_json) {

			if(!(bluray_title.seconds >= d_min_seconds && bluray_title.minutes >= d_min_minutes && bluray_title.audio_streams >= d_min_audio_streams && bluray_title.pg_streams >= d_min_pg_streams)) {
				bd_stream = NULL;
				continue;
			}

			printf("  {\n");
			printf("   \"title\": %u,\n", bluray_title.number);
			printf("   \"playlist\": %u,\n", bluray_title.playlist);
			printf("   \"length\": \"%s\",\n", bluray_title.length);
			printf("   \"msecs\": %lu,\n", bluray_title.duration / 900);
			printf("   \"filesize\": %lu,\n", bluray_title.size);

		}

		// Blu-ray video streams
		if((p_bluray_info && d_video) || p_bluray_json) {

			if(p_bluray_json)
				printf("   \"video\": [\n");

			for(stream_ix = 0; stream_ix < bluray_title.video_streams; stream_ix++) {

				bd_stream = &bd_title->clips[0].video_streams[stream_ix];

				if(bd_stream == NULL)
					continue;

				bluray_video_codec(bluray_video.codec, bd_stream->coding_type);
				bluray_video_codec_name(bluray_video.codec_name, bd_stream->coding_type);
				bluray_video_format(bluray_video.format, bd_stream->format);
				bluray_video.framerate = bluray_video_framerate(bd_stream->rate);
				bluray_video_aspect_ratio(bluray_video.aspect_ratio, bd_stream->aspect);

				if(p_bluray_info && d_video) {
					printf("	Video: %02u, Format: %s, Aspect ratio: %s, FPS: %.02f, Codec: %s\n", stream_ix + 1, bluray_video.format, bluray_video.aspect_ratio, bluray_video.framerate, bluray_video.codec);
				}

				if(p_bluray_json) {
					printf("    {\n");
					printf("     \"track\": %u,\n", stream_ix + 1);
					printf("     \"stream\": \"0x%x\",\n", bd_stream->pid);
					printf("     \"format\": \"%s\",\n", bluray_video.format);
					printf("     \"aspect ratio\": \"%s\",\n", bluray_video.aspect_ratio);
					printf("     \"framerate\": %.02f,\n", bluray_video.framerate);
					printf("     \"codec\": \"%s\",\n", bluray_video.codec);
					printf("     \"codec name\": \"%s\"\n", bluray_video.codec_name);
					if(stream_ix + 1 < bluray_title.video_streams)
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

			for(stream_ix = 0; stream_ix < bluray_title.audio_streams; stream_ix++) {

				bd_stream = &bd_title->clips[0].audio_streams[stream_ix];

				if(bd_stream == NULL)
					continue;

				bluray_audio_lang(bluray_audio.lang, bd_stream->lang);
				bluray_audio_codec(bluray_audio.codec, bd_stream->coding_type);
				bluray_audio_codec_name(bluray_audio.codec_name, bd_stream->coding_type);
				bluray_audio_format(bluray_audio.format, bd_stream->format);
				bluray_audio_rate(bluray_audio.rate, bd_stream->rate);

				if(p_bluray_info && d_audio) {
					printf("	Audio: %02u, Language: %s, Codec: %s, Format: %s, Rate: %s\n", stream_ix + 1, bluray_audio.lang, bluray_audio.codec, bluray_audio.format, bluray_audio.rate);
				}

				if(p_bluray_json) {
					printf("    {\n");
					printf("     \"track\": %u,\n", stream_ix + 1);
					printf("     \"stream\": \"0x%x\",\n", bd_stream->pid);
					printf("     \"language\": \"%s\",\n", bluray_audio.lang);
					printf("     \"codec\": \"%s\",\n", bluray_audio.codec);
					printf("     \"codec name\": \"%s\",\n", bluray_audio.codec_name);
					printf("     \"format\": \"%s\",\n", bluray_audio.format);
					printf("     \"rate\": \"%s\"\n", bluray_audio.rate);
					if(stream_ix + 1 < bluray_title.audio_streams)
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

			for(stream_ix = 0; stream_ix < bluray_title.pg_streams; stream_ix++) {

				bd_stream = &bd_title->clips[0].pg_streams[stream_ix];

				bluray_pgs_lang(bluray_pgs.lang, bd_stream->lang);

				if(bd_stream == NULL)
					continue;

				if(p_bluray_info && d_subtitles) {
					printf("	Subtitle: %02u, Language: %s\n", stream_ix + 1, bluray_pgs.lang);
				}

				if(p_bluray_json) {
					printf("    {\n");
					printf("     \"track\": %u,\n", stream_ix + 1);
					printf("     \"stream\": \"0x%x\",\n", bd_stream->pid);
					printf("     \"language\": \"%s\"\n", bluray_pgs.lang);
					if(stream_ix + 1 < bluray_title.pg_streams)
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

				bd_chapter = &bd_title->chapters[chapter_ix];

				if(bd_chapter == NULL)
					continue;

				bluray_chapter.start = chapter_start;
				bluray_chapter.duration = bd_chapter->duration;
				bluray_duration_length(bluray_chapter.length, bd_chapter->duration);
				bluray_duration_length(bluray_chapter.start_time, bluray_chapter.start);

				if(p_bluray_info && d_chapters) {
					printf("	Chapter: %03lu, Start: %s, Length: %s\n", chapter_ix + 1, bluray_chapter.start_time, bluray_chapter.length);
				}

				if(p_bluray_json) {
					printf("    {\n");
					printf("     \"chapter\": %lu,\n", chapter_ix + 1);
					printf("     \"start time\": \"%s\",\n", bluray_chapter.start_time);
					printf("     \"length\": \"%s\",\n", bluray_chapter.length);
					printf("     \"start\": %lu,\n", bluray_chapter.start / 900);
					printf("     \"duration\": %lu\n", bd_chapter->duration / 900);
					if(chapter_ix + 1 < bluray_title.chapters)
						printf("    },\n");
					else
						printf("    }\n");
				}

				if(p_bluray_xchap && ix == d_first_ix) {
					printf("CHAPTER%03lu=%s\n", chapter_ix + 1, bluray_chapter.start_time);
					printf("CHAPTER%03luNAME=Chapter %03lu\n", chapter_ix + 1, chapter_ix + 1);
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

		bd_free_title_info(bd_title);
		bd_title = NULL;

	}

	if(p_bluray_json) {
		printf(" ]\n");
		printf("}\n");
	}

	if(p_bluray_info && !d_title_number && !d_main_title && d_num_titles != 1 && d_quiet == false)
		printf("Main title: %i\n", bluray_info.main_title + 1);

	bd_free_title_info(bd_title);
	bd_title = NULL;

	bd_close(bd);
	bd = NULL;

	return 0;

}
