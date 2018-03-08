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
#include "libbluray/bluray.h"
#include "bluray_device.h"
#include "bluray_info.h"
#include "bluray_time.h"

#define BLURAY_COPY_VERSION BLURAY_INFO_VERSION

/**
 * Blu-ray spec - 1x drive can read at 32 Mbits / second, or 4.5 MBs
 * Fastest drives on the market right now (2018) are 12x, so limit to that max
 * My drive from 2009 is a 4x, so you can probably handle at least that speed :)
 * Default read speed is 128 Mbits / second, or 18 MBs
 **/
#define BLURAY_OPTICAL_DRIVE_SPEED_1X 4718592
#define BLURAY_OPTICAL_DEFAULT_DRIVE_SPEED 4
#define BLURAY_OPTICAL_BUFFER_SIZE ( BLURAY_OPTICAL_DRIVE_SPEED_1X * BLURAY_OPTICAL_DEFAULT_DRIVE_SPEED )
#define BLURAY_OPTICAL_MAX_DRIVE_SPEED 12

#define BLURAY_COPY_BUFFER_SIZE 1048576
#define BLURAY_CAT_BUFFER_SIZE 1048576

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

struct bluray_copy {
	char *filename;
	bool optical_drive;
	unsigned char *buffer;
	uint64_t buffer_size;
	int fd;
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

struct bluray_chapter {
	uint64_t duration;
	char length[BLURAY_DURATION + 1];
};

int main(int argc, char **argv) {

	uint32_t ix = 0;
	uint32_t d_num_titles = 0;
	int retval = 0;
	bool p_bluray_copy = true;
	bool p_bluray_cat = false;
	struct bluray_copy bluray_copy;
	bluray_copy.filename = NULL;
	bluray_copy.optical_drive = false;
	bluray_copy.buffer = NULL;
	bluray_copy.buffer_size = 1;
	bluray_copy.fd = -1;
	uint64_t buffer_size = 0;

	// Parse options and arguments
	bool opt_title_number = false;
	bool opt_playlist_number = false;
	bool opt_main_title = false;
	bool opt_drive_speed = false;
	bool invalid_opt = false;
	bool quiet = false;
	uint32_t arg_title_number = 0;
	uint32_t arg_playlist_number = 0;
	uint32_t arg_angle_number = 1;
	uint32_t arg_first_chapter = 1;
	uint32_t arg_last_chapter = 99;
	uint32_t copy_chapters = 1;
	uint32_t arg_drive_speed = 1;
	bool debug = false;
	const char *key_db_filename = NULL;
	char *token = NULL;
	int g_opt = 0;
	int g_ix = 0;
	opterr = 1;
	const char p_short_opts[] = "a:c:hk:mo:p:t:qVx:z";
	struct option p_long_opts[] = {
		{ "angle", required_argument, NULL, 'a' },
		{ "chapters", required_argument, NULL, 'c' },
		{ "help", no_argument, NULL, 'h' },
		{ "keydb", required_argument, NULL, 'k' },
		{ "main", no_argument, NULL, 'm' },
		{ "output", required_argument, NULL, 'o' },
		{ "playlist", required_argument, NULL, 'p' },
		{ "title", required_argument, NULL, 't' },
		{ "version", no_argument, NULL, 'V' },
		{ "quiet", no_argument, NULL, 'q' },
		{ "debug", no_argument, NULL, 'z' },
		{ "speed", required_argument, NULL, 'x' },
		{ 0, 0, 0, 0 }
	};
	while((g_opt = getopt_long(argc, argv, p_short_opts, p_long_opts, &g_ix)) != -1) {

		switch(g_opt) {

			case 'a':
				arg_angle_number = (unsigned int)strtoumax(optarg, NULL, 0);
				break;

			case 'c':
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
				bluray_copy.filename = optarg;
				if(strncmp("-", bluray_copy.filename, 1) == 0) {
					p_bluray_copy = false;
					p_bluray_cat = true;
				}
				break;

			case 'p':
				opt_playlist_number = true;
				arg_playlist_number = (unsigned int)strtoumax(optarg, NULL, 0);
				break;

			case 't':
				opt_title_number = true;
				arg_title_number = (unsigned int)strtoumax(optarg, NULL, 0);
				break;

			case 'q':
				quiet = true;
				break;

			case 'V':
				printf("bluray_copy %s - http://dvds.beandog.org/ - (c) 2018 Steve Dibb <steve.dibb@gmail.com>, licensed under GPL-2\n", BLURAY_INFO_VERSION);
				return 0;

			case 'x':
				opt_drive_speed = true;
				arg_drive_speed = (unsigned int)strtoumax(optarg, NULL, 0);
				if(((arg_drive_speed % 2) != 0) || arg_drive_speed > BLURAY_OPTICAL_MAX_DRIVE_SPEED) {
					fprintf(stderr, "Override drive speed must be a multiplier of 2, up to %d\n", BLURAY_OPTICAL_MAX_DRIVE_SPEED);
					return 1;
				}
				break;

			case 'z':
				debug = true;
				break;

			case '?':
				invalid_opt = true;
			case 'h':
				printf("bluray_copy %s - Copy a title track or playlist entry to an MPEG2-TS file\n\n", BLURAY_COPY_VERSION);
				printf("Usage: bluray_copy [options] [bluray path]\n\n");
				printf("Options:\n");
				printf("  -m, --main 		Copy main title (default)\n");
				printf("  -t, --title # 	Copy title number\n");
				printf("  -p, --playlist #	Copy playlist number\n");
				printf("  -c, --chapters #[-#]	Copy chapter number(s)\n");
				printf("  -a, --angle #		Video angle (default: 1)\n");
				printf("  -o, --output <file>	Save to filename (default: bluray_track_###.m2ts)\n");
				printf("  			Use - as filename to send to stdout\n");
				printf("  -k, --keydb		Location to KEYDB.CFG (default: use libaacs to look up)\n");
				printf("  -q, --quiet		Don't print progress output\n\n");
				printf("Other:\n");
				printf("  -h, --help		   This output\n");
				printf("  -V, --version		   Version information\n\n");
				printf("Blu-ray path can be a device filename, a file, or a directory; default is %s\n\n", DEFAULT_BLURAY_DEVICE);
				printf("Examples:\n");
				printf("  bluray_copy				 	# Copy main title to bluray_title_###.m2ts\n");
				printf("  bluray_copy -m			 	# Copy main title to bluray_title_###.m2ts\n");
				printf("  bluray_copy -t 51			 	# Copy title 51 to bluray_title_051.m2ts\n");
				printf("  bluray_copy -t 71 -c 8-12		 	# Copy title 71, chapters 8 to 12 to bluray_title_071.m2ts\n");
				printf("  bluray_copy -p				# Copy main playlist to bluray_playlist_#####.m2ts\n");
				printf("  bluray_copy -p 0				# Copy playlist 0 to bluray_playlist_00000.m2ts\n");
				printf("  bluray_copy -m -o bluray_main_title.m2ts	# Copy main title to bluray_main_title.m2ts\n");
				printf("  bluray_copy -o - > bluray_main_title.m2ts	# Stream output to bluray_main_title.m2ts\n");
				printf("  bluray_copy /dev/bluray			# Copy from device filename\n");
				printf("  bluray_copy /mnt/bluray			# Copy from mounted UDF volume or device\n");
				printf("  bluray_copy bluray.iso			# Copy from single ISO file\n");
				printf("  bluray_copy Videos/Blu-ray/			# Copy from directory backup\n");
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
		device_filename = realpath(argv[optind], NULL);
		bluray_copy.optical_drive = bluray_optical_drive(device_filename);
	} else {
		device_filename = DEFAULT_BLURAY_DEVICE;
		bluray_copy.optical_drive = true;
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

	// Select track passed as an argument
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
		if(bluray_copy.filename == NULL) {
			bluray_copy.filename = calloc(strlen("bluray_title_000.m2ts") + 1, sizeof(unsigned char));
			snprintf(bluray_copy.filename, strlen("bluray_title_000.m2ts") + 1, "%s%03u%s", "bluray_title_", bluray_title.ix + 1, ".m2ts");
		}
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
		if(bluray_copy.filename == NULL) {
			bluray_copy.filename = calloc(strlen("bluray_playlist_00000.m2ts") + 1, sizeof(unsigned char));
			snprintf(bluray_copy.filename, strlen("bluray_playlist_00000.m2ts") + 1, "%s%05u%s", "bluray_playlist_", bluray_title.playlist, ".m2ts");
		}
	} else {
		bluray_title.ix = (uint32_t)bluray_info.main_title;
		if(bd_select_title(bd, bluray_title.ix) == 0) {
			fprintf(stderr, "Could not open title %u\n", bluray_title.ix + 1);
			bd_close(bd);
			bd = NULL;
			return 1;
		}
		bd_title = bd_get_title_info(bd, bluray_title.ix, 0);
		if(bluray_copy.filename == NULL) {
			bluray_copy.filename = calloc(strlen("bluray_title_000.m2ts") + 1, sizeof(unsigned char));
			snprintf(bluray_copy.filename, strlen("bluray_title_000.m2ts") + 1, "%s%03u%s", "bluray_title_", bluray_title.ix + 1, ".m2ts");
		}
	}

	if(bd_title == NULL) {
		fprintf(stderr, "Could not get info for title %u\n", bluray_title.ix + 1);
		bd_close(bd);
		bd = NULL;
		if(bluray_copy.filename) {
			free(bluray_copy.filename);
			bluray_copy.filename = NULL;
		}
		return 1;
	}

	if(bd_info->udf_volume_id && d_num_titles && p_bluray_copy)
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

	if(p_bluray_copy && bluray_copy.optical_drive && opt_drive_speed && arg_drive_speed > 1)
		fprintf(stderr, "*** EASTER EGG ALERT! Setting drive speed manually to %ux may break your face since its untested. Attempting to copy at %u MBs/second. Have fun and good luck..... :D\n", arg_drive_speed, (BLURAY_OPTICAL_DRIVE_SPEED_1X * arg_drive_speed) / 1024 / 1024);


	if(p_bluray_copy)
		printf("Title: %03u, Playlist: %04u, Length: %s, Chapters: %02u, Video streams: %02u, Audio streams: %02u, Subtitles: %02u, Filesize: %05lu MBs\n", bluray_title.ix + 1, bluray_title.playlist, bluray_title.length, bluray_title.chapters, bluray_title.video_streams, bluray_title.audio_streams, bluray_title.pg_streams, bluray_title.size_mbs);

	retval = bd_select_angle(bd, arg_angle_number);
	if(retval < 0) {
		fprintf(stderr, "Could not select angle # %u\n", arg_angle_number);
		bd_free_title_info(bd_title);
		bd_title = NULL;
		bd_close(bd);
		bd = NULL;
		if(bluray_copy.filename) {
			free(bluray_copy.filename);
			bluray_copy.filename = NULL;
		}
		return 1;
	}

	if(p_bluray_copy) {
		bluray_copy.fd = open(bluray_copy.filename, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0644);
		if(!bluray_copy.fd) {
			fprintf(stderr, "Could not open filename %s\n", bluray_copy.filename);
			bd_free_title_info(bd_title);
			bd_title = NULL;
			bd_close(bd);
			bd = NULL;
			if(bluray_copy.filename) {
				free(bluray_copy.filename);
				bluray_copy.filename = NULL;
			}
			return 1;
		}
	} else if(p_bluray_cat) {
		bluray_copy.fd = 1;
	}

	int64_t seek_pos = 0;
	int64_t stop_pos = 0;

	if(p_bluray_copy)
		buffer_size = BLURAY_COPY_BUFFER_SIZE;
	else if(p_bluray_cat)
		buffer_size = BLURAY_CAT_BUFFER_SIZE;
	else
		buffer_size = BLURAY_COPY_BUFFER_SIZE;

	int64_t bd_bytes_read = 0;

	ssize_t bytes_written = 0;
	ssize_t total_bytes_written = 0;

	BLURAY_TITLE_CHAPTER *bd_chapter = NULL;

	if(bluray_copy.optical_drive && !opt_drive_speed)
		bluray_copy.buffer_size = BLURAY_OPTICAL_BUFFER_SIZE;
	else if(bluray_copy.optical_drive && opt_drive_speed && arg_drive_speed)
		bluray_copy.buffer_size = BLURAY_OPTICAL_DRIVE_SPEED_1X * arg_drive_speed;
	else if(p_bluray_cat)
		bluray_copy.buffer_size = BLURAY_CAT_BUFFER_SIZE;
	else
		bluray_copy.buffer_size = BLURAY_COPY_BUFFER_SIZE;

	bluray_copy.buffer = calloc(bluray_copy.buffer_size, sizeof(unsigned char));

	if(bluray_copy.buffer == NULL) {
		fprintf(stderr, "Couldn't allocate memory for copy buffer\n");
		bd_free_title_info(bd_title);
		bd_title = NULL;
		bd_close(bd);
		bd = NULL;
		if(bluray_copy.filename) {
			free(bluray_copy.filename);
			bluray_copy.filename = NULL;
		}
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
		if(debug && p_bluray_copy)
			printf("Chapter %02i: %015lo - %015lo = %015lo, Filesize: %lu MBs\n", ix, chapter_start_pos[ix], chapter_stop_pos[ix], chapter_stop_pos[ix] - chapter_start_pos[ix], (chapter_stop_pos[ix] - chapter_start_pos[ix]) / 1024 / 1024);
	}

	int64_t total_bytes = chapter_stop_pos[stop_chapter] - chapter_start_pos[start_chapter];
	if(debug && p_bluray_copy)
		printf("Copy total bytes: %lo\n", total_bytes);

	bool copy_success = true;

	struct bluray_chapter bluray_chapter;
	bluray_chapter.duration = 0;
	snprintf(bluray_chapter.length, BLURAY_DURATION + 1, "%s", "00:00:00.00");

	// Chapters are zero-indexed on Blu-rays
	for(ix = start_chapter; ix < stop_chapter + 1; ix++) {

		if(total_bytes == total_bytes_written)
			break;

		if(!copy_success)
			break;

		bd_chapter = &bd_title->chapters[ix];
		bluray_chapter.duration = bd_chapter->duration;
		bluray_duration_length(bluray_chapter.length, bd_chapter->duration);
		if(p_bluray_copy)
			printf("        Chapter: %02u, Length: %s, Filesize: %lu MBs\n", ix + 1, bluray_chapter.length, (chapter_stop_pos[ix] - chapter_start_pos[ix]) / 1024 / 1024);

		seek_pos = bd_seek_chapter(bd, ix);
		stop_pos = chapter_stop_pos[ix];

		while(seek_pos < stop_pos && copy_success == true) {

			if(total_bytes == total_bytes_written)
				break;

			buffer_size = bluray_copy.buffer_size;

			if(bluray_copy.buffer_size > (uint64_t)(stop_pos - seek_pos)) {
				buffer_size = (uint64_t)(stop_pos - seek_pos);
			}

			// bd_read will read from the start of the title, not from
			// the seek position, meaning that the final filesize will
			// be larger than the total of end seek position minus
			// begin seek position (with the extra data at the front).
			bd_bytes_read = bd_read(bd, bluray_copy.buffer, (int32_t)buffer_size);
			if(bd_bytes_read < 0 || bd_bytes_read == EOF)
				break;

			bytes_written = write(bluray_copy.fd, bluray_copy.buffer, (unsigned long)buffer_size);

			if(p_bluray_copy) {
				if(bytes_written != bd_bytes_read) {
					fprintf(stderr, "Read %zu bytes, but only wrote %zu ... out of disk space? Quitting.", bd_bytes_read, bytes_written);
					copy_success = false;
					retval = -1;
					break;
				}
			}

			total_bytes_written += bytes_written;

			if(!quiet) {
				fprintf(p_bluray_cat ? stderr : stdout, "Progress: %lu%% - %lu/%lu MBs\r", total_bytes_written * 100 / (ssize_t)total_bytes, total_bytes_written / 1024 / 1024, ((chapter_stop_pos[stop_chapter] - chapter_start_pos[start_chapter]) / 1024 / 1024));
				fflush(p_bluray_cat ? stderr : stdout);
			}

			if(total_bytes == total_bytes_written)
				break;

			seek_pos = (int64_t)bd_tell(bd);

		}

	}

	if(!quiet)
		fprintf(p_bluray_cat ? stderr : stdout, "\n");

	if(bluray_copy.buffer)
		bluray_copy.buffer = NULL;

	bd_free_title_info(bd_title);
	bd_title = NULL;

	bd_close(bd);
	bd = NULL;

	if(p_bluray_copy) {
		if(debug)
			fprintf(stderr, "Closing file ...");
		retval = close(bluray_copy.fd);
		if(debug)
			fprintf(stderr, "done\n");
		if(retval < 0) {
			fprintf(stderr, "Could not finish writing to %s\n", bluray_copy.filename);
			if(bluray_copy.filename)
				bluray_copy.filename = NULL;

			return 1;
		}
	}

	if(bluray_copy.filename)
		bluray_copy.filename = NULL;
	
	return 0;

}
