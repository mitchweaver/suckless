/* See LICENSE file for copyright and license details. */
#include <math.h>

static inline double
nnpow_d(double a, double b)
{
	int neg = a < 0;
	a = pow(neg ? -a : a, b);
	return neg ? -a : a;
}

static inline float
nnpow_f(float a, float b)
{
	int neg = a < 0;
	a = powf(neg ? -a : a, b);
	return neg ? -a : a;
}

static inline double
posmod_d(double a, double b)
{
	double x = fmod(a, b);
	return x < 0 ? x + b : x;
}

static inline float
posmod_f(float a, float b)
{
	float x = fmodf(a, b);
	return x < 0 ? x + b : x;
}

static inline double
degsin_d(double u)
{
	if (!fmod(u, 90)) {
		int64_t v = (int64_t)u;
		v = ((v / 90) % 4 + 4) % 4;
		return ((double[]){0, 1, 0, -1})[v];
	}
	return sin(u * (M_PI / 180));
}

static inline float
degsin_f(float u)
{
	if (!fmodf(u, 90)) {
		int64_t v = (int64_t)u;
		v = ((v / 90) % 4 + 4) % 4;
		return ((float[]){0, 1, 0, -1})[v];
	}
	return sinf(u * (float)(M_PI / 180));
}

static inline double
degcos_d(double u)
{
	if (!fmod(u, 90)) {
		int64_t v = (int64_t)u;
		v = ((v / 90) % 4 + 4) % 4;
		return ((double[]){1, 0, -1, 0})[v];
	}
	return cos(u * (M_PI / 180));
}

static inline float
degcos_f(float u)
{
	if (!fmodf(u, 90)) {
		int64_t v = (int64_t)u;
		v = ((v / 90) % 4 + 4) % 4;
		return ((float[]){1, 0, -1, 0})[v];
	}
	return cosf(u * (float)(M_PI / 180));
}

#define GENERIC(TYPE, FUNC, ...)\
	TYPE:           FUNC(__VA_ARGS__),\
	TYPE *:         FUNC(__VA_ARGS__),\
	TYPE **:        FUNC(__VA_ARGS__),\
	TYPE ***:       FUNC(__VA_ARGS__),\
	const TYPE:     FUNC(__VA_ARGS__),\
	const TYPE *:   FUNC(__VA_ARGS__),\
	const TYPE **:  FUNC(__VA_ARGS__),\
	const TYPE ***: FUNC(__VA_ARGS__)

#define MATH_GENERIC_1(FUNC, A)       (_Generic((A),\
						GENERIC(double, FUNC,    A),\
						GENERIC(float,  FUNC##f, A)))

#define MATH_GENERIC_N(FUNC, A, ...)  (_Generic((A),\
						GENERIC(double, FUNC,    A, __VA_ARGS__),\
						GENERIC(float,  FUNC##f, A, __VA_ARGS__)))

#define BLIND_GENERIC_1(FUNC, A)      (_Generic((A),\
						GENERIC(double, FUNC##_d, A),\
						GENERIC(float,  FUNC##_f, A)))

#define BLIND_GENERIC_N(FUNC, A, ...) (_Generic((A),\
						GENERIC(double, FUNC##_d, A, __VA_ARGS__), \
						GENERIC(float,  FUNC##_f, A, __VA_ARGS__)))

#define pow(...)        MATH_GENERIC_N(pow,      __VA_ARGS__)
#define log2(...)       MATH_GENERIC_1(log2,     __VA_ARGS__)
#define log(...)        MATH_GENERIC_1(log,      __VA_ARGS__)
#define abs(...)        MATH_GENERIC_1(fabs,     __VA_ARGS__)
#define sqrt(...)       MATH_GENERIC_1(sqrt,     __VA_ARGS__)
#define exp(...)        MATH_GENERIC_1(exp,      __VA_ARGS__)
#define g_isnan(...)    MATH_GENERIC_1(isnan,    __VA_ARGS__)
#define g_isinf(...)    MATH_GENERIC_1(isinf,    __VA_ARGS__)
#define g_isfinite(...) MATH_GENERIC_1(isfinite, __VA_ARGS__)
#define mod(...)        MATH_GENERIC_N(fmod,     __VA_ARGS__)
#define cos(...)        MATH_GENERIC_1(cos,      __VA_ARGS__)
#define sin(...)        MATH_GENERIC_1(sin,      __VA_ARGS__)
#define tan(...)        MATH_GENERIC_1(tan,      __VA_ARGS__)
#define atan2(...)      MATH_GENERIC_N(atan2,    __VA_ARGS__)

#define nnpow(...)       BLIND_GENERIC_N(nnpow,       __VA_ARGS__)
#define posmod(...)      BLIND_GENERIC_N(posmod,      __VA_ARGS__)
#define degcos(...)      BLIND_GENERIC_1(degcos,      __VA_ARGS__)
#define degsin(...)      BLIND_GENERIC_1(degsin,      __VA_ARGS__)
#define srgb_encode(...) BLIND_GENERIC_1(srgb_encode, __VA_ARGS__)
#define srgb_decode(...) BLIND_GENERIC_1(srgb_decode, __VA_ARGS__)

#define yuv_to_srgb(a, b, c, d, e, f)\
	BLIND_GENERIC_N(yuv_to_srgb, (a), (b), (c), (void *)(d), (void *)(e), (void *)(f))
#define srgb_to_yuv(a, b, c, d, e, f)\
	BLIND_GENERIC_N(srgb_to_yuv, (a), (b), (c), (void *)(d), (void *)(e), (void *)(f))
#define ciexyz_to_srgb(a, b, c, d, e, f)\
	BLIND_GENERIC_N(ciexyz_to_srgb, (a), (b), (c), (void *)(d), (void *)(e), (void *)(f))
#define srgb_to_ciexyz(a, b, c, d, e, f)\
	BLIND_GENERIC_N(srgb_to_ciexyz, (a), (b), (c), (void *)(d), (void *)(e), (void *)(f))
#define scaled_yuv_to_ciexyz(a, b, c, d, e, f)\
	BLIND_GENERIC_N(scaled_yuv_to_ciexyz, (a), (b), (c), (void *)(d), (void *)(e), (void *)(f))
#define ciexyz_to_scaled_yuv(a, b, c, d, e, f)\
	BLIND_GENERIC_N(ciexyz_to_scaled_yuv, (a), (b), (c), (void *)(d), (void *)(e), (void *)(f))

#define htole(A) (_Generic((A),\
			   uint8_t: (A),\
			    int8_t: (uint8_t)(A),\
			   uint16_t: htole16((uint16_t)(A)),\
			    int16_t: (uint16_t)htole16((uint16_t)(A)),\
			   uint32_t: htole32((uint32_t)(A)),\
			    int32_t: (uint32_t)htole32((uint32_t)(A)),\
			   uint64_t: htole64((uint64_t)(A)),\
			    int64_t: (uint64_t)htole64((uint64_t)(A))))

#define letoh(A) (_Generic((A),\
			   uint8_t: (A),\
			    int8_t: (uint8_t)(A),\
			   uint16_t: le16toh((uint16_t)(A)),\
			    int16_t: (uint16_t)le16toh((uint16_t)(A)),\
			   uint32_t: le32toh((uint32_t)(A)),\
			    int32_t: (uint32_t)le32toh((uint32_t)(A)),\
			   uint64_t: le64toh((uint64_t)(A)),\
			    int64_t: (uint64_t)le64toh((uint64_t)(A))))
