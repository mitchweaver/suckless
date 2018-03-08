/* See LICENSE file for copyright and license details. */
#include "common.h"

USAGE("pixel-format ...")

static int in_level = INT_MAX;
static int out_level = INT_MAX;

static void
lf_to_f(double *in, float *out, size_t n)
{
	size_t i;
	for (i = 0; i < n; i++)
		out[i] = (float)(in[i]);
}

static void
f_to_lf(double *in, float *out, size_t n)
{
	size_t i;
	for (i = 0; i < n; i++)
		out[i] = (double)(in[i]);
}

#if !defined(HOST_ENDIAN_IS_LITTLE_ENDIAN_16)
static void
le_to_h_16(uint16_t *buf, size_t n)
{
	size_t i;
	for (i = 0; i < n; i++)
		buf[i] = letoh(buf[i]);
}

static void
h_to_le_16(uint16_t *buf, size_t n)
{
	size_t i;
	for (i = 0; i < n; i++)
		buf[i] = htole(buf[i]);
}
#else
# define le_to_h_16(buf, n) ((void) buf, (void) n)
# define h_to_le_16(buf, n) ((void) buf, (void) n)
#endif

static size_t
remove_alpha_u16(uint16_t *buf, size_t n)
{
	size_t i, j;
	long int a, max = (long int)0xFF00L, ymax = 0xDAF4L;
	for (i = j = 0; i < n; i += 4, j += 3) {
		a = (long int)(buf[i + 3]);
		buf[j + 0] = (uint16_t)(((long int)(buf[i + 0]) - 0x1001L) * a / ymax + 0x1001L);
		buf[j + 1] = (uint16_t)(((long int)(buf[i + 1]) - 0x8000L) * a /  max + 0x8000L);
		buf[j + 2] = (uint16_t)(((long int)(buf[i + 2]) - 0x8000L) * a /  max + 0x8000L);
	}
	return j;
}

#define REMOVE_ALPHA()\
	do {\
		size_t i, j;\
		for (i = j = 0; i < n; i += 4, j += 3) {\
			buf[j + 0] = buf[i + 0] * buf[i + 3];\
			buf[j + 1] = buf[i + 1] * buf[i + 3];\
			buf[j + 2] = buf[i + 2] * buf[i + 3];\
		}\
		return j;\
	} while (0)

static size_t remove_alpha_lf(double *buf, size_t n) { REMOVE_ALPHA(); }
static size_t remove_alpha_f (float  *buf, size_t n) { REMOVE_ALPHA(); }

#define ADD_ALPHA(TYPE, MAX)\
	do {\
		size_t i = n, j = n + n / 3;\
		for (; i; i -= 3, j -= 4) {\
			out[j - 1] = (TYPE)(MAX);\
			out[j - 2] = in[i - 1];\
			out[j - 3] = in[i - 2];\
			out[j - 4] = in[i - 3];\
		}\
		return n + n / 3;\
	} while (0)

static size_t add_alpha_u16(uint16_t *in, uint16_t *out, size_t n) { ADD_ALPHA(uint16_t, UINT16_MAX); }
static size_t add_alpha_lf (double   *in, double   *out, size_t n) { ADD_ALPHA(double,   1); }
static size_t add_alpha_f  (float    *in, float    *out, size_t n) { ADD_ALPHA(float,    1); }

static void
raw0_to_raw1(uint16_t *buf, size_t n)
{
	size_t i;
	uint16_t t;
	for (i = 0; i < n; i += 4, buf += 4)
		t = buf[0], buf[0] = buf[1], buf[1] = buf[2], buf[2] = buf[3], buf[3] = t;
}

static void
raw1_to_raw0(uint16_t *buf, size_t n)
{
	size_t i;
	uint16_t t;
	for (i = 0; i < n; i += 4, buf += 4)
		t = buf[3], buf[3] = buf[2], buf[2] = buf[1], buf[1] = buf[0], buf[0] = t;
}

