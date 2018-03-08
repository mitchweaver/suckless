/* See LICENSE file for copyright and license details. */
#define INCLUDE_UINT16
#include "common.h"

USAGE("[-s]")

static int strict = 1;

#define CONV(ITYPE, SITYPE, OTYPE, EXPONENT, HA2EXPONENT, FRACTION)\
	do {\
		static int cache_i = 0;\
		static ITYPE cache_in[] = {0, 0, 0, 0};\
		static OTYPE cache_out[] = {0, 0, 0, 0};\
		ITYPE exponent, fraction, signb;\
		SITYPE sexponent;\
		OTYPE ret, dexponent;\
		if (portable == cache_in[cache_i]) {\
			ret = cache_out[cache_i++];\
			cache_i &= 3;\
			return ret;\
		}\
		cache_in[cache_i] = portable;\
		signb = portable >> (EXPONENT + FRACTION);\
		exponent = (portable >> FRACTION) ^ (signb << EXPONENT);\
		fraction = portable & (((ITYPE)1 << FRACTION) - 1);\
		if (!exponent) {\
			if (!fraction) {\
				ret = (OTYPE)0.0;\
			} else {\
				sexponent = 1 - HA2EXPONENT - FRACTION;\
				dexponent = (OTYPE)sexponent;\
				ret = (OTYPE)fraction;\
				ret *= pow((OTYPE)2.0, dexponent);\
			}\
		} else if (exponent + 1 == (ITYPE)1 << EXPONENT) {\
			ret = (OTYPE)(fraction ? NAN : INFINITY);\
		} else {\
			fraction |= (ITYPE)1 << FRACTION;\
			sexponent = (SITYPE)exponent;\
			sexponent -= HA2EXPONENT + FRACTION;\
			dexponent = (OTYPE)sexponent;\
			ret = (OTYPE)fraction;\
			ret *= pow((OTYPE)2.0, dexponent);\
		}\
		ret = signb ? -ret : ret;\
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
		if (stream->endian == HOST && !strict) {\
			esend_stream(stream, STDOUT_FILENO, "<stdout>");\
			break;\
		}\
		do {\
			n = stream->ptr / sizeof(ITYPE);\
			if (strict) {\
				for (i = 0; i < n; i++)\
					obuf[i] = conv_##OTYPE(letoh(ibuf[i]));\
			} else {\
				for (i = 0; i < n; i++)\
					obuf[i] = *(OTYPE *)&(ITYPE){letoh(ibuf[i])};\
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
		if (stream->endian == HOST) {\
			esend_stream(stream, STDOUT_FILENO, "<stdout>");\
			break;\
		}\
		do {\
			n = stream->ptr / sizeof(TYPE);\
			for (i = 0; i < n; i++)\
				buf[i] = letoh(buf[i]);\
			n *= sizeof(TYPE);\
			ewriteall(STDOUT_FILENO, buf, n, "<stdout>");\
			memmove(stream->buf, stream->buf + n, stream->ptr -= n);\
		} while (eread_stream(stream, SIZE_MAX));\
		if (stream->ptr)\
			eprintf("%s: incomplete frame\n", stream->file);\
	} while (0)

static double conv_double(uint64_t portable) {CONV(uint64_t, int64_t, double, 11, 1023, 52);}
static float  conv_float (uint32_t portable) {CONV(uint32_t, int32_t, float,   8,  127, 23);}

static void process_lf (struct stream *stream) {PROCESS_FLOAT(uint64_t, double, 64);}
static void process_f  (struct stream *stream) {PROCESS_FLOAT(uint32_t, float,  32);}
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
	if (stream.endian == LITTLE)
		stream.endian = HOST;
#elif defined(HOST_ENDIAN_IS_BIG_ENDIAN)
	if (stream.endian == BIG)
		stream.endian = HOST;
#endif

	SELECT_PROCESS_FUNCTION(&stream);
	fprint_stream_head(stdout, &stream);
	efflush(stdout, "<stdout>");
	process(&stream);
	return 0;
}
