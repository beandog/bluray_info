#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <inttypes.h>
#include <ctype.h>
#include <math.h>
#include <getopt.h>
#include <sys/stat.h>
#include <time.h>
#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#include "libbluray/bluray.h"
#include "libbluray/filesystem.h"
#include "bluray_device.h"
#include "bluray_open.h"
#include "bluray_time.h"

#define BLURAY_M2TS_UNIT_SIZE 6144
#define BLURAY_FILENAME_STRLEN 256

/*
 *  _     _                           _                _
 * | |__ | |_   _ _ __ __ _ _   _    | |__   __ _  ___| | ___   _ _ __
 * | '_ \| | | | | '__/ _` | | | |   | '_ \ / _` |/ __| |/ / | | | '_ \
 * | |_) | | |_| | | | (_| | |_| |   | |_) | (_| | (__|   <| |_| | |_) |
 * |_.__/|_|\__,_|_|  \__,_|\__, |___|_.__/ \__,_|\___|_|\_\\__,_| .__/
 *                          |___/_____|                          |_|
 *
 * Back-up a Blu-ray and ignore DRM.
 *
 * A rewrite of 'bluraybackup' by Matteo Bini
 * https://git.golem.linux.it/matteobin/bluraybackup
 *
 */

bool debug;
int fd_dnames;
int fd_fnames;
int bluray_dirs;
int bluray_files;

// qsort has a sample function for this, but since we're passing an array
// of arrays instead of an array of char *, theirs won't work.
// https://stackoverflow.com/questions/31751782/qsort-array-of-strings-in-alphabetical-order
static int sort_arr_filenames(const void *, const void *);

int add_filename_to_tmpfile(char *, int);

int mk_backup_dir(char *, char *);

int log_filenames(BLURAY *, char *, char *);

static int sort_arr_filenames(const void *a, const void *b) {

	const char *str1 = (const char *)a;
	const char *str2 = (const char *)b;

	return strcmp(str1, str2);

}

int add_filename_to_tmpfile(char *filename, int fd) {

	ssize_t w;
	w = write(fd, filename, strlen(filename));
	if(w == -1)
		return 1;
	return 0;

}

int mk_backup_dir(char *backup_dir, char *bluray_dir) {

	int retval;

	char backup_mkdir_path[PATH_MAX];
	memset(backup_mkdir_path, '\0', PATH_MAX);

	snprintf(backup_mkdir_path, PATH_MAX - 1, "%s/%s", backup_dir, bluray_dir);

	if(debug)
		fprintf(stderr, "Creating Blu-ray backup directory: '%s'\n", backup_mkdir_path);

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(__MSYS__)
	retval = mkdir(backup_mkdir_path);
#else
	retval = mkdir(backup_mkdir_path, 0755);
#endif

	// I'm too lazy to see if the directory exists and permissions are correct, maybe
	// some day. Let's just assume the setup already works.
	if(retval == -1 && errno == EEXIST)
		return 0;

	if(retval == -1 && errno == EEXIST)
		fprintf(stderr, "Cannot create directory '%s'\n", backup_mkdir_path);

	return retval;

}

/**
 * MASTER PLAN
 *
 * Run this function (or something like it) multiple times:
 * 1) Get all the directory filenames
 * 1a) Go ahead and make a function to create ALL the directories and run through that (need to
 *     write this still)
 * 2) Write all the filenames to a temp file
 *
 * So that, I can then have a separate function just to make
 * all the directories and clearly check that it's working properly
 * and then a separate function just to copy all the files, and
 * make sure that works properly. Yay!
 */
