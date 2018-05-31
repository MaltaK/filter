#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "common.h"
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <math.h>
#include <pthread.h>

const char Gx[3][3] = {{-1,  0,  1}, {-2, 0, 2}, {-1, 0, 1} };
const char Gy[3][3] = {{-1, -2, -1}, { 0, 0, 0}, { 1, 2, 1} };

struct s_thr_data {
	unsigned int cnt, num;
	imag_t *imag;
	pixe_t *data;
};
typedef struct s_thr_data thr_t;

void *sobel_thr(void *arg)
{
	thr_t *thd = (thr_t *)arg;
	const int w = thd->imag->w;
	const float h = thd->imag->h / (float)thd->cnt;
	const int start_y = thd->num ? h * thd->num : 1;
	int end_y;

	if (thd->num + 1 < thd->cnt)
		end_y = h * (thd->num + 1);
	else
		end_y = thd->imag->h - 1;

	for (int y = start_y; y < end_y; ++y) {
		pixe_t *row = thd->imag->data + (w * y);

		for (int x = 1; x < w - 1; ++x) {
			int gx = 0, gy = 0;
			pixe_t *px;

			for (int i = 0; i < 3; ++i)
				for (int j = 0; j < 3; ++j) {
					px = thd->data + (x - 1 + i +
						w * (y - 1 + j));

					int d = (px->R + px->G + px->B) / 3;

					gx += Gx[i][j] * d;
					gy += Gy[i][j] * d;
				}

			px = row + x;
			px->R = px->G = px->B =
				sqrt(gx * gx + gy * gy) < 255 ? sqrt(gx * gx + gy * gy) : 255;
		}
	}
	pthread_exit(0);
}

double safe_pop_timer(timespec_t *prev)
{
	timespec_t timer, dur;

	assert(clock_gettime(CLOCK_MONOTONIC_RAW, &timer) == 0);

	if ((timer.tv_nsec - prev->tv_nsec) < 0) {
		dur.tv_sec = timer.tv_sec - prev->tv_sec - 1;
		dur.tv_nsec = 1E9 + timer.tv_nsec - prev->tv_nsec;
	} else {
		dur.tv_sec = timer.tv_sec - prev->tv_sec;
		dur.tv_nsec = timer.tv_nsec - prev->tv_nsec;
	}

	return dur.tv_sec + dur.tv_nsec / 1E9;
}


int main(int argc, char *argv[])
{
	int th_num =atoi(argv[3]);
	char *path_i =argv[1], *path_o =argv[2];
	
	if (!path_i || !path_o || !th_num) {
		puts("usage:\n\t./fil img.pnm out 4");
		exit(EXIT_SUCCESS);
	}


	FILE *fd = fopen(path_i, "r");
	char format[3];

	fscanf(fd, "%s", format);

	assert(strncmp(format, "P6", 2) == 0);

	imag_t *imagg = safe_calloc(sizeof(imag_t), 1);

	fscanf(fd, "%u", &imagg->w);
	fscanf(fd, "%u", &imagg->h);

	int temp;

	fscanf(fd, "%d", &temp);
	assert(temp == 255);

	imagg->size = imagg->w * imagg->h * sizeof(pixe_t);

	imagg->data = safe_calloc(imagg->size, 1);

	fread(imagg->data, 1, imagg->size, fd);

	fclose(fd);

	imag_t *imag = imagg;
	pixe_t *data = imag->data;

	imag->data = safe_calloc(imag->size, 1);

	if (th_num * 3 > imag->h)
		th_num = imag->h / 3.0 > 1 ? imag->h / 3.0 : 1;

	thr_t *thr_arr = safe_calloc(th_num, sizeof(thr_t));
	pthread_t *thrs = safe_calloc(th_num, sizeof(pthread_t));

	timespec_t timer;

	safe_push_timer(&timer);
	for (int i = 0; i < th_num; i++) {
	
		thr_arr[i] = (thr_t){th_num, i, imag, data};
		assert(pthread_create(thrs + i, NULL, sobel_thr, thr_arr + i) == 0);
	}

	for (int i = 0; i < th_num; i++)
		assert(pthread_join(thrs[i], NULL) == 0);

	printf("total proc. time = %lf\n", safe_pop_timer(&timer));
	free(thr_arr);
	free(thrs);

	FILE *fd1 = fopen(path_o, "w");

	fprintf(fd1, "P6\n%u %u\n255\n", imag->w, imag->h);

	fwrite(imag->data, 1, imag->size, fd1);

	fclose(fd1);

	free(data);
	free(imag->data);
	free(imag);

	exit(EXIT_SUCCESS);
}
