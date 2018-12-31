#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "libbluray/bluray.h"
#include "openssl/sha.h"

#define BLURAY_BLOCK_SIZE 65536

// Returns a SHA1 sum of the first bluray block of the main playlist

// gcc -o bluray_uniqid bluray_uniqid.c -lbluray -lcrypto

int main(int argc, char **argv) {

	BLURAY *bd = NULL;

	bd = bd_open(argv[optind] ? argv[optind] : "/dev/sr0", NULL);

	if(bd == NULL)
		return 1;

	bd_get_disc_info(bd);
	bd_get_titles(bd, TITLES_RELEVANT, 0);
	bd_select_title(bd, bd_get_main_title(bd));
	bd_get_title_info(bd, bd_get_main_title(bd), 0);
	bd_select_angle(bd, 1);
	bd_seek_chapter(bd, 1);

	unsigned char *bluray_buffer = NULL;
	bluray_buffer = calloc(BLURAY_BLOCK_SIZE, sizeof(unsigned char));

	bd_read(bd, bluray_buffer, BLURAY_BLOCK_SIZE);
	bd_close(bd);
	bd = NULL;

	if(bluray_buffer == NULL)
		return 1;

	int i;
	unsigned char str_source[BLURAY_BLOCK_SIZE * 2] = {'\0'};
	for(i = 0; i < BLURAY_BLOCK_SIZE; i++) {
		sprintf(&str_source[i * 2], "%02x", bluray_buffer[i]);
	}

	unsigned char str_dest[SHA_DIGEST_LENGTH];
	unsigned char str_hash[SHA_DIGEST_LENGTH];
	SHA1(str_source, strlen(str_source), str_dest);

	for(i = 0; i < SHA_DIGEST_LENGTH; i++) {
		sprintf(&str_hash[i * 2], "%02x", str_dest[i]);
	}
	printf("%s\n", str_hash);

	return 0;

}
