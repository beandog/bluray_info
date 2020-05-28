#ifndef BLURAY_CHAPTER_H
#define BLURAY_CHAPTER_H

#include <stdlib.h>
#include <inttypes.h>
#include "libbluray/bluray.h"

uint64_t bluray_chapter_first_position(struct bluray *bd, const uint32_t title_ix, const uint32_t chapter_ix);

uint64_t bluray_chapter_last_position(struct bluray *bd, const uint32_t title_ix, const uint32_t chapter_ix);

uint64_t bluray_chapter_size(struct bluray *bd, const uint32_t title_ix, const uint32_t chapter_ix);

#endif
