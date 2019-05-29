#ifndef BLURAY_INFO_TIME_H
#define BLURAY_INFO_TIME_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "libbluray/bluray.h"

uint64_t bluray_duration_seconds(const uint64_t duration);

uint64_t bluray_duration_minutes(const uint64_t duration);

void bluray_duration_length(char *str, const uint64_t duration);

uint64_t bluray_chapter_duration(struct bluray *bd, const uint32_t title_ix, const uint32_t chapter_ix, const uint8_t angle_ix);

void bluray_chapter_length(char *dest_str, struct bluray *bd, const uint32_t title_ix, const uint32_t chapter_ix, const uint8_t angle_ix);

#endif
