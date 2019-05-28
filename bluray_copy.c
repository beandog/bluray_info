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
#include "config.h"
#include "libbluray/bluray.h"
#include "bluray_device.h"
#include "bluray_info.h"
#include "bluray_time.h"

// libbluray packet size
#define BLURAY_COPY_BUFFER_SIZE 192

struct bluray_copy {
	char *filename;
	int fd;
	int64_t size;
	double size_mbs;
};

struct bluray_title {
	uint32_t ix;
	uint32_t number;
	uint32_t playlist;
	uint64_t duration;
	int64_t size;
	double size_mbs;
	uint32_t chapters;
	uint32_t clips;
	uint8_t angles;
	uint8_t video_streams;
	uint8_t audio_streams;
	uint8_t pg_streams;
	char length[12];
};

struct bluray_chapter {
	uint64_t duration;
	uint64_t start;
	char start_time[12];
	char length[12];
	int64_t range[2];
	int64_t size;
	double size_mbs;
};

int main(int argc, char **argv) {

	FILE *io = stdout;
	uint32_t ix = 0;
	uint32_t chapter_ix = 0;
	uint32_t chapter_number = chapter_ix + 1;
	uint32_t d_num_titles = 0;
	int retval = 0;
	bool p_bluray_copy = true;
	bool p_bluray_cat = false;
	struct bluray_copy bluray_copy;
	bluray_copy.filename = NULL;
	bluray_copy.fd = -1;
	bluray_copy.size = 0;
	bluray_copy.size_mbs = 0;

	// Parse options and arguments
	bool opt_title_number = false;
	bool opt_playlist_number = false;
	bool opt_main_title = false;
	bool invalid_opt = false;
	long int arg_number = 0;
	uint32_t arg_title_number = 0;
	uint32_t arg_playlist_number = 0;
	uint8_t arg_angle_number = 1;
	bool debug = false;
	const char *key_db_filename = NULL;

	// Chapter range selection
	uint32_t arg_chapter_numbers[2];
	arg_chapter_numbers[0] = 1;
	arg_chapter_numbers[1] = 0;

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
		{ "debug", no_argument, NULL, 'z' },
		{ "speed", required_argument, NULL, 'x' },
		{ 0, 0, 0, 0 }
	};
	while((g_opt = getopt_long(argc, argv, p_short_opts, p_long_opts, &g_ix)) != -1) {

		switch(g_opt) {

			case 'a':
				arg_number = strtol(optarg, NULL, 10);
				if(arg_number < 1) {
					arg_angle_number = 1;
				} else {
					arg_angle_number = (uint8_t)arg_number;
				}
				arg_number = 0;
				break;

			case 'c':
				token = strtok(optarg, "-");

				arg_number = strtol(token, NULL, 10);
				if(arg_number > 0) {
					arg_chapter_numbers[0] = (uint32_t)arg_number;
				}

				token = strtok(NULL, "-");
				if(token == NULL) {
					arg_chapter_numbers[1] = arg_chapter_numbers[0];
				} else {
					arg_number = strtol(token, NULL, 10);
					if(arg_number > 0) {
						arg_chapter_numbers[1] = (uint32_t)arg_number;
					}
				}
				arg_number = 0;
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
					io = stderr;
				}
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
				printf("\n");
				printf("Other:\n");
				printf("  -k, --keydb <filename>   Location to KEYDB.cfg (default: ~/.config/aacs/KEYDB.cfg)\n");
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
	BLURAY_TITLE_CHAPTER *bd_chapter = NULL;

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
		strncpy(bluray_info.bluray_title, bd_info->udf_volume_id, BLURAY_TITLE_STRLEN);
	bluray_info.titles = bd_info->num_titles;
	if(bd_info->libaacs_detected) {
		for(ix = 0; ix < 20; ix++) {
			sprintf(bluray_info.bluray_id + 2 * ix, "%02X", bd_info->disc_id[ix]);
		}
	}

	// Use relevant titles as index / reference
	bluray_info.titles = bd_get_titles(bd, TITLES_RELEVANT, 0);
	d_num_titles = (uint32_t)bluray_info.titles;
	bluray_info.main_title = (int32_t)bd_get_main_title(bd);

	struct bluray_title bluray_title;
	bluray_title.ix = 0;
	bluray_title.number = 1;
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
	strcpy(bluray_title.length, "00:00:00.00");

	// Select title passed as an argument
	if(opt_title_number) {
		bluray_title.number = arg_title_number;
		bluray_title.ix = bluray_title.number - 1;
		if(bluray_title.number > d_num_titles) {
			fprintf(stderr, "Could not open title %" PRIu32 ", choose from 1 to %" PRIu32 "\n", bluray_title.number, d_num_titles);
			bd_close(bd);
			bd = NULL;
			return 1;
		}
		if(bd_select_title(bd, bluray_title.ix) == 0) {
			fprintf(stderr, "Could not open title %u\n", bluray_title.number);
			bd_close(bd);
			bd = NULL;
			return 1;
		}
		bd_title = bd_get_title_info(bd, bluray_title.ix, 0);
		bluray_title.playlist = bd_title->playlist;
		if(bluray_copy.filename == NULL) {
			bluray_copy.filename = calloc(strlen("bluray_title_000.m2ts") + 1, sizeof(unsigned char));
			snprintf(bluray_copy.filename, strlen("bluray_title_000.m2ts") + 1, "%s%03" PRIu32 "%s", "bluray_title_", bluray_title.number, ".m2ts");
		}
	} else if(opt_playlist_number) {
		bluray_title.playlist = arg_playlist_number;
		if(bd_select_playlist(bd, bluray_title.playlist) == 0) {
			fprintf(stderr, "Could not open playlist %" PRIu32 "\n", bluray_title.playlist);
			bd_close(bd);
			bd = NULL;
			return 1;
		}
		bluray_title.ix = bd_get_current_title(bd);
		bd_title = bd_get_title_info(bd, bluray_title.ix, 0);
		if(bluray_copy.filename == NULL) {
			bluray_copy.filename = calloc(strlen("bluray_playlist_00000.m2ts") + 1, sizeof(unsigned char));
			snprintf(bluray_copy.filename, strlen("bluray_playlist_00000.m2ts") + 1, "%s%05" PRIu32 "%s", "bluray_playlist_", bluray_title.playlist, ".m2ts");
		}
	} else {
		bluray_title.ix = (uint32_t)bluray_info.main_title;
		if(bd_select_title(bd, bluray_title.ix) == 0) {
			fprintf(stderr, "Could not open title %" PRIu32 "\n", bluray_title.number);
			bd_close(bd);
			bd = NULL;
			return 1;
		}
		bd_title = bd_get_title_info(bd, bluray_title.ix, 0);
		if(bluray_copy.filename == NULL) {
			bluray_copy.filename = calloc(strlen("bluray_title_000.m2ts") + 1, sizeof(unsigned char));
			snprintf(bluray_copy.filename, strlen("bluray_title_000.m2ts") + 1, "%s%03" PRIu32 "%s", "bluray_title_", bluray_title.number, ".m2ts");
		}
	}

	if(bd_title == NULL) {
		fprintf(stderr, "Could not get info for title %" PRIu32 "\n", bluray_title.number);
		bd_close(bd);
		bd = NULL;
		if(bluray_copy.filename) {
			free(bluray_copy.filename);
			bluray_copy.filename = NULL;
		}
		return 1;
	}

	// Check for out of bounds chapter selection
	bluray_title.chapters = bd_title->chapter_count;

	// Handle no argument given for last chapter
	if(arg_chapter_numbers[1] == 0)
		arg_chapter_numbers[1] = bluray_title.chapters;
	// Handle first chapter argument being past last chapter argument
	if(arg_chapter_numbers[0] > arg_chapter_numbers[1])
		arg_chapter_numbers[0] = arg_chapter_numbers[1];
	// Handle chapter selection for first or last being out of bounds
	if(arg_chapter_numbers[0] > bluray_title.chapters || arg_chapter_numbers[1] > bluray_title.chapters) {
		fprintf(stderr, "Chapter selection is out of bounds, select between 1 and %" PRIu32 "\n", bluray_title.chapters);
		return 1;
	}
	// Finally set the actual zero-based chapter range indexes
	uint32_t chapters_range[2];
	chapters_range[0] = arg_chapter_numbers[0] - 1;
	chapters_range[1] = arg_chapter_numbers[1] - 1;

	// Pouplate title
	bluray_title.duration = bd_title->duration;
	bluray_title.size = (int64_t)bd_get_title_size(bd);
	bluray_title.size_mbs = ceil((double)bluray_title.size / 1048576);
	bluray_title.clips = bd_title->clip_count;
	bluray_title.angles = bd_title->angle_count;
	bluray_duration_length(bluray_title.length, bluray_title.duration);

	if(bluray_title.clips) {
		bluray_title.video_streams = bd_title->clips[0].video_stream_count;
		bluray_title.audio_streams = bd_title->clips[0].audio_stream_count;
		bluray_title.pg_streams = bd_title->clips[0].pg_stream_count;
	}

	// Display disc title
	if(bd_info->udf_volume_id && d_num_titles) {
		fprintf(io, "Disc Title: %s\n", bluray_info.bluray_title);
	}

	// Display title information
	if(p_bluray_copy) {
		fprintf(io, "Title: %03" PRIu32 ", Playlist: %04" PRIu32 ", Length: %s, Chapters: %02" PRIu32 ", Video streams: %02" PRIu8 ", Audio streams: %02" PRIu8 ", Subtitles: %02" PRIu8 ", Filesize: %05.0lf MBs\n", bluray_title.number, bluray_title.playlist, bluray_title.length, bluray_title.chapters, bluray_title.video_streams, bluray_title.audio_streams, bluray_title.pg_streams, bluray_title.size_mbs);
	}

	// Check for valid angle number
	if(arg_angle_number > bluray_title.angles) {
		fprintf(stderr, "Cannot select angle %" PRIu8 ", highest angle number is %" PRIu8 ".\n", arg_angle_number, bluray_title.angles);
		return 1;
	}
	retval = bd_select_angle(bd, arg_angle_number);
	if(retval < 0) {
		fprintf(stderr, "Could not select angle # %" PRIu8 "\n", arg_angle_number);
		return 1;
	}

	// Set fd output based on copying track to filename or sending to stdout
	if(p_bluray_copy) {
		bluray_copy.fd = open(bluray_copy.filename, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0644);
		if(!bluray_copy.fd) {
			fprintf(stderr, "Could not open filename %s\n", bluray_copy.filename);
			return 1;
		}
	} else if(p_bluray_cat) {
		bluray_copy.fd = 1;
	}

	/**
	 * Seek positions
	 *
	 * The bd object will always be at the same seek position as the last time it was
	 * read at. This can be displayed using bd_tell(). These function jump to the
	 * first position of its title to be safe.
	 *
	 * Some functions, while returning a position, will also seek to it as well.
	 *
	 * bd_tell(bd) returns current position
	 *
	 * bd_select_title() jumps to position 0 of title, returns 1 or 0 on success
	 *
	 * bd_select_playlist() jumps to position 0 of playlist, returns 1 or 0 on success
	 *
	 * bd_seek() will seek to a given position, and return the one it actually jumped to,
	 * which may not be the same. Jumping to 0 in cause stderr output, but also jump there:
	 *   bluray.c:256: 00264.m2ts: no timestamp for SPN 0 (got 0). clip 524280-58519717.
	 * Don't rely on bd_seek, and instead seek to the title or chapter directly.
	 *
	 * bd_seek_chapter() jumps and returns its new seek position. The position is the
	 * offset in bytes from the title (0); for the first chapter, it is probably 768.
	 *
	 */

	// Index: 0 amount to read, 1 amount successfully read, 3 total bytes successfully read
	int64_t bluray_read[3];
	bluray_read[0] = BLURAY_COPY_BUFFER_SIZE;
	bluray_read[1] = 0;
	bluray_read[2] = 0;

	// Index 0 amount to write, 1 amount successfully written, 3 total bytes successfully written
	int64_t bluray_write[3];
	bluray_write[0] = BLURAY_COPY_BUFFER_SIZE;
	bluray_write[1] = 0;
	bluray_write[2] = 0;

	// Initialize chapters array
	struct bluray_chapter bluray_chapters[bluray_title.chapters];
	uint64_t chapter_start = 0;
	for(chapter_ix = 0; chapter_ix < bluray_title.chapters; chapter_ix++) {

		bluray_chapters[chapter_ix].duration = 0;
		bluray_chapters[chapter_ix].start = chapter_start;
		strcpy(bluray_chapters[chapter_ix].length, "00:00:00.00");
		strcpy(bluray_chapters[chapter_ix].start_time, "00:00:00.00");
		bluray_chapters[chapter_ix].range[0] = 0;
		bluray_chapters[chapter_ix].range[1] = 0;
		bluray_chapters[chapter_ix].size = 0;
		bluray_chapters[chapter_ix].size_mbs = 0;

		bd_chapter = &bd_title->chapters[chapter_ix];

		if(bd_chapter == NULL)
			continue;

		bluray_chapters[chapter_ix].start = chapter_start;
		bluray_chapters[chapter_ix].duration = bd_chapter->duration;
		bluray_duration_length(bluray_chapters[chapter_ix].length, bd_chapter->duration);
		bluray_duration_length(bluray_chapters[chapter_ix].start_time, chapter_start);
		chapter_start += bd_chapter->duration;

	}

	// Keep track of chapter ranges, sizes: 0 start sector, 1 stop sector, 2 size
	// Since there's an offset of the first chapter from the title itself, the
	// size has to be calculated. When doing seeks or reads though, use position
	// as the index to see where you're at. No real reason to use the size other than
	// you'd like to know how big the chapter is.
	// Populate the array
	int64_t chapter_size = 0;
	int64_t chapter_pos = 0;
	for(chapter_ix = 0; chapter_ix < bluray_title.chapters; chapter_ix++) {
		chapter_pos = bd_chapter_pos(bd, chapter_ix);
		bluray_chapters[chapter_ix].range[0] = chapter_pos;
		if(debug) {
			fprintf(stderr, "* setting chapter ix %2" PRIu32 " start to %12" PRIi64 "\n", chapter_ix, chapter_pos);
			fprintf(stderr, "* bluray_chapters ix %2" PRIu32 " range 0: %" PRIi64 "\n", chapter_ix, bluray_chapters[chapter_ix].range[0]);
		}
	}

	// Set the final position size using the previous one as its ending point.
	// For the last chapter, it's simply the title size.
	// Calculate the chapter size as well.
	for(chapter_ix = 0; chapter_ix < bluray_title.chapters; chapter_ix++) {

		chapter_number = chapter_ix + 1;
		if(chapter_number == bluray_title.chapters) {
			chapter_pos = bluray_title.size;
		} else {
			chapter_pos = bluray_chapters[chapter_ix + 1].range[0];
		}
		bluray_chapters[chapter_ix].range[1] = chapter_pos;
		if(debug) {
			fprintf(stderr, "* setting chapter ix %2" PRIu32 " stop to	%12" PRIi64 "\n", chapter_ix, bluray_chapters[chapter_ix].range[1]);
		}

		chapter_size = bluray_chapters[chapter_ix].range[1] - bluray_chapters[chapter_ix].range[0];
		bluray_chapters[chapter_ix].size = chapter_size;
		bluray_chapters[chapter_ix].size_mbs = round((double)chapter_size / 1048576);
		if(debug) {
			fprintf(stderr, "* setting chapter ix %2" PRIu32 " size to	%12" PRIi64 "\n", chapter_ix, bluray_chapters[chapter_ix].size);
		}

	}

	if(debug) {
		fprintf(stderr, "* last chapter stop should be title size %" PRIi64 " which is: %" PRIi64 "\n", bluray_title.size, bluray_chapters[bluray_title.chapters - 1].range[1]);
	}

	// Buffer to read into
	unsigned char *bluray_buffer = calloc(BLURAY_COPY_BUFFER_SIZE, 1);

	// Current position
	if(debug) {
		fprintf(stderr, "* bd_tell position: %" PRIu64 "\n", bd_tell(bd));
	}

	// Keep track of when it's time to display next chapter's information
	// Track progress in MBs, percentages; actual, then displayed
	double progress[3];
	progress[0] = 0;
	progress[1] = 0;
	progress[2] = 0;

	// Get the total size to be copied from the first position of the
	// first chapter, to the end of the second.
	if(bluray_title.chapters == 1) {
		bluray_copy.size = bluray_chapters[0].range[1] - bluray_chapters[0].range[0];
	} else {
		bluray_copy.size = bluray_chapters[chapters_range[1]].range[1] - bluray_chapters[chapters_range[0]].range[0];
	}
	bluray_copy.size_mbs = ceil(ceil((double)bluray_copy.size) / 1048576);

	if(debug) {
		printf("* bluray_copy.size: %" PRIi64 "\n", bluray_copy.size);
		printf("* bluray_copy.size_mbs: %lf\n", bluray_copy.size_mbs);
		printf("* bd_tell: %" PRIu64 "\n", bd_tell(bd));
	}

	// Reset indexes
	chapter_ix = chapters_range[0];
	chapter_number = chapter_ix + 1;

	// Jump to the first requested chapter
	bd_seek_chapter(bd, chapter_ix);

	if(debug) {
		printf("* chapters_range[0]: %" PRIu32 "\n", chapters_range[0]);
		printf("* chapter_number: %" PRIu32 "\n", chapter_number);
		printf("* bd_tell: %" PRIu64 "\n", bd_tell(bd));
	}

	// Display the first chapter
	fprintf(io, "	Chapter: %03" PRIu32 ", Start: %s, Length: %s\n", chapter_number, bluray_chapters[chapter_ix].start_time, bluray_chapters[chapter_ix].length);

	// Loop until specifically broken out
	while(true) {

		// Display chapter information -- have to start on the second chapter, because while jumping
		// positions, the first chapter position will actually be hit twice.
		if(bluray_chapters[chapter_ix].range[0] == (int64_t)bd_tell(bd) && chapter_number <= bluray_title.chapters && chapter_ix > chapters_range[0]) {
			fprintf(io, "\33[2K");
			fprintf(io, "	Chapter: %03" PRIu32 ", Start: %s, Length: %s\n", chapter_number, bluray_chapters[chapter_ix].start_time, bluray_chapters[chapter_ix].length);
		}

		// Read from the bluray
		bluray_read[1] = (int64_t)bd_read(bd, bluray_buffer, (int)bluray_read[0]);

		// bd_read will return up to the length required, and stop if it's at the
		// end of the file. Therefore, your buffer size is going to be the result
		// of the read if using the same amount.
		if(bluray_read[1] == 0) {
			if(debug) {
				fprintf(stderr, "\n");
				fprintf(stderr, "* EOF\n");
			}
			break;
		}

		// Find out what AACS read error occurred
		if(bluray_read[1] == -1) {
			fprintf(stderr, "\n");
			fprintf(stderr, "* Error decoding AACS on disc: ");
			switch(bd_info->aacs_error_code) {
				case BD_AACS_CORRUPTED_DISC:
					fprintf(stderr, "corrupted disc\n");
					break;
				case BD_AACS_NO_CONFIG:
					fprintf(stderr, "missing configuration file\n");
					break;
				case BD_AACS_NO_PK:
					fprintf(stderr, "no matching process key\n");
					break;
				case BD_AACS_NO_CERT:
					fprintf(stderr, "no certificate\n");
					break;
				case BD_AACS_CERT_REVOKED:
					fprintf(stderr, "certificate revoked\n");
					break;
				case BD_AACS_MMC_FAILED:
					fprintf(stderr, "MMC authentication failed\n");
					break;
				default:
					fprintf(stderr, "unknown\n");
					break;
			}
			break;
		}

		// If the amount received is less than we requested, then this is the last pass
		if(debug && bluray_read[1] < bluray_read[0]) {
			fprintf(stderr, "\n");
			fprintf(stderr, "* read less than normal read amount, this current pass should be the last one\n");
		}

		// Keep track of amount read *after* checking for EOF and errors
		// A 4K quad-layer disc can hold 128 GB (source: Blu-ray spec PDF, 4th Edition, p14)
		// Therefore, max byte size is 137438953472 (1024 * 1024 * 1024 * 128)
		// For original Blu-ray, max size is 50 GB, or 53687091200. Since the character
		// length of the 4K max is one more than the 50, every position displayed is padded
		// up to 12 integers.
		// Pad MBs to 6 integers
		// The minimum integer size for that high is int64_t, and uint64_t passes
		// that positive max (man limits.h). It's always going to be safe to cast everything up
		// even though you'll never read that much in this program. The amount written max
		// going to be some multiplication of 1 MB (1048576), and that is chosen based on
		// human-readability of progress output. A double can also store the max size as well.
		bluray_read[2] += bluray_read[1];

		// Write to the file
		bluray_write[0] = bluray_read[1];
		bluray_write[1] = (int64_t)write(bluray_copy.fd, bluray_buffer, (size_t)bluray_write[0]);
		bluray_write[2] += bluray_write[1];

		progress[0] = (double)bluray_read[2] / 1048576;
		if(progress[0] > progress[1]) {
			progress[1]++;
			progress[2] = (progress[1] / bluray_copy.size_mbs) * 100;
			if(debug) {
				fprintf(stderr, "* success: %08" PRIi64 " bytes; total size_mbs read: %06" PRIi64 "; position: %012" PRIu64 ", chapter ix: %03" PRIu32 ", chapter number: %03" PRIu32 "; Progress: %.0lf/%.0lf MBs\r", bluray_read[1], bluray_read[2] / 1048576, bd_tell(bd), bd_get_current_chapter(bd), bd_get_current_chapter(bd) + 1, progress[1], bluray_copy.size_mbs);
				fflush(stderr);
			}
			fprintf(stderr, "Progress: %6.0lf/%.0lf MBs (%.0lf%%)\r", progress[1], bluray_copy.size_mbs, progress[2]);
			fflush(stderr);
		}

		// Set variables for the current chapter after this read
		chapter_ix = bd_get_current_chapter(bd);
		chapter_number = chapter_ix + 1;

		// Stop if we've reached pulling the last of the chapter range
		// This will only work if the last requested chapter is before the actual last one
		if(chapter_ix > chapters_range[1]) {
			break;
		}

	}

	fprintf(io, "\n");

	if(debug) {
		fprintf(stderr, "* current chapter ix: %" PRIu32 "\n", bd_get_current_chapter(bd));
		fprintf(stderr, "* total bytes read: %" PRIi64 " bytes\n", bluray_read[2]);
		fprintf(stderr, "* total MBs read: %lf bytes\n", ceil(ceil((double)bluray_read[2]) / 1048576));
	}

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
