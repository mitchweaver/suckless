/* See LICENSE file for copyright and license details. */
#define INCLUDE_UINT16
#include "common.h"

/* Disable warnings in <math.h> */
#if defined(__clang__)
# pragma clang diagnostic ignored "-Wdouble-promotion"
# pragma clang diagnostic ignored "-Wconversion"
#endif

USAGE("[-s]")

static int strict = 1;

#define CONV(ITYPE, OTYPE, SOTYPE, EXPONENT, HA2EXPONENT, FRACTION)\
	do {\
		static int cache_i = 0;\
		static ITYPE cache_in[] = {0, 0, 0, 0};\
		static OTYPE cache_out[] = {0, 0, 0, 0};\
		OTYPE signb, fraction, ret;\
		SOTYPE exponent;\
		ITYPE u, dexponent;\
		if (host == cache_in[cache_i]) {\
			ret = cache_out[cache_i++];\
			cache_i &= 3;\
			return ret;\
		}\
		cache_in[cache_i] = host;\
		signb = (OTYPE)signbit(host);\
		u = signb ? -host : host;\
		if (g_isnan(host) || g_isinf(host)) {\
			ret = ((((OTYPE)1 << EXPONENT) - (OTYPE)1) << FRACTION) | (OTYPE)!g_isinf(host);\
		} else if (u == (ITYPE)0.0) {\
			ret = 0;\
		} else {\
			dexponent = log2(u);\
			exponent = (SOTYPE)dexponent;\
			if (u == pow((ITYPE)2.0, (ITYPE)exponent)) {\
				exponent += HA2EXPONENT;\
				fraction = 0;\
			} else {\
				/* TODO subnormals are a bit rounded off */\
				u *= pow((ITYPE)2.0, (ITYPE)(FRACTION + 1 - exponent));\
				fraction = (OTYPE)u;\
				while (fraction >= (OTYPE)2 << FRACTION) {\
					fraction >>= 1;\
					exponent += 1;\
				}\
				fraction &= ((OTYPE)1 << FRACTION) - 1;\
				exponent += HA2EXPONENT - 1;\
			}\
			if (exponent < 1) {\
				/* TODO subnormal result */\
				exponent = 0;\
				fraction = 0;\
			} else if (exponent >= ((SOTYPE)1 << EXPONENT) - 1) { \
				exponent = ((SOTYPE)1 << EXPONENT) - 1;\
				fraction = 0;\
			}\
			ret = ((OTYPE)exponent << FRACTION) + fraction;\
		}\
		ret |= signb << (FRACTION + EXPONENT);\
		cache_out[cache_i++] = ret;\
		cache_i &= 3;\
		return ret;\
	} while (0)

#define PROCESS_FLOAT(ITYPE, OTYPE, BITS)\
	do {\
		size_t i, n;\
		ITYPE *ibuf = (ITYPE *)(stream->buf);\
		OTYPE *obuf = sizeof(ITYPE) == sizeof(OTYPE) ? (OTYPE *)(stream->buf)\
			: alloca(sizeof(stream->buf) / sizeof(ITYPE) * sizeof(OTYPE));\
		strict *= !USING_BINARY##BITS;\
		if (!strict && sizeof(ITYPE) != sizeof(OTYPE))\
			eprintf("-s is supported not on this machine\n");\
		if (stream->endian == LITTLE && !strict) {\
			esend_stream(stream, STDOUT_FILENO, "<stdout>");\
			break;\
		}\
		do {\
			n = stream->ptr / sizeof(ITYPE);\
			if (strict) {\
				for (i = 0; i < n; i++)\
					obuf[i] = htole(conv_##ITYPE(ibuf[i]));\
			} else {\
				for (i = 0; i < n; i++)\
					obuf[i] = htole(*(OTYPE *)&ibuf[i]);\
			}\
			ewriteall(STDOUT_FILENO, obuf, n * sizeof(OTYPE), "<stdout>");\
			n *= sizeof(ITYPE);\
			memmove(stream->buf, stream->buf + n, stream->ptr -= n);\
		} while (eread_stream(stream, SIZE_MAX));\
		if (stream->ptr)\
			eprintf("%s: incomplete frame\n", stream->file);\
	} while (0)

#define PROCESS_INTEGER(TYPE)\
	do {\
		size_t i, n;\
		TYPE *buf = (TYPE *)(stream->buf);\
		if (stream->endian == LITTLE) {\
			esend_stream(stream, STDOUT_FILENO, "<stdout>");\
			break;\
		}\
		do {\
			n = stream->ptr / sizeof(TYPE);\
			for (i = 0; i < n; i++)\
				buf[i] = htole(buf[i]);\
			n *= sizeof(TYPE);\
			ewriteall(STDOUT_FILENO, buf, n, "<stdout>");\
			memmove(stream->buf, stream->buf + n, stream->ptr -= n);\
		} while (eread_stream(stream, SIZE_MAX));\
		if (stream->ptr)\
			eprintf("%s: incomplete frame\n", stream->file);\
	} while (0)

static uint64_t conv_double(double host) {CONV(double, uint64_t, int64_t, 11, 1023, 52);}
static uint32_t conv_float (float  host) {CONV(float,  uint32_t, int32_t,  8,  127, 23);}

static void process_lf (struct stream *stream) {PROCESS_FLOAT(double, uint64_t, 64);}
static void process_f  (struct stream *stream) {PROCESS_FLOAT(float,  uint32_t, 32);}
static void process_u16(struct stream *stream) {PROCESS_INTEGER(uint16_t);}

int
main(int argc, char *argv[])
{
	struct stream stream;
	void (*process)(struct stream *stream);

	ARGBEGIN {
	case 's':
		strict = 0;
		break;
	default:
		usage();
	} ARGEND;

	if (argc)
		usage();

	eopen_stream(&stream, NULL);
#if defined(HOST_ENDIAN_IS_LITTLE_ENDIAN)
	if (stream.endian == HOST)
		stream.endian = LITTLE;
#elif defined(HOST_ENDIAN_IS_BIG_ENDIAN)
	if (stream.endian == HOST)
		stream.endian = BIG;
#endif

	SELECT_PROCESS_FUNCTION(&stream);
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	process(&stream);
	return 0;
}
