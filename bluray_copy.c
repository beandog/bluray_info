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
#include "config.h"
#include "libbluray/bluray.h"
#include "bluray_device.h"
#include "bluray_info.h"
#include "bluray_time.h"

/**
 * For reference:
 * Blu-ray spec - 1x drive can read at 32 Mbits / second, or 4.5 MBs
 * Fastest drives on the market right now (2018) are 12x, so limit to that max
 * My drive from 2009 is a 4x, so you can probably handle at least that speed :)
 * Default read speed is 128 Mbits / second, or 18 MBs
 **/
// #define BLURAY_OPTICAL_DRIVE_SPEED_1X 4718592
// #define BLURAY_OPTICAL_DEFAULT_DRIVE_SPEED 4

/**
 * bluray_copy reads 4 MB into buffer at a time -- a good mix between speed and
 * patience in watching the progress bar. :)
 */
#define BLURAY_COPY_BUFFER_SIZE 4194304

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
	int32_t buffer_size;
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
	bluray_copy.buffer_size = BLURAY_COPY_BUFFER_SIZE;
	bluray_copy.fd = -1;

	// Parse options and arguments
	bool opt_title_number = false;
	bool opt_playlist_number = false;
	bool opt_main_title = false;
	bool invalid_opt = false;
	bool quiet = false;
	uint32_t arg_title_number = 0;
	uint32_t arg_playlist_number = 0;
	uint32_t arg_angle_number = 1;
	uint32_t arg_first_chapter = 1;
	uint32_t arg_last_chapter = 99;
	uint32_t copy_chapters = 1;
	bool debug = false;
	const char *key_db_filename = NULL;
	char *token = NULL;
	int g_opt = 0;
	int g_ix = 0;
	opterr = 1;
	const char p_short_opts[] = "a:c:hk:mo:p:t:qVz";
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
				printf("bluray_copy %s\n", PACKAGE_VERSION);
				return 0;

			case 'z':
				debug = true;
				break;

			case '?':
				invalid_opt = true;
			case 'h':
				printf("bluray_copy - copy a Blu-ray title or playlist to a file\n");
				printf("\n");
				printf("Usage: bluray_copy [path] [options]\n");
				printf("\n");
				printf("Options:\n");
				printf("  -m, --main               Copy main title (default)\n");
				printf("  -t, --title <#>          Copy title number\n");
				printf("  -p, --playlist <#>       Copy playlist number\n");
				printf("  -c, --chapters <#>[-#]   Copy chapter number(s)\n");
				printf("\n");
				printf("Destination:\n");
				printf("  -o, --output <filename>  Save to filename (default: bluray_title_###.m2ts)\n");
				printf("      --output -           Write to stdout\n");
				printf("  -q, --quiet		   Don't display copy progress\n");
				printf("\n");
				printf("Other:\n");
				printf("  -k, --keydb <filename>   Location to KEYDB.CFG (default: ~/.config/aacs/KEYDB.cfg)\n");
				printf("  -a, --angle <#>          Video angle (default: 1)\n");
				printf("  -h, --help		   This output\n");
				printf("  -V, --version		   Version information\n");
				printf("\n");
				printf("Blu-ray path can be a device, a filename, or directory; default is %s\n", DEFAULT_BLURAY_DEVICE);
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

	if(debug)
		printf("Optical drive: %s\n", bluray_copy.optical_drive ? "yes" : "no");

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

	int64_t bd_bytes_read = 0;

	int64_t bytes_written = 0;
	int64_t total_bytes_written = 0;

	BLURAY_TITLE_CHAPTER *bd_chapter = NULL;

	bluray_copy.buffer_size = BLURAY_COPY_BUFFER_SIZE;

	if(debug)
		printf("Buffer size: %d\n", bluray_copy.buffer_size);

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
			chapter_stop_pos[ix] = bluray_title.size;
		else
			chapter_stop_pos[ix] = bd_chapter_pos(bd, ix + 1);
		if(debug && p_bluray_copy)
			printf("Chapter %02i: %015ld - %015ld = %015ld, Filesize: %lu MBs\n", ix + 1, chapter_start_pos[ix], chapter_stop_pos[ix], chapter_stop_pos[ix] - chapter_start_pos[ix], (chapter_stop_pos[ix] - chapter_start_pos[ix]) / 1024 / 1024);
	}

	// Fix chapter size for first
	chapter_start_pos[0] = 0;

	int64_t total_bytes = chapter_stop_pos[stop_chapter] - chapter_start_pos[start_chapter];
	if(debug && p_bluray_copy)
		printf("Copy total bytes: %ld\n", total_bytes);

	bool copy_success = true;

	struct bluray_chapter bluray_chapter;
	bluray_chapter.duration = 0;
	snprintf(bluray_chapter.length, BLURAY_DURATION + 1, "%s", "00:00:00.00");

	// Chapters are zero-indexed on Blu-rays
	for(ix = start_chapter; ix < stop_chapter + 1; ix++) {

		if(!copy_success)
			break;

		bd_chapter = &bd_title->chapters[ix];
		bluray_chapter.duration = bd_chapter->duration;
		bluray_duration_length(bluray_chapter.length, bd_chapter->duration);
		if(p_bluray_copy)
			printf("        Chapter: %02u, Length: %s, Filesize: %lu MBs\n", ix + 1, bluray_chapter.length, (uint64_t)((chapter_stop_pos[ix] - chapter_start_pos[ix]) / 1048576));

		seek_pos = bd_seek_chapter(bd, ix);
		stop_pos = chapter_stop_pos[ix];

		while(seek_pos < stop_pos && copy_success == true) {

			bluray_copy.buffer_size = BLURAY_COPY_BUFFER_SIZE;

			if(bluray_copy.buffer_size > (int32_t)(stop_pos - seek_pos)) {
				bluray_copy.buffer_size = (int32_t)(stop_pos - seek_pos);
			}

			// bd_read will read from the start of the title, not from
			// the seek position, meaning that the final filesize will
			// be larger than the total of end seek position minus
			// begin seek position (with the extra data at the front).
			bd_bytes_read = bd_read(bd, bluray_copy.buffer, bluray_copy.buffer_size);
			if(bd_bytes_read < 0 || bd_bytes_read == EOF)
				break;

			bytes_written = (int64_t)write(bluray_copy.fd, bluray_copy.buffer, bluray_copy.buffer_size);

			if(p_bluray_copy) {
				if(bytes_written != bd_bytes_read) {
					fprintf(stderr, "Read %ld bytes, but only wrote %ld ... out of disk space? Quitting.", bd_bytes_read, bytes_written);
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