#define RAW2_TO_RAW3(TYPE, WITH_ALPHA)\
	do {\
		size_t i;\
		TYPE max = (TYPE)0xFF00L, ymax = (TYPE)0xDAF4L;\
		if (sizeof(*in) > sizeof(*out)) {\
			for (i = 0; i < n; i += 3 + WITH_ALPHA) {\
				out[i + 0] = (TYPE)((long int)(in[i + 0]) - 0x1001L) / ymax;\
				out[i + 1] = (TYPE)((long int)(in[i + 1]) - 0x8000L) /  max;\
				out[i + 2] = (TYPE)((long int)(in[i + 2]) - 0x8000L) /  max;\
				if (WITH_ALPHA) {\
					out[i + 3] = (TYPE)(in[i + 3]) / max;\
					out[i + 3] = CLIP(0, out[i + 3], 1);\
				}\
			}\
		} else {\
			for (i = n; i; i -= 3 + WITH_ALPHA) {\
				if (WITH_ALPHA) {\
					out[i - 1] = (TYPE)(in[i - 1]) / max;\
					out[i - 1] = CLIP(0, out[i - 1], 1);\
				}\
				out[i - 1 - WITH_ALPHA] = (TYPE)((long int)(in[i - 1 - WITH_ALPHA]) - 0x8000L) /  max;\
				out[i - 2 - WITH_ALPHA] = (TYPE)((long int)(in[i - 2 - WITH_ALPHA]) - 0x8000L) /  max;\
				out[i - 3 - WITH_ALPHA] = (TYPE)((long int)(in[i - 3 - WITH_ALPHA]) - 0x1001L) / ymax;\
			}\
		}\
	} while (0)

static void raw2_to_raw3_lf  (uint16_t *in, double *out, size_t n) { RAW2_TO_RAW3(double, 0); }
static void raw2_to_raw3_f   (uint16_t *in, float  *out, size_t n) { RAW2_TO_RAW3(float,  0); }
static void raw2a_to_raw3a_lf(uint16_t *in, double *out, size_t n) { RAW2_TO_RAW3(double, 1); }
static void raw2a_to_raw3a_f (uint16_t *in, float  *out, size_t n) { RAW2_TO_RAW3(float,  1); }

#define RAW3_TO_RAW2(TYPE, WITH_ALPHA)\
	do {\
		size_t i;\
		TYPE max = (TYPE)0xFF00L, ymax = (TYPE)0xDAF4L;\
		long int y, u, v;\
		if (sizeof(*in) > sizeof(*out)) {\
			for (i = 0; i < n; i += 3 + WITH_ALPHA) {\
				y = (long int)(in[i + 0] * ymax) + 0x1001L;\
				u = (long int)(in[i + 1] *  max) + 0x8000L;\
				v = (long int)(in[i + 2] *  max) + 0x8000L;\
				out[i + 0] = (uint16_t)CLIP(0, y, 0xFFFFL);\
				out[i + 1] = (uint16_t)CLIP(0, u, 0xFFFFL);\
				out[i + 2] = (uint16_t)CLIP(0, v, 0xFFFFL);\
				if (WITH_ALPHA) {\
					v = (long int)(in[i + 3] * max);\
					out[i + 3] = (uint16_t)CLIP(0, v, 0xFFFFL);\
				}\
			}\
		} else {\
			for (i = n; i; i -= 3 + WITH_ALPHA) {\
				if (WITH_ALPHA) {\
					v = (long int)(in[i - 1] * max);\
					out[i - 1] = (uint16_t)CLIP(0, v, 0xFFFFL); \
				}\
				v = (long int)(in[i - 1 - WITH_ALPHA] *  max) + 0x8000L;\
				u = (long int)(in[i - 2 - WITH_ALPHA] *  max) + 0x8000L;\
				y = (long int)(in[i - 3 - WITH_ALPHA] * ymax) + 0x1001L;\
				out[i - 1 - WITH_ALPHA] = (uint16_t)CLIP(0, v, 0xFFFFL);\
				out[i - 2 - WITH_ALPHA] = (uint16_t)CLIP(0, u, 0xFFFFL);\
				out[i - 3 - WITH_ALPHA] = (uint16_t)CLIP(0, y, 0xFFFFL);\
			}\
		}\
	} while (0)

