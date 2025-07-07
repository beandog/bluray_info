#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
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
#include "libbluray/bluray.h"
#include "bluray_device.h"
#include "bluray_open.h"
#include "bluray_time.h"
#include "bluray_copy.h"

/**
 *   _     _
 *  | |__ | |_   _ _ __ __ _ _   _     ___ ___  _ __  _   _
 *  | '_ \| | | | | '__/ _` | | | |   / __/ _ \| '_ \| | | |
 *  | |_) | | |_| | | | (_| | |_| |  | (_| (_) | |_) | |_| |
 *  |_.__/|_|\__,_|_|  \__,_|\__, |___\___\___/| .__/ \__, |
 *                           |___/_____|       |_|    |___/
 *
 * copy tracks from a disc using libbluray
 *
 * If your Blu-ray is encrypted, you'll need libaacs as well as a modern
 * KEYDB.cfg file to access them.
 *
 * More information at https://dvds.beandog.org/
 *
 * Have fun :)
 *
 */

int main(int argc, char **argv) {

	FILE *io = stdout;
	char device_filename[PATH_MAX] = {'\0'};
	int retval = 0;
	bool p_bluray_copy = true;
	bool p_bluray_cat = false;
	struct bluray_copy bluray_copy;
	memset(bluray_copy.filename, '\0', PATH_MAX);
	bluray_copy.fd = -1;
	bluray_copy.size = 0;
	bluray_copy.size_mbs = 0;
	struct bluray_title bluray_title;
	struct bluray_info bluray_info;
	uint32_t main_playlist = 0;
	uint32_t copy_playlist = 0;
	uint32_t ix = 0;

	// Parse options and arguments
	bool opt_playlist_number = false;
	bool opt_main_title = false;
	bool exit_help = false;
	bool duplicates = true;
	bool opt_filename = false;
	unsigned long int arg_number = 0;
	uint32_t arg_playlist_number = 0;
	uint8_t angle_ix = 0;
	uint8_t arg_angle_number = 1;
	bool debug = false;
	char key_db_filename[PATH_MAX] = {'\0'};

	// Chapter range selection
	uint32_t arg_chapter_numbers[2];
	arg_chapter_numbers[0] = 1;
	arg_chapter_numbers[1] = 0;

	char *token = NULL;
	int g_opt = 0;
	int g_ix = 0;
	struct option p_long_opts[] = {
		{ "angle", required_argument, NULL, 'a' },
		{ "chapter", required_argument, NULL, 'c' },
		{ "no-duplicates", required_argument, NULL, 'D' },
		{ "help", no_argument, NULL, 'h' },
		{ "keydb", required_argument, NULL, 'k' },
		{ "main", no_argument, NULL, 'm' },
		{ "output", required_argument, NULL, 'o' },
		{ "playlist", required_argument, NULL, 'p' },
		{ "version", no_argument, NULL, 'V' },
		{ "debug", no_argument, NULL, 'z' },
		{ 0, 0, 0, 0 }
	};
	while((g_opt = getopt_long(argc, argv, "a:c:Dhk:mo:p:zV", p_long_opts, &g_ix)) != -1) {

		switch(g_opt) {

			case 'a':
				arg_number = strtoul(optarg, NULL, 10);
				if(arg_number < 2) {
					arg_angle_number = 1;
					angle_ix = 0;
				} else {
					arg_angle_number = (uint8_t)arg_number;
					angle_ix = arg_angle_number - 1;
				}
				break;

			case 'c':
				token = strtok(optarg, "-");

				arg_number = strtoul(token, NULL, 10);
				if(arg_number > 0) {
					arg_chapter_numbers[0] = (uint32_t)arg_number;
				}

				token = strtok(NULL, "-");
				if(token == NULL) {
					arg_chapter_numbers[1] = arg_chapter_numbers[0];
				} else {
					arg_number = strtoul(token, NULL, 10);
					if(arg_number > 0) {
						arg_chapter_numbers[1] = (uint32_t)arg_number;
					}
				}
				break;

			case 'D':
				duplicates = false;
				break;

			case 'k':
				memset(key_db_filename, '\0', PATH_MAX);
				strncpy(key_db_filename, optarg, PATH_MAX - 1);
				break;

			case 'm':
				opt_main_title = true;
				opt_playlist_number = false;
				break;

			case 'o':
				opt_filename = true;
				strncpy(bluray_copy.filename, optarg, PATH_MAX - 1);
				if(strncmp("-", bluray_copy.filename, 1) == 0) {
					p_bluray_copy = false;
					p_bluray_cat = true;
					io = stderr;
				}
				break;

			case 'p':
				opt_playlist_number = true;
				opt_main_title = false;
				arg_number = strtoul(optarg, NULL, 10);
				arg_playlist_number = (uint32_t)arg_number;
				if(arg_playlist_number > 99999) {
					fprintf(stderr, "Playlist number must be between 0 and 99999\n");
					return 1;
				}
				break;

			case 'z':
				debug = true;
				break;

			case 'V':
				printf("bluray_copy " PACKAGE_VERSION "\n");
				return 0;

			case 'h':
				printf("bluray_copy - copy a Blu-ray playlist to a file\n");
				printf("\n");
				printf("Usage: bluray_copy [path] [options]\n");
				printf("\n");
				printf("  -m, --main               Copy main playlist (default)\n");
				printf("  -p, --playlist <#>       Copy playlist number\n");
				printf("  -c, --chapter <#>[-#]    Copy chapter number or range\n");
				printf("  -o, --output <filename>  Save to filename (default: bluray_playlist_###.m2ts)\n");
				printf("      --output -           Write to stdout\n");
				printf("  -k, --keydb <filename>   Location to KEYDB.cfg (default: ~/.config/aacs/KEYDB.cfg)\n");
				printf("  -a, --angle <#>          Video angle (default: 1)\n");
				printf("  -D, --no-duplicates      Do not use duplicates index (see man bluray_copy)\n");
				printf("  -V, --version            Display version\n");
				printf("  -h, --help               This output\n");
				printf("\n");
				printf("Blu-ray path can be a device, filename, or directory (default: " DEFAULT_BLURAY_DEVICE ")\n");
				exit_help = true;
				break;

			case 0:
			default:
				exit_help = true;
				break;

		}

	}

	if(exit_help)
		return 0;

	if(!opt_playlist_number)
		opt_main_title = true;

	memset(device_filename, '\0', PATH_MAX);
	if (argv[optind])
		strncpy(device_filename, argv[optind], PATH_MAX - 1);
	else
		strncpy(device_filename, DEFAULT_BLURAY_DEVICE, PATH_MAX - 1);

	// Open device
	BLURAY *bd = NULL;
	struct stat buffer;

	if(strlen(key_db_filename)) {
		if(stat(key_db_filename, &buffer) != 0) {
			fprintf(stderr, "Could not open device %s and KEYDB file %s\n", device_filename, key_db_filename);
			return 1;
		}
		bd = bd_open(device_filename, key_db_filename);
	}
	if(bd == NULL && (stat("KEYDB.cfg", &buffer) == 0))
		bd = bd_open(device_filename, "KEYDB.cfg");
	if(bd == NULL)
		bd = bd_open(device_filename, NULL);

	if(bd == NULL) {
		if(strlen(key_db_filename))
			fprintf(stderr, "Could not open device %s and KEYDB file %s\n", device_filename, key_db_filename);
		else
			fprintf(stderr, "Could not open device %s\n", device_filename);
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
	retval = bluray_info_init(bd, &bluray_info, duplicates);

	if(retval) {
		fprintf(stderr, "Couldn't open Blu-ray\n");
		return 1;
	}


	// Find main playlist
	uint32_t arr_playlists[bluray_info.titles];
	memset(arr_playlists, 0, sizeof(arr_playlists));

	BLURAY_TITLE_INFO *bd_title;
	bd_title = NULL;

	for(ix = 0; ix < bluray_info.titles; ix++) {

		bd_title = bd_get_title_info(bd, ix, angle_ix);

		// Not sure what's going to happen if this doesn't work, I don't
		// have any broken Blu-rays. Skipping it should be safe, though.
		if(bd_title == NULL)
			continue;

		arr_playlists[ix] = bd_title->playlist;

		if(bd_title->idx == bluray_info.main_title)
			main_playlist = bd_title->playlist;

		bd_free_title_info(bd_title);
		bd_title = NULL;

	}

	if(opt_main_title) {

		retval = bluray_title_init(bd, &bluray_title, bluray_info.main_title, angle_ix, false);

		if(retval) {
			fprintf(stderr, "Could not open main playlist %" PRIu32 "\n", main_playlist);
			bd_close(bd);
			return 1;
		}

		copy_playlist = main_playlist;


	} else if(opt_playlist_number) {

		retval = bd_select_playlist(bd, arg_playlist_number);

		if(retval == 0) {
			fprintf(stderr, "Could not open playlist %" PRIu32 "\n", arg_playlist_number);
			bd_close(bd);
			return 1;
		}

		copy_playlist = arg_playlist_number;

		bluray_title.ix = bd_get_current_title(bd);

	}

	if(!opt_filename)
		snprintf(bluray_copy.filename, PATH_MAX - 1, "%s%05" PRIu32 "%s", "bluray_playlist_", copy_playlist, ".m2ts");

	// Init bluray_title struct
	retval = bluray_title_init(bd, &bluray_title, bluray_title.ix, angle_ix, false);

	// Quit if playlist couldn't be opened
	if(retval) {

		if(opt_playlist_number)
			fprintf(stderr, "Could not open playlist %u\n", arg_playlist_number);
		else
			fprintf(stderr, "Could not open main playlist %u\n", main_playlist);

		bd_close(bd);
		bd = NULL;
		return 1;

	}

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

	// Display disc title
	if(strlen(bluray_info.disc_name)) {
		fprintf(io, "Disc title: %s\n", bluray_info.disc_name);
	}

	// Display title information
	if(p_bluray_copy) {
		fprintf(io, "Playlist: %*" PRIu32 ", Length: %s, Chapters: %*" PRIu32 ", Video streams: %*" PRIu8 ", Audio streams: %*" PRIu8 ", Subtitles: %*" PRIu8 ", Angles: %" PRIu8 ", Filesize: %*" PRIu64 " MBs\n", 4, bluray_title.playlist, bluray_title.length, 2, bluray_title.chapters, 2, bluray_title.video_streams, 2, bluray_title.audio_streams, 2, bluray_title.pg_streams, bluray_title.angles, 5, bluray_title.size_mbs);
	}

	// Check for valid angle number
	if(arg_angle_number > bluray_title.angles) {
		fprintf(stderr, "Cannot select angle %" PRIu8 ", highest angle number is %" PRIu8 ".\n", arg_angle_number, bluray_title.angles);
		return 1;
	}

	// Don't change angle number unless passed as an argument
	if(arg_angle_number) {
		retval = bd_select_angle(bd, angle_ix);
		if(retval < 0) {
			fprintf(stderr, "Could not select angle # %" PRIu8 "\n", arg_angle_number);
			return 1;
		}
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

	// Index: 0 - amount to read; 1 - amount successfully read; 2 - total bytes successfully read
	int64_t bluray_read[3];
	bluray_read[0] = BLURAY_COPY_BUFFER_SIZE;
	bluray_read[1] = 0;
	bluray_read[2] = 0;

	// Index: 0 - amount to write; 1 - amount successfully written; 2 - total bytes successfully written
	int64_t bluray_write[3];
	bluray_write[0] = BLURAY_COPY_BUFFER_SIZE;
	bluray_write[1] = 0;
	bluray_write[2] = 0;

	// Initialize chapters array
	BLURAY_TITLE_CHAPTER *bd_chapter = NULL;
	struct bluray_chapter bluray_chapters[bluray_title.chapters];
	uint64_t chapter_start = 0;
	uint32_t chapter_ix = 0;
	for(chapter_ix = 0; chapter_ix < bluray_title.chapters; chapter_ix++) {

		bluray_chapters[chapter_ix].duration = 0;
		bluray_chapters[chapter_ix].start = chapter_start;
		strcpy(bluray_chapters[chapter_ix].length, "00:00:00.000");
		strcpy(bluray_chapters[chapter_ix].start_time, "00:00:00.000");
		bluray_chapters[chapter_ix].range[0] = 0;
		bluray_chapters[chapter_ix].range[1] = 0;
		bluray_chapters[chapter_ix].size = 0;
		bluray_chapters[chapter_ix].size_mbs = 0;

		// bd_chapter = &bd_title->chapters[chapter_ix];
		bd_chapter = &bluray_title.title_chapters[chapter_ix];

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

	// The size for chapter one may be incorrect, because while the title start position
	// is at 0, the chapter start position will have an offset. To do proper calculation
	// I'd have to do a read, and check the new positions. It's a bit of a headache, and
	// I'm not comfortable with it, and I don't want to display the filesize anyway, so
	// everything here but the first chapter is going to be accurate. Only display the
	// estimated sizes for debugging.

	// Populate the array
	uint64_t chapter_size = 0;
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
	uint32_t chapter_number = 1;
	for(chapter_ix = 0; chapter_ix < bluray_title.chapters; chapter_ix++) {

		chapter_number = chapter_ix + 1;
		if(chapter_number == bluray_title.chapters) {
			chapter_pos = (int64_t)bluray_title.size;
		} else {
			chapter_pos = bluray_chapters[chapter_ix + 1].range[0];
		}
		bluray_chapters[chapter_ix].range[1] = chapter_pos;
		if(debug) {
			fprintf(stderr, "* setting chapter ix %2" PRIu32 " stop to	%12" PRIi64 "\n", chapter_ix, bluray_chapters[chapter_ix].range[1]);
		}

		chapter_size = (uint64_t)(bluray_chapters[chapter_ix].range[1] - bluray_chapters[chapter_ix].range[0]);
		bluray_chapters[chapter_ix].size = chapter_size;
		bluray_chapters[chapter_ix].size_mbs = (uint64_t)(round((double)chapter_size / 1048576));
		if(debug) {
			fprintf(stderr, "* setting chapter ix %2" PRIu32 " size to	%12" PRIi64 "\n", chapter_ix, bluray_chapters[chapter_ix].size);
		}

	}

	if(debug) {
		fprintf(stderr, "* last chapter stop should be title size %" PRIu64 " which is: %" PRIi64 "\n", bluray_title.size, bluray_chapters[bluray_title.chapters - 1].range[1]);
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
	// Note that even though the first chapter will have an offset from the beginning
	// of the title, bd_read() will start from 0, meaning you can't rely on the
	// chapter positions as absolute definitions of what the filesize will actually be.
	// For that reason, I used to have filesize in here, but it's not guaranteed to
	// be accurate, so don't display it.
	fprintf(io, "	Chapter: %03" PRIu32 ", Start: %s, Length: %s\n", chapter_number, bluray_chapters[chapter_ix].start_time, bluray_chapters[chapter_ix].length);

	// Loop until specifically broken out
	ssize_t write_retval = -1;
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

		// Find out what read error occurred
		if(bluray_read[1] == -1) {

			fprintf(stderr, "\n");
			fprintf(stderr, "Read error on disc\n");

			if(bd_info->aacs_error_code) {

				fprintf(stderr, "Error decoding AACS: ");

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
		write_retval = write(bluray_copy.fd, bluray_buffer, (size_t)bluray_write[0]);

		// Check for failed write
		if(write_retval != (ssize_t)bluray_write[0]) {
			if(errno == ENOSPC)
				fprintf(stderr, "Could not write to device, no remaining space available\n");
			fprintf(stderr, "Tried to write %" PRIi64 " bytes to %s and failed, quitting\n", bluray_write[0], bluray_copy.filename);
			close(bluray_copy.fd);
			return 1;
		}

		bluray_write[1] = (int64_t)write_retval;
		bluray_write[2] += bluray_write[1];

		progress[0] = (double)bluray_read[2] / 1048576;
		if(progress[0] > progress[1]) {
			progress[1]++;
			progress[2] = (progress[1] / bluray_copy.size_mbs) * 100;
			if(debug) {
				fprintf(stderr, "* success: %08" PRIi64 " bytes; total size_mbs read: %06" PRIi64 "; position: %012" PRIu64 ", chapter ix: %03" PRIu32 ", chapter number: %03" PRIu32 "; Progress: %.0lf/%.0lf MBs\r", bluray_read[1], bluray_read[2] / 1048576, bd_tell(bd), bd_get_current_chapter(bd), bd_get_current_chapter(bd) + 1, progress[1], bluray_copy.size_mbs);
				fflush(stderr);
			}
			if(progress[2] == 100.0)
				progress[2] = 99.0;
			fprintf(stderr, "Progress: %6.0lf/%.0lf MBs (%.0lf%%)\r", progress[1], bluray_copy.size_mbs, progress[2]);
			fflush(stderr);
		}

		// Only use getting current chapter as setting boundary for copying, not the
		// calculated size or specified end positions. This is intentionally done
		// to avoid calculation errors.
		chapter_ix = bd_get_current_chapter(bd);
		chapter_number = chapter_ix + 1;

		// Stop if we've reached pulling the last of the chapter range
		// This will only work if the last requested chapter is before the actual last one
		if(chapter_ix > chapters_range[1]) {
			break;
		}

	}

	fprintf(stderr, "Progress: %6.0lf/%.0lf MBs (100%%)\r", progress[1], bluray_copy.size_mbs);
	fflush(stderr);

	fprintf(io, "\n");

	if(debug) {
		fprintf(stderr, "* current chapter ix: %" PRIu32 "\n", bd_get_current_chapter(bd));
		fprintf(stderr, "* total bytes read: %" PRIi64 " bytes\n", bluray_read[2]);
		fprintf(stderr, "* total MBs read: %lf bytes\n", ceil(ceil((double)bluray_read[2]) / 1048576));
	}

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
			return 1;
		}
	}

	if(!p_bluray_cat)
		fprintf(io, "Filename: %s\n", bluray_copy.filename);

	return 0;

}
