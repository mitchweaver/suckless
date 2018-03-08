/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("[-d | -g | -h | -l power-stream | -p power-stream | -v]")
/* TODO add [-w weight-stream] for [-ghlpv] */

/* Because the syntax for a function returning a function pointer is disgusting. */
typedef void (*process_func)(struct stream *stream, void *buffer, void *image, size_t frame);

/*
 * X-parameter 1: method enum value
 * X-parameter 2: identifier-friendly name
 * X-parameter 3: images
 * X-parameter 4: action for first frame
 * X-parameter 5: pre-process assignments
 * X-parameter 6: subcell processing
 * X-parameter 7: pre-finalise assignments
 * X-parameter 8: subcell finalisation
 */
#define LIST_MEANS(TYPE)\
	/* [default] arithmetic mean */\
	X(ARITHMETIC, arithmetic, 1, COPY_FRAME,, *img += *buf,\
	  a = (TYPE)1 / (TYPE)frame, *img *= a)\
	/* standard deviation */\
	X(STANDARD_DEVIATION, sd, 2, ZERO_AND_PROCESS_FRAME,, (*img += *buf * *buf, *aux += *buf),\
	  a = (TYPE)1 / (TYPE)frame, *img = nnpow((*img - *aux * *aux * a) * a, (TYPE)0.5))\
	/* geometric mean */\
	X(GEOMETRIC, geometric, 1, COPY_FRAME,, *img *= *buf,\
	  a = (TYPE)1 / (TYPE)frame, *img = nnpow(*img, a))\
	/* harmonic mean */\
	X(HARMONIC, harmonic, 1, ZERO_AND_PROCESS_FRAME,, *img += (TYPE)1 / *buf,\
	  a = (TYPE)frame, *img = a / *img)\
	/* Lehmer mean */\
	X(LEHMER, lehmer, 2, ZERO_AND_PROCESS_FRAME,,\
	  (*img += nnpow(*buf, *pows), *aux += nnpow(*buf, *pows - (TYPE)1)),, *img /= *aux)\
	/* power mean (Hölder mean) (m = 2 for root square mean; m = 3 for cubic mean) */\
	X(POWER, power, 1, ZERO_AND_PROCESS_FRAME,, *img += nnpow(*buf, *pows),\
	  a = (TYPE)1 / (TYPE)frame, *img = a * nnpow(*img, (TYPE)1 / *pows))\
	/* variance */\
	X(VARIANCE, variance, 2, ZERO_AND_PROCESS_FRAME,, (*img += *buf * *buf, *aux += *buf),\
	  a = (TYPE)1 / (TYPE)frame, *img = (*img - *aux * *aux * a) * a)

enum first_frame_action {
	COPY_FRAME,
	PROCESS_FRAME,
	ZERO_AND_PROCESS_FRAME,
};

#define X(V, ...) V,
enum method { LIST_MEANS() };
#undef X

static void *powerbuf = NULL;

#define MAKE_PROCESS(PIXFMT, TYPE,\
		     _1, NAME, _3, _4, PRE_PROCESS, PROCESS_SUBCELL, PRE_FINALISE, FINALISE_SUBCELL)\
	static void\
	process_##PIXFMT##_##NAME(struct stream *stream, void *buffer, void *image, size_t frame)\
	{\
		TYPE *buf = buffer, *img = image, a, *pows = powerbuf;\
		TYPE *aux = (TYPE *)(((char *)image) + stream->frame_size);\
		size_t x, y, z;\
		if (!buf) {\
			PRE_FINALISE;\
			for (z = 0; z < stream->n_chan; z++)\
				for (y = 0; y < stream->height; y++)\
					for (x = 0; x < stream->width; x++, img++, aux++, pows++)\
						FINALISE_SUBCELL;\
		} else {\
			PRE_PROCESS;\
			for (z = 0; z < stream->n_chan; z++)\
				for (y = 0; y < stream->height; y++)\
					for (x = 0; x < stream->width; x++, img++, aux++, pows++, buf++) { \
						PROCESS_SUBCELL;\
					}\
		}\
		(void) aux, (void) a, (void) pows, (void) frame;\
	}
#define X(...) MAKE_PROCESS(lf, double, __VA_ARGS__)
LIST_MEANS(double)
#undef X
#define X(...) MAKE_PROCESS(f, float, __VA_ARGS__)
LIST_MEANS(float)
#undef X
#undef MAKE_PROCESS

#define X(ID, NAME, ...) [ID] = process_lf_##NAME,
static const process_func process_functions_lf[] = { LIST_MEANS() };
#undef X

#define X(ID, NAME, ...) [ID] = process_f_##NAME,
static const process_func process_functions_f[] = { LIST_MEANS() };
#undef X

int
main(int argc, char *argv[])
{
	struct stream stream, power;
	void *buf, *img;
	process_func process;
	size_t frames, images;
	enum method method = ARITHMETIC;
	enum first_frame_action first_frame_action;
	const char *power_file = NULL;

	ARGBEGIN {
	case 'd':
		method = STANDARD_DEVIATION;
		break;
	case 'g':
		method = GEOMETRIC;
		break;
	case 'h':
		method = HARMONIC;
		break;
	case 'l':
		method = LEHMER;
		power_file = UARGF();
		break;
	case 'p':
		method = POWER;
		power_file = UARGF();
		break;
	case 'v':
		method = VARIANCE;
		break;
	default:
		usage();
	} ARGEND;

	if (argc)
		usage();

#define X(ID, _2, IMAGES, FIRST_FRAME_ACTION, ...)\
	case ID:\
		images = IMAGES;\
		first_frame_action = FIRST_FRAME_ACTION;\
		break;
	switch (method) {
	LIST_MEANS()
	default:
		abort();
	}
#undef X

	eopen_stream(&stream, NULL);
	if (power_file != NULL) {
		eopen_stream(&power, power_file);
		echeck_compat(&stream, &power);
		powerbuf = emalloc(power.frame_size);
		if (!eread_frame(&power, powerbuf))
			eprintf("%s is no frames\n", power_file);
	}

        if (stream.encoding == DOUBLE)
                process = process_functions_lf[method];
        else if (stream.encoding == FLOAT)
                process = process_functions_f[method];
	else
		eprintf("pixel format %s is not supported, try xyza\n", stream.pixfmt);

	stream.frames = 1;
	echeck_dimensions(&stream, WIDTH | HEIGHT, NULL);
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	buf = emalloc(stream.frame_size);
	if (first_frame_action == ZERO_AND_PROCESS_FRAME)
		img = ecalloc(images, stream.frame_size);
	else
		img = emalloc2(images, stream.frame_size);

	frames = 0;
	if (first_frame_action == COPY_FRAME) {
		if (!eread_frame(&stream, img))
			eprintf("video is no frames\n");
		frames++;
	}
	for (; eread_frame(&stream, buf); frames++)
		process(&stream, buf, img, frames);
	if (!frames)
		eprintf("video has no frames\n");
	process(&stream, NULL, img, frames);

	ewriteall(STDOUT_FILENO, img, stream.frame_size, "<stdout>");
	free(buf);
	free(img);
	free(powerbuf);
	return 0;
}
