#include "bluray_device.h"

// Improved check to see if device filename is a mounted device
bool bluray_optical_drive(const char *device_filename) {

	if(strncmp(device_filename, "/dev/", 5) == 0)
		return true;

	FILE *mtab = setmntent("/proc/mounts", "r");

	if(mtab == NULL)
		return false;

	struct mntent *mnt = getmntent(mtab);

	while(mnt != NULL) {

		if(strncmp(device_filename, mnt->mnt_dir, strlen(device_filename)) == 0)
			return true;

		mnt = getmntent(mtab);

	}

	return false;

}
