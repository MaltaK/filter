#pragma once

#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define safe_calloc(nb, size) \
	({ void *buf = calloc(nb, size); assert(buf != NULL); buf;})

struct __attribute__((__packed__)) spixel {
	unsigned char R, G, B;
};
typedef struct spixel pixe_t;

struct simage {
	unsigned int w, h, size;
	pixe_t *data;
};
typedef struct simage imag_t;

imag_t *get_imag(const char *path);
void put_imag(const char *path, imag_t *imag);

typedef struct timespec timespec_t;

void safe_push_timer(timespec_t *timer)
{
	assert(clock_gettime(CLOCK_MONOTONIC_RAW, timer) == 0);
}

void sproc(const char *path_i, const char *path_o, int th_num);


double safe_pop_timer(timespec_t *prev);