static void raw3_to_raw2_lf  (double *in, uint16_t *out, size_t n) { RAW3_TO_RAW2(double, 0); }
static void raw3_to_raw2_f   (float  *in, uint16_t *out, size_t n) { RAW3_TO_RAW2(float,  0); }
static void raw3a_to_raw2a_lf(double *in, uint16_t *out, size_t n) { RAW3_TO_RAW2(double, 1); }
static void raw3a_to_raw2a_f (float  *in, uint16_t *out, size_t n) { RAW3_TO_RAW2(float,  1); }

#define CONVERT_COLOUR_SPACE(TYPE, CONV)\
	do {\
		size_t i, s = 3 + (size_t)a;\
		for (i = 0; i < n; i += s)\
			CONV(buf[i + 0], buf[i + 1], buf[i + 2], buf + i + 0, buf + i + 1, buf + i + 2);\
	} while (0)

static void conv_yuv_to_srgb_lf(double *buf, size_t n, int a) { CONVERT_COLOUR_SPACE(double, yuv_to_srgb); }
static void conv_yuv_to_srgb_f (float  *buf, size_t n, int a) { CONVERT_COLOUR_SPACE(float,  yuv_to_srgb); }
static void conv_srgb_to_yuv_lf(double *buf, size_t n, int a) { CONVERT_COLOUR_SPACE(double, srgb_to_yuv); }
static void conv_srgb_to_yuv_f (float  *buf, size_t n, int a) { CONVERT_COLOUR_SPACE(float,  srgb_to_yuv); }
static void conv_xyz_to_srgb_lf(double *buf, size_t n, int a) { CONVERT_COLOUR_SPACE(double, ciexyz_to_srgb); }
static void conv_xyz_to_srgb_f (float  *buf, size_t n, int a) { CONVERT_COLOUR_SPACE(float,  ciexyz_to_srgb); }
static void conv_srgb_to_xyz_lf(double *buf, size_t n, int a) { CONVERT_COLOUR_SPACE(double, srgb_to_ciexyz); }
static void conv_srgb_to_xyz_f (float  *buf, size_t n, int a) { CONVERT_COLOUR_SPACE(float,  srgb_to_ciexyz); }

#define CHANGE_TRANSFER(TYPE, CONV)\
	do {\
		size_t i, s = 3 + (size_t)a;\
		for (i = 0; i < n; i += s) {\
			buf[i + 0] = CONV(buf[i + 0]);\
			buf[i + 1] = CONV(buf[i + 1]);\
			buf[i + 2] = CONV(buf[i + 2]);\
		}\
	} while (0)

static void conv_srgbt_to_srgb_lf(double *buf, size_t n, int a) { CHANGE_TRANSFER(double, srgb_decode); }
static void conv_srgbt_to_srgb_f (float  *buf, size_t n, int a) { CHANGE_TRANSFER(float,  srgb_decode); }
static void conv_srgb_to_srgbt_lf(double *buf, size_t n, int a) { CHANGE_TRANSFER(double, srgb_encode); }
static void conv_srgb_to_srgbt_f (float  *buf, size_t n, int a) { CHANGE_TRANSFER(float,  srgb_encode); }

#define CONVERT_COLOUR_SPACE_AUTO(CONV)\
	static void\
	conv_##CONV(enum encoding encoding, int with_alpha, void *buf, size_t n)\
	{\
		if (encoding == DOUBLE)\
			conv_##CONV##_lf(buf, n, !!with_alpha);\
		else\
			conv_##CONV##_f(buf, n, !!with_alpha);\
	}