int log_filenames(BLURAY *bd, char *parent_dir, char *target_dir) {

	char dirname[BLURAY_FILENAME_STRLEN];
	char tmp_dirname[BLURAY_FILENAME_STRLEN];
	char tmp_filename[BLURAY_FILENAME_STRLEN];

	struct bd_dir_s *bdnd_dir;
	BD_DIRENT *bdnd_dirent;
	bdnd_dirent = calloc(1, sizeof(BD_DIRENT));
	if(bdnd_dirent == NULL) {
		fprintf(stderr, "calloc() for bdnd_dirent failed\n");
		return 1;
	}
	bdnd_dir = bd_open_dir(bd, parent_dir);
	if(bdnd_dir == NULL) {
		fprintf(stderr, "bd_open_dir(bd, parent_dir) failed\n");
		return 1;
	}

	int bdnd_read = 0;
	while(bdnd_read == 0) {

		bdnd_read = bdnd_dir->read(bdnd_dir, bdnd_dirent);

		if(bdnd_read == -1) {
			if(debug)
				fprintf(stderr, "bdnd_dir->read() moving to next result\n");
			continue;
		}

		if (strchr(bdnd_dirent->d_name, '.') == NULL) {

			// FIXME doing the counting here first, need this REGARDLESS of whether making the directory works or not
			bluray_dirs++;

			memset(dirname, '\0', BLURAY_FILENAME_STRLEN);
			snprintf(dirname, BLURAY_FILENAME_STRLEN, "%s/%s", parent_dir, bdnd_dirent->d_name);

			if(debug)
				printf("%s\n", dirname);

			memset(tmp_dirname, '\0', BLURAY_FILENAME_STRLEN);
			snprintf(tmp_dirname, BLURAY_FILENAME_STRLEN - 1, "%s\n", dirname);
			add_filename_to_tmpfile(tmp_dirname, fd_dnames);

			log_filenames(bd, dirname, target_dir);

		} else {

			// Doing the counting here first, need this REGARDLESS of whether the actual backup of the file works or not
			bluray_files++;

			memset(tmp_filename, '\0', BLURAY_FILENAME_STRLEN);
			snprintf(tmp_filename, BLURAY_FILENAME_STRLEN - 1, "%s/%s\n", parent_dir, bdnd_dirent->d_name);

			// FIXME I don't like this, not having the newline, it's going to cause confusion
			// at some point.
			if(debug)
				printf("%s", tmp_filename);

			add_filename_to_tmpfile(tmp_filename, fd_fnames);

		}

	}

	// I don't think the order matters here, but I'm freeing the one who is created second, first
	free(bdnd_dir);
	free(bdnd_dirent);

	return 0;

}

