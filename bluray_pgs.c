#include "bluray_pgs.h"

void bluray_pgs_lang(char *str, const uint8_t lang[BLURAY_PGS_LANG_STRLEN]) {

	memset(str, '\0', BLURAY_PGS_LANG_STRLEN);
	memcpy(str, lang, BLURAY_PGS_LANG_STRLEN - 1);

}
