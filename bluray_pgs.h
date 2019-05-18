#ifndef BLURAY_INFO_PGS_H
#define BLURAY_INFO_PGS_H
#define BLURAY_PGS_LANG_STRLEN 4
#define BLURAY_PGS_CHAR_CODE 13

#include <string.h>
#include "libbluray/bluray.h"

void bluray_pgs_lang(char *str, const uint8_t lang[4]);

#endif
