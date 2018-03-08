/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-xy] mosaic-stream")

static int tiled_x = 0;
static int tiled_y = 0;

#define TEST(X, Y)\
	(!*(size_t *)(img + (Y) * mosaic->width + (X)) &&\
	 mos[(Y) * mosaic->width + (X)][0] == ch1 &&\
	 mos[(Y) * mosaic->width + (X)][1] == ch2 &&\
	 mos[(Y) * mosaic->width + (X)][2] == ch3 &&\
	 mos[(Y) * mosaic->width + (X)][3] == ch4)

#define SEARCH(TYPE, SEARCH_FUNCTION)\
	do {\
		typedef TYPE pixel_t[4];\
		\
		pixel_t *restrict mos = (pixel_t *)mbuf;\
		pixel_t *restrict img = (pixel_t *)output;\
		size_t n, s, e, w;\
		\
		*(size_t *)(img + y * mosaic->width + x) = index;\
		\
		n = y ? y - 1 : tiled_y ? mosaic->height - 1 : y;\
		s = y <= mosaic->height ? y + 1 : tiled_y ? 0 : y;\
		w = x ? x - 1 : tiled_x ? mosaic->width - 1 : x;\
		e = x <= mosaic->width ? x + 1 : tiled_x ? 0 : x;\
		\
		if (TEST(x, n)) SEARCH_FUNCTION(output, mbuf, mosaic, x, n, index, ch1, ch2, ch3, ch4);\
		if (TEST(x, s)) SEARCH_FUNCTION(output, mbuf, mosaic, x, s, index, ch1, ch2, ch3, ch4);\
		if (TEST(e, y)) SEARCH_FUNCTION(output, mbuf, mosaic, e, y, index, ch1, ch2, ch3, ch4);\
		if (TEST(w, y)) SEARCH_FUNCTION(output, mbuf, mosaic, w, y, index, ch1, ch2, ch3, ch4);\
	} while (0)\

#define PROCESS(TYPE, SEARCH_FUNCTION)\
	do {\
		typedef TYPE pixel_t[4];\
		\
		static pixel_t *avg = NULL;\
		static TYPE *cnt = NULL;\
		static size_t size = 0;\
		\
		pixel_t *restrict clr = (pixel_t *)cbuf;\
		pixel_t *restrict mos = (pixel_t *)mbuf;\
		pixel_t *img = (pixel_t *)output;\
		size_t index = 0;\
		size_t x, y, i;\
		\
		memset(img, 0, mosaic->frame_size);\
		\
		for (y = 0; y < mosaic->height; y++)\
			for (x = 0; x < mosaic->width; x++)\
				if (!*(size_t *)(img + y * mosaic->width + x))\
					SEARCH_FUNCTION(img, mos, mosaic, x, y, ++index,\
					                mos[y * mosaic->width + x][0],\
					                mos[y * mosaic->width + x][1],\
					                mos[y * mosaic->width + x][2],\
					                mos[y * mosaic->width + x][3]);\
		\
		if (index > size) {\
			size = index;\
			avg = erealloc2(avg, size, sizeof(*avg));\
			cnt = erealloc2(cnt, size, sizeof(*cnt));\
		}\
		memset(avg, 0, index * sizeof(*avg));\
		memset(cnt, 0, index * sizeof(*cnt));\
		\
		for (y = 0; y < mosaic->height; y++) {\
			for (x = 0; x < mosaic->width; x++) {\
				i = y * mosaic->width + x;\
				index = *(size_t *)(img + i) - 1;\
				cnt[index] += (TYPE)1;\
				avg[index][0] *= (cnt[index] - (TYPE)1) / cnt[index];\
				avg[index][1] *= (cnt[index] - (TYPE)1) / cnt[index];\
				avg[index][2] *= (cnt[index] - (TYPE)1) / cnt[index];\
				avg[index][3] *= (cnt[index] - (TYPE)1) / cnt[index];\
				avg[index][3] += clr[i][3] /= cnt[index];\
				avg[index][0] += clr[i][0] *= clr[i][3];\
				avg[index][1] += clr[i][1] *= clr[i][3];\
				avg[index][2] += clr[i][2] *= clr[i][3];\
			}\
		}\
		\
		for (i = 0; i < index; i++) {\
			if (avg[i][3]) {\
				avg[i][0] /= avg[i][3];\
				avg[i][1] /= avg[i][3];\
				avg[i][2] /= avg[i][3];\
			}\
		}\
		\
		for (y = 0; y < mosaic->height; y++) {\
			for (x = 0; x < mosaic->width; x++) {\
				i = y * mosaic->width + x;\
				index = *(size_t *)(img + i) - 1;\
				img[i][0] = avg[index][0];\
				img[i][1] = avg[index][1];\
				img[i][2] = avg[index][2];\
				img[i][3] = avg[index][3];\
			}\
		}\
		\
		(void) colour;\
	} while (0)

static void
search_lf(void *restrict output, void *restrict mbuf, struct stream *mosaic,
	  size_t x, size_t y, size_t index, double ch1, double ch2, double ch3, double ch4)
{
	SEARCH(double, search_lf);
}

static void
search_f(void *restrict output, void *restrict mbuf, struct stream *mosaic,
	 size_t x, size_t y, size_t index, float ch1, float ch2, float ch3, float ch4)
{
	SEARCH(float, search_f);
}

static void
process_lf(char *restrict output, char *restrict cbuf, char *restrict mbuf,
	   struct stream *colour, struct stream *mosaic)
{
	PROCESS(double, search_lf);
}

static void
process_f(char *restrict output, char *restrict cbuf, char *restrict mbuf,
	  struct stream *colour, struct stream *mosaic)
{
	PROCESS(float, search_f);
}

int
main(int argc, char *argv[])
{
	struct stream colour, mosaic;
	void (*process)(char *restrict output, char *restrict cbuf, char *restrict mbuf,
			struct stream *colour, struct stream *mosaic);

	ARGBEGIN {
	case 'x':
		tiled_x = 1;
		break;
	case 'y':
		tiled_y = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 1)
		usage();

	eopen_stream(&colour, NULL);
	eopen_stream(&mosaic, argv[0]);

	SELECT_PROCESS_FUNCTION(&colour);
	CHECK_ALPHA(&colour);
	CHECK_N_CHAN(&colour, 4, 4);

	echeck_compat(&colour, &mosaic);
	fprint_stream_head(stdout, &colour);
	efflush(stdout, "<stdout>");
	process_each_frame_two_streams(&colour, &mosaic, STDOUT_FILENO, "<stdout>", process);
	return 0;
}
