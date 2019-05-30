#ifndef BLURAY_INFO_TIME_H
#define BLURAY_INFO_TIME_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "libbluray/bluray.h"

uint64_t bluray_duration_seconds(uint64_t duration);

uint64_t bluray_duration_minutes(uint64_t duration);

void bluray_duration_length(char *str, uint64_t duration);

uint64_t bluray_chapter_duration(struct bluray *bd, uint32_t title_ix, uint32_t chapter_ix, uint8_t angle_ix);

void bluray_chapter_length(char *dest_str, struct bluray *bd, uint32_t title_ix, uint32_t chapter_ix, uint8_t angle_ix);

#endif