CONVERT_COLOUR_SPACE_AUTO(yuv_to_srgb)
CONVERT_COLOUR_SPACE_AUTO(srgb_to_yuv)
CONVERT_COLOUR_SPACE_AUTO(xyz_to_srgb)
CONVERT_COLOUR_SPACE_AUTO(srgb_to_xyz)
CONVERT_COLOUR_SPACE_AUTO(srgbt_to_srgb)
CONVERT_COLOUR_SPACE_AUTO(srgb_to_srgbt)

static void
convert(struct stream *stream, struct stream *out, void *buf, size_t n)
{
	enum encoding encoding = stream->encoding;

	if (in_level <= 0 && out_level > 0)
		raw0_to_raw1(buf, n);

	if (in_level <= 1 && out_level > 1)
		le_to_h_16(buf, n);

	if (in_level <= 2 && out_level > 2) {
		if (out->encoding == DOUBLE && stream->alpha)
			raw2a_to_raw3a_lf(buf, buf, n);
		else if (out->encoding == FLOAT && stream->alpha)
			raw2a_to_raw3a_f(buf, buf, n);
		else if (out->encoding == DOUBLE)
			raw2_to_raw3_lf(buf, buf, n);
		else if (out->encoding == FLOAT)
			raw2_to_raw3_f(buf, buf, n);
		encoding = out->encoding;
	} else if (stream->encoding == FLOAT && out->encoding == DOUBLE) {
		f_to_lf(buf, buf, n);
		encoding = out->encoding;
	} else if (stream->encoding == DOUBLE && out->encoding == FLOAT) {
		lf_to_f(buf, buf, n);
		encoding = out->encoding;
	}

	if (stream->alpha && !out->alpha) {
		if (encoding == DOUBLE)
			n = remove_alpha_lf(buf, n);
		else if (encoding == FLOAT)
			n = remove_alpha_f(buf, n);
		else
			n = remove_alpha_u16(buf, n);
	} else if (!stream->alpha && out->alpha) {
		if (encoding == DOUBLE)
			n = add_alpha_lf(buf, buf, n);
		else if (encoding == FLOAT)
			n = add_alpha_f(buf, buf, n);
		else
			n = add_alpha_u16(buf, buf, n);
	}

	if (stream->space == CIEXYZ && out->space == YUV_NONLINEAR) {
		conv_xyz_to_srgb(encoding, out->alpha, buf, n);
		conv_srgb_to_srgbt(encoding, out->alpha, buf, n);
		conv_srgb_to_yuv(encoding, out->alpha, buf, n);
	} else if (stream->space == CIEXYZ && out->space == SRGB_NONLINEAR) {
		conv_xyz_to_srgb(encoding, out->alpha, buf, n);
		conv_srgb_to_srgbt(encoding, out->alpha, buf, n);
	} else if (stream->space == CIEXYZ && out->space == SRGB) {
		conv_xyz_to_srgb(encoding, out->alpha, buf, n);
	} else if (stream->space == YUV_NONLINEAR && out->space == CIEXYZ) {
		conv_yuv_to_srgb(encoding, out->alpha, buf, n);
		conv_srgbt_to_srgb(encoding, out->alpha, buf, n);
		conv_srgb_to_xyz(encoding, out->alpha, buf, n);
	} else if (stream->space == YUV_NONLINEAR && out->space == SRGB_NONLINEAR) {
		conv_yuv_to_srgb(encoding, out->alpha, buf, n);
	} else if (stream->space == YUV_NONLINEAR && out->space == SRGB) {
		conv_yuv_to_srgb(encoding, out->alpha, buf, n);
		conv_srgbt_to_srgb(encoding, out->alpha, buf, n);
	} else if (stream->space == SRGB_NONLINEAR && out->space == CIEXYZ) {
		conv_srgbt_to_srgb(encoding, out->alpha, buf, n);
		conv_srgb_to_xyz(encoding, out->alpha, buf, n);
	} else if (stream->space == SRGB_NONLINEAR && out->space == YUV_NONLINEAR) {
		conv_srgb_to_yuv(encoding, out->alpha, buf, n);
	} else if (stream->space == SRGB_NONLINEAR && out->space == SRGB) {
		conv_srgbt_to_srgb(encoding, out->alpha, buf, n);
	} else if (stream->space == SRGB && out->space == CIEXYZ) {
		conv_srgb_to_xyz(encoding, out->alpha, buf, n);
	} else if (stream->space == SRGB && out->space == YUV_NONLINEAR) {
		conv_srgb_to_srgbt(encoding, out->alpha, buf, n);
		conv_srgb_to_yuv(encoding, out->alpha, buf, n);
	} else if (stream->space == SRGB && out->space == SRGB_NONLINEAR) {
		conv_srgb_to_srgbt(encoding, out->alpha, buf, n);
	}

	if (out_level <= 2 && in_level > 2) {
		if (encoding == DOUBLE && out->alpha)
			raw3a_to_raw2a_lf(buf, buf, n);
		else if (encoding == FLOAT && out->alpha)
			raw3a_to_raw2a_f(buf, buf, n);
		else if (encoding == DOUBLE)
			raw3_to_raw2_lf(buf, buf, n);
		else if (encoding == FLOAT)
			raw3_to_raw2_f(buf, buf, n);
	}

	if (out_level <= 1 && in_level > 1)
		h_to_le_16(buf, n);

	if (out_level <= 0 && in_level > 0)
		raw1_to_raw0(buf, n);

	ewriteall(STDOUT_FILENO, buf, n * out->chan_size, "<stdout>");
}

