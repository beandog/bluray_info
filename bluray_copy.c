#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <getopt.h>
#include "libbluray/bluray.h"
#include "bluray_device.h"
#include "bluray_time.h"

#define BLURAY_INFO_VERSION "1.2.0"
#define BLURAY_ECC_BLOCK 64 * 1024 // 64k
#define BLURAY_BUFFER_SIZE ( BLURAY_ECC_BLOCK * 16 ) // 1 MB

struct bluray_info {
	char bluray_id[41];
	char bluray_title[33];
	uint32_t hdmv_titles;
	uint32_t bdj_titles;
	uint32_t unsupported_titles;
	uint32_t titles;
	uint32_t relevant_titles;
	uint32_t longest_title;
	int main_title;
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
	char length[12];
};

struct bluray_video {
	char codec[6];
	char format[6];
	char framerate[6];
	char aspect_ratio[5];
};

struct bluray_chapter {
	uint64_t ix;
	uint64_t duration;
	char length[12];
	uint64_t size;
	uint64_t size_mbs;
};

int main(int argc, char **argv) {

	uint32_t ix = 0;
	uint32_t d_num_titles = 0;
	int retval = 0;
	FILE *fd;

	// Parse options and arguments
	bool opt_title_number = false;
	uint32_t arg_title_number = 1;
	bool opt_playlist_number = false;
	uint32_t arg_playlist_number = 0;
	bool opt_main_title = true;
	bool opt_angle_number = false;
	uint32_t arg_angle_number = 1;
	bool opt_chapter_number = false;
	uint32_t arg_first_chapter = 1;
	uint32_t arg_last_chapter = 99;
	uint32_t copy_chapters = 1;
	bool debug = false;
	const char *key_db_filename = NULL;
	const char *output_filename = NULL;
	char *token = NULL;
	int g_opt = 0;
	int g_ix = 0;
	opterr = 1;
	const char p_short_opts[] = "a:c:hk:mo:p:t:z";
	struct option p_long_opts[] = {
		{ "angle", required_argument, NULL, 'a' },
		{ "chapters", required_argument, NULL, 'c' },
		{ "help", no_argument, NULL, 'h' },
		{ "keydb", required_argument, NULL, 'k' },
		{ "main", no_argument, NULL, 'm' },
		{ "output_filename", required_argument, NULL, 'o' },
		{ "playlist", required_argument, NULL, 'p' },
		{ "title", required_argument, NULL, 't' },
		{ "debug", no_argument, NULL, 'z' },
		{ 0, 0, 0, 0 }
	};
	while((g_opt = getopt_long(argc, argv, p_short_opts, p_long_opts, &g_ix)) != -1) {

		switch(g_opt) {

			case 'a':
				opt_angle_number = true;
				arg_angle_number = (unsigned int)strtoumax(optarg, NULL, 0);
				break;

			case 'c':
				opt_chapter_number = true;
				token = strtok(optarg, "-"); {
					if(strlen(token) > 2) {
						fprintf(stderr, "Chapter range must be between 1 and 99\n");
						return 1;
					}
					arg_first_chapter = (uint8_t)strtoumax(token, NULL, 0);
				}

				token = strtok(NULL, "-");
				if(token == NULL) {
					arg_last_chapter = arg_first_chapter;
				} else {
					if(strlen(token) > 2) {
						fprintf(stderr, "Chapter range must be between 1 and 99\n");
						return 1;
					}
					arg_last_chapter = (uint8_t)strtoumax(token, NULL, 0);
				}

				if(arg_first_chapter == 0)
					arg_first_chapter = 1;
				if(arg_last_chapter < arg_first_chapter)
					arg_last_chapter = arg_first_chapter;
				if(arg_first_chapter > arg_last_chapter)
					arg_first_chapter = arg_last_chapter;
				break;

			case 'k':
				key_db_filename = optarg;
				break;

			case 'm':
				opt_main_title = true;
				break;

			case 'o':
				output_filename = optarg;
				break;

			case 'p':
				opt_playlist_number = true;
				opt_title_number = false;
				opt_main_title = false;
				arg_playlist_number = (unsigned int)strtoumax(optarg, NULL, 0);
				break;

			case 't':
				opt_title_number = true;
				opt_playlist_number = false;
				opt_main_title = false;
				arg_title_number = (unsigned int)strtoumax(optarg, NULL, 0);
				break;

			case 'z':
				debug = true;
				break;

			case 'h':
			case '?':
				printf("bluray_copy %s - Copy a title track or playlist entry to an MPEG2-TS file\n\n", BLURAY_INFO_VERSION);
				printf("Usage: bluray_copy -o filename [options] [bluray device]\n\n");
				printf("Options:\n");
				printf("  -m, --main 		Copy main title (default)\n");
				printf("  -t, --title # 	Copy title number\n");
				printf("  -p, --playlist #	Copy playlist number\n");
				printf("  -c, --chapters #[-#]	Copy chapter number(s)\n");
				printf("  -a, --angle #		Video angle (default: 1)\n");
				printf("  -o, --output <file>	Save to filename\n");
				printf("  -k, --keydb		Location to KEYDB.CFG (default: use libaacs to look up)\n\n");
				printf("Blu-ray path can be a device filename, a file, or a directory.\n\n");
				printf("Examples:\n");
				printf("  bluray_copy -o video.m2ts /dev/bluray\n");
				printf("  bluray_copy -o video.m2ts bluray.iso\n");
				printf("  bluray_copy -o video.m2ts bluray/\n\n");
				printf("Default device filename is %s\n\n", DEFAULT_BLURAY_DEVICE);
				printf("For more information, see http://dvds.beandog.org/\n");
				return 0;

			case 0:
			default:
				break;

		}

	}

	if(output_filename == NULL) {
		printf("Need a an output filename (example: bluray_copy -o video.m2ts)\n");
		return 1;
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

	// Blu-ray
	struct bluray_info bluray_info;
	memset(bluray_info.bluray_id, '\0', sizeof(bluray_info.bluray_id));
	memset(bluray_info.bluray_title, '\0', sizeof(bluray_info.bluray_title));
	bluray_info.hdmv_titles = 0;
	bluray_info.bdj_titles = 0;
	bluray_info.unsupported_titles = 0;
	bluray_info.titles = 0;
	bluray_info.relevant_titles = 0;
	bluray_info.longest_title = 0;
	bluray_info.main_title = 0;

	if(bd_info->udf_volume_id)
		strncpy(bluray_info.bluray_title, bd_info->udf_volume_id, 33);
	bluray_info.titles = bd_info->num_titles;
	if(bd_info->libaacs_detected) {
		for(ix = 0; ix < 20; ix++) {
			sprintf(bluray_info.bluray_id + 2 * ix, "%02X", bd_info->disc_id[ix]);
		}
	}

	// Use "relevant titles" as index / reference
	bluray_info.relevant_titles = bd_get_titles(bd, TITLES_RELEVANT, 0);
	d_num_titles = (uint32_t)bluray_info.relevant_titles;

	// Select track passed as an argument
	if(opt_title_number) {
		if(arg_title_number == 0 || arg_title_number > d_num_titles) {
			printf("Could not open title %u, choose from 1 to %u\n", arg_title_number, d_num_titles);
			return 1;
		}
		retval = bd_select_title(bd, arg_title_number - 1);
		if(retval == 0) {
			printf("Could not open title %u\n", arg_title_number);
			return 1;
		}
	}

	if(opt_playlist_number) {
		retval = bd_select_playlist(bd, arg_playlist_number);
		if(retval == 0) {
			printf("Could not open playlist %u\n", arg_playlist_number);
			return 1;
		}
		arg_title_number = bd_get_current_title(bd) + 1;
	}

	bluray_info.main_title = bd_get_main_title(bd);

	if(opt_main_title) {
		arg_title_number = (uint32_t)bluray_info.main_title + 1;
	}

	if(bd_info->udf_volume_id && d_num_titles)
		printf("Disc Title: %s\n", bluray_info.bluray_title);
	
	struct bluray_title bluray_title;
	struct bluray_chapter bluray_chapter;

	retval = bd_select_title(bd, arg_title_number - 1);

	bd_title = bd_get_title_info(bd, arg_title_number - 1, 0);

	if(bd_title == NULL) {
		printf("Could not open title %u\n", arg_title_number);
		return 1;
	}
	
	bluray_title.ix = ix;
	bluray_title.playlist = bd_title->playlist;
	bluray_title.duration = bd_title->duration;
	bluray_title.size = bd_get_title_size(bd);
	bluray_title.size_mbs = bluray_title.size / 1024 / 1024;
	bluray_title.chapters = bd_title->chapter_count;
	bluray_title.clips = bd_title->clip_count;
	bluray_title.angles = bd_title->angle_count;
	strncpy(bluray_title.length, bluray_duration_length(bluray_title.duration), 12);

	if(arg_last_chapter > bluray_title.chapters)
		arg_last_chapter = bluray_title.chapters;
	if(arg_first_chapter > arg_last_chapter)
		arg_first_chapter = arg_last_chapter;
	copy_chapters = arg_last_chapter - arg_first_chapter;
	if(copy_chapters == 0)
		copy_chapters++;

	uint32_t start_chapter;
	uint32_t stop_chapter;
	
	start_chapter = arg_first_chapter - 1;
	stop_chapter = arg_last_chapter - 1;

	printf("Title: %03u, Playlist: %04u, Length: %s, Chapters: %02u, Video streams: %02u, Audio streams: %02u, Subtitles: %02u, Filesize: %05lu MBs\n", bluray_title.ix + 1, bluray_title.playlist, bluray_title.length, bluray_title.chapters, bluray_title.video_streams, bluray_title.audio_streams, bluray_title.pg_streams, bluray_title.size_mbs);

	retval = bd_select_angle(bd, arg_angle_number);
	if(retval < 0) {
		printf("Could not select angle # %u\n", arg_angle_number);
		return 1;
	}

	fd = fopen(output_filename, "wb");
	if(fd == NULL) {
		fprintf(stderr, "Could not open filename %s\n", output_filename);
		return 1;
	}

	int64_t seek_pos = 0;
	int64_t stop_pos = 0;

	int64_t buffer_size = BLURAY_BUFFER_SIZE;
	int64_t bd_bytes_read;

	size_t bytes_fwritten = 0;
	size_t total_bytes_written = 0;

	BLURAY_TITLE_CHAPTER *bd_chapter = NULL;

	unsigned char *buffer = NULL;
	buffer = (unsigned char *)calloc(1, (uint64_t)buffer_size * sizeof(unsigned char));
	if(buffer == NULL) {
		fprintf(stderr, "Couldn't allocate memory for copy buffer\n");
		return 1;
	}

	if(arg_last_chapter > bluray_title.chapters)
		arg_last_chapter--;

	int64_t chapter_start_pos[99] = { 0 };
	int64_t chapter_stop_pos[99] = { 0 };

	for(ix = 0; ix < bluray_title.chapters; ix++) {
		chapter_start_pos[ix] = bd_chapter_pos(bd, ix);
		if(ix + 1 == bluray_title.chapters)
			chapter_stop_pos[ix] = (int64_t)bluray_title.size - chapter_start_pos[0];
		else
			chapter_stop_pos[ix] = bd_chapter_pos(bd, ix + 1);
		if(debug)
			printf("Chapter %02i: %015lo - %015lo = %015lo, Filesize: %lu MBs\n", ix, chapter_start_pos[ix], chapter_stop_pos[ix], chapter_stop_pos[ix] - chapter_start_pos[ix], (chapter_stop_pos[ix] - chapter_start_pos[ix]) / 1024 / 1024);
	}

	int64_t total_bytes = chapter_stop_pos[stop_chapter] - chapter_start_pos[start_chapter];
	if(debug)
		printf("Copy total bytes: %lo\n", total_bytes);

	bool copy_success = true;

	// Chapters are zero-indexed on Blu-rays
	for(ix = start_chapter; ix < stop_chapter + 1; ix++) {

		if(!copy_success)
			break;

		bd_chapter = &bd_title->chapters[ix];
		bluray_chapter.duration = bd_chapter->duration;
		strncpy(bluray_chapter.length, bluray_duration_length(bd_chapter->duration), 12);
		printf("        Chapter: %02u, Length: %s, Filesize: %lu MBs\n", ix + 1, bluray_chapter.length, (chapter_stop_pos[ix] - chapter_start_pos[ix]) / 1024 / 1024);

		seek_pos = bd_seek_chapter(bd, ix);
		stop_pos = chapter_stop_pos[ix];

		while(seek_pos < stop_pos && copy_success == true) {

			buffer_size = BLURAY_BUFFER_SIZE;
			if(buffer_size > (stop_pos - seek_pos)) {
				buffer_size = stop_pos - seek_pos;
			}

			// bd_read will read from the start of the title, not from
			// the seek position, meaning that the final filesize will
			// be larger than the total of end seek position minus
			// begin seek position (with the extra data at the front).
			bd_bytes_read = bd_read(bd, buffer, (int32_t)buffer_size);
			if(bd_bytes_read < 0 || bd_bytes_read == EOF)
				break;

			// Can safely cast bd_bytes_read here to size_t because
			// if it is already less than zero, then we would have exited.
			bytes_fwritten = fwrite(buffer, 1, (size_t)bd_bytes_read, fd);

			if(bytes_fwritten != (size_t)bd_bytes_read) {
				fprintf(stderr, "Read %zu bytes, but only wrote %zu ... out of disk space? Quitting.", bd_bytes_read, bytes_fwritten);
				copy_success = false;
				retval = -1;
				break;
			}

			total_bytes_written += bytes_fwritten;

			printf("Progress: %lu%%\r", total_bytes_written * 100 / (uint64_t)total_bytes);
			fflush(stdout);

			seek_pos = (int64_t)bd_tell(bd);

		}

	}

	printf("\n");

	bd_free_title_info(bd_title);
	bd_title = NULL;

	bd_free_title_info(bd_title);
	bd_title = NULL;

	bd_close(bd);
	bd = NULL;

	retval = fclose(fd);
	if(retval < 0) {
		printf("Could not finish writing to %s\n", output_filename);
		return 1;
	}
	
	return 0;

}