int main(int argc, char **argv) {

	debug = false;

	char device_filename[PATH_MAX] = {'\0'};
	memset(device_filename, '\0', PATH_MAX);

	char key_db_filename[PATH_MAX] = {'\0'};
	memset(key_db_filename, '\0', PATH_MAX);

	char bluray_backup_dir[PATH_MAX] = {'\0'};
	memset(bluray_backup_dir, '\0', PATH_MAX);
	strcpy(bluray_backup_dir, ".");

	bool copy_m2ts = true;

	bool exit_help = false;

	int g_opt = 0;
	int g_ix = 0;
	struct option p_long_opts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "directory", required_argument, NULL, 'd' },
		{ "keydb", required_argument, NULL, 'k' },
		{ "skip-m2ts", no_argument, NULL, 's' },
		{ "version", no_argument, NULL, 'V' },
		{ "debug", no_argument, NULL, 'z' },
		{ 0, 0, 0, 0 }
	};
	while((g_opt = getopt_long(argc, argv, "hd:k:szV", p_long_opts, &g_ix)) != -1) {

		switch(g_opt) {

			case 'd':
				strncpy(bluray_backup_dir, optarg, PATH_MAX - 1);
				break;

			case 'k':
				strncpy(key_db_filename, optarg, PATH_MAX - 1);
				break;

			case 's':
				copy_m2ts = false;
				break;

			case 'V':
				printf("bluray_backup " PACKAGE_VERSION "\n");
				exit_help = true;
				break;

			case 'z':
				debug = true;
				break;

			case 'h':
				printf("bluray_backup - copy a Blu-ray to a directory\n");
				printf("\n");
				printf("Usage: bluray_backup [path] [options]\n");
				printf("\n");
				printf("  -d, --directory       Save to directory (default: .)\n");
				printf("  -k, --keydb           Location of KEYDB file (default: libaacs directory)\n");
				printf("  -s, --skip-m2ts       Skip copying M2TS files\n");
				printf("  -V, --version         Display version\n");
				printf("  -h, --help            This output\n");
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

	if(argv[optind])
		strncpy(device_filename, argv[optind], PATH_MAX - 1);
	else
		strncpy(device_filename, DEFAULT_BLURAY_DEVICE, PATH_MAX - 1);

	BLURAY *bd = NULL;
	struct stat buffer;

	if(strlen(key_db_filename))
		bd = bd_open(device_filename, key_db_filename);
	if(bd == NULL && (stat("KEYDB.cfg", &buffer) == 0))
		bd = bd_open(device_filename, "KEYDB.cfg");
	if(bd == NULL)
		bd = bd_open(device_filename, NULL);

	if (bd == NULL) {
		fprintf(stderr, "Can't open device %s.\n", device_filename);
		exit(1);
	}

	bluray_dirs = 0;
	bluray_files = 0;

	srand(time(NULL));
	int random_number = rand();

	char dnames_filename[PATH_MAX];
	char fnames_filename[PATH_MAX];

	snprintf(dnames_filename, PATH_MAX, "/tmp/bd_dnames_%i", random_number);
	snprintf(fnames_filename, PATH_MAX, "/tmp/bd_fnames_%i", random_number);
	if(debug)
		fprintf(stderr, "temporary filenames: %s %s\n", dnames_filename, fnames_filename);

	// It's read + write here because I'm going to open it with fdopen() later
	fd_dnames = open(dnames_filename, O_RDWR | O_CREAT | O_TRUNC, 0644);

	if(fd_dnames == -1) {
		fprintf(stderr, "open(%s) failed\n", dnames_filename);
		return 1;
	}

	fd_fnames = open(fnames_filename, O_RDWR | O_CREAT | O_TRUNC, 0644);

	if(fd_fnames == -1) {
		fprintf(stderr, "open(%s) failed\n", fnames_filename);
		return 1;
	}

	/** Get the numbers of all the directories and files **/
	BD_DIRENT *parent_dirent = NULL;
	parent_dirent = calloc(1, sizeof(BD_DIRENT));
	if(parent_dirent == NULL) {
		fprintf(stderr, "calloc() on parent_dirent failed\n");
		return 1;
	}

	printf("Backing up '%s' to '%s'\n", device_filename, bluray_backup_dir);

	int retval;
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(__MSYS__)
	retval = mkdir(bluray_backup_dir);
#else
	retval = mkdir(bluray_backup_dir, 0755);
#endif
	if(retval == -1 && errno != EEXIST) {
		fprintf(stderr, "* could not create backup directory: %s\n", bluray_backup_dir);
		return 1;
	}

	bool directory = true;
	int fd;

	// BLURAY_FILENAME_STRLEN is less than PATH_MAX, so compiler warnings are thrown in gcc (clang is fine)
	char mkdir_path[BLURAY_FILENAME_STRLEN];
	char mkfile_path[BLURAY_FILENAME_STRLEN];

	// Selecting a subdirectory doesn't work for some reason, so root works fine
	struct bd_dir_s *parent_dir = NULL;
	parent_dir = bd_open_dir(bd, "");
	if(parent_dir == NULL) {
		fprintf(stderr, "bd_open_dir() failed\n");
		return 1;
	}

	// FIXME this is going through the ROOT directory (""), but maybe should call the function on
	// this instead? I've already found one bug. TODO I need a source where it has a file on the
	// root path. I need to make my own ISO with a file there.
	int parent_read = 0;
	while(parent_read == 0) {

		// READ THE DOCS
		// https://videolan.videolan.me/libbluray/structbd__dir__s.html#a8a1d68e8cfde09327d669ee1bc06c82e
		// 0 on success, 1 on EOF, <0 on error
		parent_read = parent_dir->read(parent_dir, parent_dirent);

		// Don't backup the DRM
		if(strstr(parent_dirent->d_name, "AACS") || strstr(parent_dirent->d_name, "CERTIFICATE"))
			continue;

		if(parent_read == -1) {
			// Failing on top directory /BDMV is expected, don't throw a warning
			if(debug && (strncmp(parent_dirent->d_name, "BDMV", 4) != 0))
				fprintf(stderr, "parent_dir->read() on '%s' failed\n", parent_dirent->d_name);
			break;
		}

		if(parent_read == 1) {
			if(debug)
				fprintf(stderr, "parent_dir->read() '%s' reached EOF\n", parent_dirent->d_name);
			continue;
		}

		// We might be backing up a directory instead of a device, so skip that
		if(parent_dirent->d_name[0] == '.') {
			if(debug)
				fprintf(stderr, "Skipping . and .. from filesystem directory\n");
			continue;
		}

		if(debug)
			printf("%s\n", parent_dirent->d_name);

		// Check if it's a directory or a file
		if (strchr(parent_dirent->d_name, '.') == NULL)
			directory = true;
		else
			directory = false;

		if(directory) {

			bluray_dirs++;

			fd = fd_dnames;

			memset(mkdir_path, '\0', BLURAY_FILENAME_STRLEN);
			snprintf(mkdir_path, BLURAY_FILENAME_STRLEN - 1, "%s\n", parent_dirent->d_name);

			add_filename_to_tmpfile(mkdir_path, fd);

			log_filenames(bd, parent_dirent->d_name, bluray_backup_dir);

		}

		// I've seen some discs with files on the root directory, but I can't remember which ones
		// they were. There are some that have PS3 updates on them.
		if(!directory) {

			bluray_files++;

			fd = fd_fnames;

			memset(mkfile_path, '\0', BLURAY_FILENAME_STRLEN);
			snprintf(mkfile_path, BLURAY_FILENAME_STRLEN - 1, "%s\n", parent_dirent->d_name);
			add_filename_to_tmpfile(mkfile_path, fd);

		}

	}

	if(debug) {
		fprintf(stderr, "total dirs: %i total files: %i\n", bluray_dirs, bluray_files);
	}

	// fdopen should be seeking to the beginning, but it's not for me for some reason. Even if
	// I was using fdopen for read only, it still needs to seek to beginning
	off_t retval_seek = 0;
	retval_seek = lseek(fd_dnames, 0, SEEK_SET);
	if(retval_seek == -1) {
		fprintf(stderr, "lseek to directory names fd failed, this shouldn't happen\n");
		return 1;
	}

	// Technically, I could open with "r" only, but since the fd is read + write, do the same here
	FILE *fp;
	fp = fdopen(fd_dnames, "r+");

	if(fp == NULL) {
		fprintf(stderr, "could not open directory names file\n");
		return 1;
	}

	// I want to create the directories in order, so get an array of the names, then sort them
	// and *then* make them
	int ix = 0;
	char line[BLURAY_FILENAME_STRLEN];
	char trimmed_line[BLURAY_FILENAME_STRLEN];
	char bluray_dirnames[bluray_dirs][BLURAY_FILENAME_STRLEN];
	while(fgets(line, BLURAY_FILENAME_STRLEN, fp)) {

		memset(bluray_dirnames[ix], '\0', BLURAY_FILENAME_STRLEN);
		// Trimming the newline from fgets() when copying
		strncpy(bluray_dirnames[ix], line, strlen(line) - 1);

		if(debug)
			printf("[%i/%i] %s\n", ix + 1, bluray_dirs, bluray_dirnames[ix]);

		ix++;

	}

	qsort(bluray_dirnames, (size_t)bluray_dirs, sizeof(*bluray_dirnames), sort_arr_filenames);

	int mkdir_retval = 0;
	for(ix = 0; ix < bluray_dirs; ix++) {

		mkdir_retval = mk_backup_dir(bluray_backup_dir, bluray_dirnames[ix]);

		if(mkdir_retval) {
			fprintf(stderr, "Can't create directory '%s/%s' quitting\n", bluray_backup_dir, trimmed_line);
			return 1;
		}

	}

	// Get filenames
	retval_seek = lseek(fd_fnames, 0, SEEK_SET);
	if(retval_seek == -1) {
		fprintf(stderr, "lseek to filenames fd failed, this shouldn't happen\n");
		return 1;
	}
	fp = fdopen(fd_fnames, "r+");

	if(fp == NULL)
		fprintf(stderr, "could not open directory names file\n");

	char bluray_filenames[bluray_files][BLURAY_FILENAME_STRLEN];

	ix = 0;
	while(fgets(line, BLURAY_FILENAME_STRLEN, fp)) {

		memset(bluray_filenames[ix], '\0', BLURAY_FILENAME_STRLEN);
		// Trimming the newline from fgets() when copying
		strncpy(bluray_filenames[ix], line, strlen(line) - 1);

		if(debug)
			printf("[%i/%i] %s\n", ix + 1, bluray_files, bluray_filenames[ix]);

		ix++;

	}

	qsort(bluray_filenames, (size_t)bluray_files, sizeof(*bluray_filenames), sort_arr_filenames);

	for(ix = 0; ix < bluray_files; ix++) {
		if(debug)
			printf("ix: %i filename: %s\n", ix, bluray_filenames[ix]);
	}

	/** THIS WORKS TOO, YAY! It copies all the files fine! :D **/
	uint8_t bluray_buffer[BLURAY_M2TS_UNIT_SIZE];
	memset(bluray_buffer, '\0', BLURAY_M2TS_UNIT_SIZE);
	struct bd_file_s *bd_file = NULL;

	// Index: 0 - amount to read; 1 - amount successfully read; 2 - total bytes successfully read
	int64_t bluray_read[3];
	bluray_read[0] = BLURAY_M2TS_UNIT_SIZE;
	bluray_read[1] = 0;
	bluray_read[2] = 0;

	// Index: 0 - amount to write; 1 - amount successfully written; 2 - total bytes successfully written
	int64_t bluray_write[3];
	bluray_write[0] = 0;
	bluray_write[1] = 0;
	bluray_write[2] = 0;

	char target_filename[BLURAY_FILENAME_STRLEN];
	memset(target_filename, '\0', BLURAY_FILENAME_STRLEN);

	int bluray_fd = 0;

	for(ix = 0; ix < bluray_files; ix++) {

		if(!copy_m2ts && strstr(bluray_filenames[ix], ".m2ts")) {
			printf("* [%i/%i] %s (skipping)\n", ix + 1, bluray_files, bluray_filenames[ix]);
			continue;
		}

		if(debug)
			printf("bluray_filenames[%i]: %s\n", ix, bluray_filenames[ix]);

		printf("* [%i/%i] %s\n", ix + 1, bluray_files, bluray_filenames[ix]);

		bd_file = bd_open_file_dec(bd, bluray_filenames[ix]);

		if(bd_file == NULL) {
			fprintf(stderr, "bd_open_file_dec(\"%s\") failed, skipping\n", bluray_filenames[ix]);
			continue;
		}

		 memset(target_filename, '\0', BLURAY_FILENAME_STRLEN);
		 snprintf(target_filename, BLURAY_FILENAME_STRLEN, "%s/%s", bluray_backup_dir, bluray_filenames[ix]);
		 bluray_fd = open(target_filename, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0644);

		if(bluray_fd == -1) {
			fprintf(stderr, "open(\"%s\") failed, skipping\n", bluray_filenames[ix]);
		}

		retval_seek = bd_file->seek(bd_file, 0, SEEK_SET);
		if(retval_seek == -1) {
			fprintf(stderr, "lseek to source filename fd failed, this shouldn't happen\n");
			return 1;
		}

		// For bd_file->read():
		// Returns number of bytes read, 0 on EOF, < 0 on error
		// https://videolan.videolan.me/libbluray/structbd__file__s.html#a4aa9a9cef0ee2b4c5009d8efb2469521
		do {
			bluray_read[1] = bd_file->read(bd_file, bluray_buffer, BLURAY_M2TS_UNIT_SIZE);

			// Error reading file
			if(bluray_read[1] == -1) {
				fprintf(stderr, "parent_dir->read() on '%s' failed\n", parent_dirent->d_name);
				break;
			}

			// Reached end of file
			if(bluray_read[1] == 0) {
				if(debug)
					fprintf(stderr, "parent_dir->read() '%s' reached EOF\n", parent_dirent->d_name);
				break;
			}

			bluray_read[2] += bluray_read[1];
			if(debug)
				fprintf(stderr, "bluray_read[2]: %" PRIi64 "\n", bluray_read[2]);

			bluray_write[1] = write(bluray_fd, &bluray_buffer, bluray_read[1]);
			bluray_write[2] += bluray_write[1];

		} while(bluray_read[1] > 0);

		bd_file->close(bd_file);

		close(bluray_fd);

	}

	fclose(fp);

	bd_close(bd);

	return 0;

}
