#include "bluray_device.h"

bool bluray_optical_drive(const char *device_filename) {

	if(strncmp(device_filename, "/dev/", 5) == 0)
		return true;

	bool optical_drive = false;

	// Improved check to see if device filename is a mounted device

	FILE *mtab = setmntent("/proc/mounts", "r");

	if(mtab != NULL) {

		struct mntent *mnt = getmntent(mtab);

		while(mnt != NULL) {
			if(strncmp(device_filename, mnt->mnt_dir, strlen(device_filename)) == 0) {
				optical_drive = true;
				break;
			}
			mnt = getmntent(mtab);
		}
	}

	return optical_drive;

}
