#ifndef BLURAY_INFO_PGS_H
#define BLURAY_INFO_PGS_H
#define BLURAY_PGS_LANG_STRLEN 4

#include <string.h>
#include "libbluray/bluray.h"

struct bluray_pgs {
	char lang[BLURAY_PGS_LANG_STRLEN];
};

void bluray_pgs_lang(char *str, uint8_t lang[BLURAY_PGS_LANG_STRLEN]);

#endif
