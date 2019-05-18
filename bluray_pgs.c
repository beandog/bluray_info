#include "bluray_pgs.h"

void bluray_pgs_lang(char *str, const uint8_t lang[4]) {

	memset(str, '\0', 4);
	memcpy(str, lang, 3);

}