int
main(int argc, char *argv[])
{
	struct stream stream, out;
	const char *pixfmt;
	void *buf = NULL;
	size_t n;

	UNOFLAGS(!argc);

	eopen_stream(&stream, NULL);

	memcpy(&out, &stream, sizeof(out));
	pixfmt = stream.pixfmt;
	while (*argv)
		pixfmt = get_pixel_format(*argv++, pixfmt);
	strcpy(out.pixfmt, pixfmt);
	if (set_pixel_format(&out, NULL))
		eprintf("output pixel format %s is not supported\n", pixfmt);

	fprint_stream_head(stdout, &out);
	efflush(stdout, "<stdout>");

	if (!strcmp(stream.pixfmt, out.pixfmt)) {
		esend_stream(&stream, STDOUT_FILENO, "<stdout>");
		return 0;
	}

	if (stream.alpha_chan == 0)
		in_level = 0;
	else if (stream.endian == LITTLE)
		in_level = 1;
	else if (stream.encoding == UINT16)
		in_level = 2;
	if (out.alpha_chan == 0)
		out_level = 0;
	else if (out.endian == LITTLE)
		out_level = 1;
	else if (out.encoding == UINT16)
		out_level = 2;

	if (out.encoding == UINT16)
		out.encoding = stream.encoding;

	n = MAX(stream.chan_size, out.chan_size) * MAX(stream.n_chan, out.n_chan);
	if (n > stream.pixel_size)
		buf = emalloc(sizeof(stream.buf) / stream.chan_size * n);

	do {
		n = stream.ptr / stream.pixel_size * stream.n_chan;
		if (buf)
			memcpy(buf, stream.buf, n * stream.chan_size);
		convert(&stream, &out, buf ? buf : stream.buf, n);
		n *= stream.chan_size;
		memmove(stream.buf, stream.buf + n, stream.ptr -= n);
	} while (eread_stream(&stream, SIZE_MAX));
	if (stream.ptr)
		eprintf("%s: incomplete frame\n", stream.file);

	free(buf);
	return 0;
}
