#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdbool.h>
#include <limits.h>
#include <fcntl.h>
#include "libbluray/bluray.h"
#include "libbluray/filesystem.h"

#define BLURAY_M2TS_UNIT_SIZE 6144
#define BLURAY_FILENAME_STRLEN 256

int main(int argc, char **argv) {

	char device_filename[PATH_MAX];
	memset(device_filename, '\0', PATH_MAX);

	if(argc == 1)
		strcpy(device_filename, "/dev/sr0");
	else
		strcpy(device_filename, argv[1]);

	BLURAY *bd = NULL;
	bd = bd_open(device_filename, NULL);

	if (bd == NULL) {
		fprintf(stderr, "bd_open() failed\n");
		return 1;
	}

	char src_filename[BLURAY_FILENAME_STRLEN];
	memset(src_filename, '\0', BLURAY_FILENAME_STRLEN);

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

	char clpi_filename[BLURAY_FILENAME_STRLEN];
	memset(clpi_filename, '\0', BLURAY_FILENAME_STRLEN);

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

		// fprintf(stderr, "%s -> %s\n", clpi_filename, dest_filename);

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
