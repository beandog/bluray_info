#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdbool.h>
#include <limits.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/stat.h>
#include "libbluray/bluray.h"
#include "libbluray/filesystem.h"

#if defined(__linux__)
#define DEFAULT_BLURAY_DEVICE "/dev/sr0"
#elif defined(__DragonFly__)
#define DEFAULT_BLURAY_DEVICE "/dev/cd0"
#elif defined(__FreeBSD__)
#define DEFAULT_BLURAY_DEVICE "/dev/cd0"
#elif defined(__NetBSD__)
#define DEFAULT_BLURAY_DEVICE "/dev/cd0d"
#elif defined(__OpenBSD__)
#define DEFAULT_BLURAY_DEVICE "/dev/rcd0c"
#elif defined(__APPLE__) && defined(__MACH__)
#define DEFAULT_BLURAY_DEVICE "/dev/disk1"
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(__MSYS__)
#define DEFAULT_BLURAY_DEVICE "D:\\"
#else
#define DEFAULT_BLURAY_DEVICE "/dev/sr0"
#endif

#define BLURAY_M2TS_UNIT_SIZE 6144

int main(int argc, char **argv) {

	bool debug = false;

	bool exit_help = false;
	char key_db_filename[PATH_MAX] = {'\0'};
	int g_opt = 0;
	int g_ix = 0;
	struct option p_long_opts[] = {
		{ "keydb", required_argument, NULL, 'k' },
		{ "debug", required_argument, NULL, 'z' },
		{ 0, 0, 0, 0 }
	};
	while((g_opt = getopt_long(argc, argv, "kz", p_long_opts, &g_ix)) != -1) {

		switch(g_opt) {
			case 'k':
				memset(key_db_filename, '\0', PATH_MAX);
				strncpy(key_db_filename, optarg, PATH_MAX - 1);
				break;
			case 'z':
				debug = true;
				break;
			case 0:
			default:
				printf("bluray_id - calculate Blu-ray unique identifier\n");
				printf("\n");
				printf("Usage: bluray_id [path] [options]\n");
				printf("\n");
				printf("Options:\n");
				printf("  -k, --keydb <filename>   Location to KEYDB.cfg\n");
				exit_help = true;
				break;
		}
	}

	if(exit_help)
		return 0;

	char device_filename[PATH_MAX];
	memset(device_filename, '\0', PATH_MAX);

	if (argv[optind])
		strncpy(device_filename, argv[optind], PATH_MAX - 1);
	else
		strncpy(device_filename, DEFAULT_BLURAY_DEVICE, PATH_MAX - 1);

	BLURAY *bd = NULL;
	struct stat s_buffer;

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
	if (bd == NULL) {
		fprintf(stderr, "Can't open device %s.\n", device_filename);
		return 1;
	}

	char src_filename[PATH_MAX];
	memset(src_filename, '\0', PATH_MAX);

	char tmp_template[] = "/tmp/id_XXXXXX";
	int fd = 0;

	char *str = mkdtemp(tmp_template);
	if(str == NULL) {
		fprintf(stderr, "couldn't create temporary directory\n");
	}

	BD_DIRENT *bdnd_dirent = NULL;
	bdnd_dirent = calloc(1, sizeof(BD_DIRENT));
	if(bdnd_dirent == NULL) {
		fprintf(stderr, "calloc() for bdnd_dirent failed\n");
		return 1;
	}

	struct bd_dir_s *bdnd_dir = NULL;
	bdnd_dir = bd_open_dir(bd, "BDMV/BACKUP/CLIPINF/");

	if(bdnd_dir == NULL) {
		fprintf(stderr, "bd_open_dir(bd, parent_dir) failed\n");
		return 1;
	}

	char tmp_dirname[PATH_MAX];
	memset(tmp_dirname, '\0', PATH_MAX);

	strcpy(tmp_dirname, str);

	struct bd_file_s *bd_file = NULL;

	unsigned char buffer[BLURAY_M2TS_UNIT_SIZE];
	memset(buffer, '\0', BLURAY_M2TS_UNIT_SIZE);

	int retval = -1;
	int64_t amount_read = -1;

	ssize_t w;

	char clpi_filename[PATH_MAX];
	memset(clpi_filename, '\0', PATH_MAX);

	char dest_filename[PATH_MAX];
	memset(dest_filename, '\0', PATH_MAX);

	int bdnd_read = 0;
	while(bdnd_read == 0) {

		bdnd_read = bdnd_dir->read(bdnd_dir, bdnd_dirent);

		if(bdnd_read > 0)
			break;


		if(bdnd_dirent->d_name[0] == '.') {
			bdnd_read = 0;
			continue;
		}

		sprintf(clpi_filename, "BDMV/BACKUP/CLIPINF/%s", bdnd_dirent->d_name);

		sprintf(dest_filename, "%s/%s", tmp_dirname, bdnd_dirent->d_name);

		if(debug)
			fprintf(stderr, "%s -> %s\n", clpi_filename, dest_filename);

		fd = open(dest_filename, O_RDWR | O_CREAT | O_TRUNC, 0644);

		bd_file = bd_open_file_dec(bd, clpi_filename);

		if(bd_file == NULL) {
			continue;
		}

		if(!strstr(bdnd_dirent->d_name, ".clpi"))
			continue;

		do {

			amount_read = bd_file->read(bd_file, buffer, BLURAY_M2TS_UNIT_SIZE);

			if(amount_read < 0) {
				fprintf(stderr, "bd_file->read() failed\n");
				return 1;
			}

			w = write(fd, buffer, amount_read);

			if(w == -1) {
				fprintf(stderr, "write() failed\n");
				return 1;
			}

		} while(amount_read > 0);

		close(fd);

	}

	char str_command[PATH_MAX];
	memset(str_command, '\0', PATH_MAX);

	sprintf(str_command, "cat %s/*.clpi | md5sum - | awk '{print $1}'", tmp_dirname);
	system(str_command);

	return 0;

}
