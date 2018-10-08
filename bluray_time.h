#ifndef BLURAY_INFO_TIME_H
#define BLURAY_INFO_TIME_H

#define BLURAY_DURATION 11

#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint64_t bluray_duration_seconds(const uint64_t duration);

uint64_t bluray_duration_minutes(const uint64_t duration);

void bluray_duration_length(char *str, const uint64_t duration);

#endif
