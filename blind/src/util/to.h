/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdlib.h>

int tollu(const char *s, unsigned long long int min, unsigned long long int max, unsigned long long int *out);
int tolli(const char *s, long long int min, long long int max, long long int *out);
#define DEF_STR_TO_INT(FNAME, INTTYPE, INTER_FNAME, INTER_INTTYPE, PRI)\
	static inline int\
	FNAME(const char *s, INTTYPE min, INTTYPE max, INTTYPE *out)\
	{\
		INTER_INTTYPE inter;\
		if (INTER_FNAME(s, (INTER_INTTYPE)min, (INTER_INTTYPE)max, &inter))\
			return -1;\
		*out = (INTTYPE)inter;\
		return 0;\
	}
DEF_STR_TO_INT(tolu, unsigned long int, tollu, unsigned long long int, "lu")
DEF_STR_TO_INT(tou, unsigned int, tollu, unsigned long long int, "u")
DEF_STR_TO_INT(toli, long int, tolli, long long int, "li")
DEF_STR_TO_INT(toi, int, tolli, long long int, "i")
#undef DEF_STR_TO_INT
#define tozu tolu
#define tozi toli
#define toju tollu
#define toji tolli

#define DEF_STR_TO_INT(FNAME, TYPE, PRI)\
	static TYPE\
	en##FNAME##_flag(int status, int flag, const char *s, TYPE min, TYPE max)\
	{\
		TYPE ret = 0;\
		if (FNAME(s, min, max, &ret))\
			enprintf(status,\
				 "argument of -%c must be an integer in [%"PRI", %"PRI"]\n",\
				 flag, min, max);\
		return ret;\
	}\
	\
	static inline TYPE\
	e##FNAME##_flag(int flag, const char *s, TYPE min, TYPE max)\
	{\
		return en##FNAME##_flag(1, flag, s, min, max);\
	}\
	\
	static TYPE\
	en##FNAME##_arg(int status, const char *name, const char *s, TYPE min, TYPE max)\
	{\
		TYPE ret = 0;\
		if (FNAME(s, min, max, &ret))\
			enprintf(status,\
				 "%s must be an integer in [%"PRI", %"PRI"]\n",\
				 name, min, max);\
		return ret;\
	}\
	\
	static inline TYPE\
	e##FNAME##_arg(const char *name, const char *s, TYPE min, TYPE max)\
	{\
		return en##FNAME##_arg(1, name, s, min, max);\
	}
DEF_STR_TO_INT(tollu, unsigned long long int, "llu")
DEF_STR_TO_INT(tolu, unsigned long int, "lu")
DEF_STR_TO_INT(tou, unsigned int, "u")
DEF_STR_TO_INT(tolli, long long int, "lli")
DEF_STR_TO_INT(toli, long int, "li")
DEF_STR_TO_INT(toi, int, "i")
#undef DEF_STR_TO_INT

#define etozu_flag etolu_flag
#define etozi_flag etoli_flag
#define etoju_flag etollu_flag
#define etoji_flag etolli_flag
#define entozu_flag entolu_flag
#define entozi_flag entoli_flag
#define entoju_flag entollu_flag
#define entoji_flag entolli_flag

#define etozu_arg etolu_arg
#define etozi_arg etoli_arg
#define etoju_arg etollu_arg
#define etoji_arg etolli_arg
#define entozu_arg entolu_arg
#define entozi_arg entoli_arg
#define entoju_arg entollu_arg
#define entoji_arg entolli_arg

#define DEF_STR_TO_FLOAT(FNAME, TYPE, FUN)\
	static inline int\
	FNAME(const char *s, TYPE *out)\
	{\
		char *end;\
		errno = 0;\
		*out = FUN(s, &end);\
		if (errno) {\
			return -1;\
		} else if (*end) {\
			errno = EINVAL;\
			return -1;\
		}\
		return 0;\
	}\
	\
	static TYPE\
	en##FNAME##_flag(int status, int flag, const char *s)\
	{\
		TYPE ret = 0;\
		if (FNAME(s, &ret))\
			enprintf(status, "argument of -%c must be floating-point value\n", flag);\
		return ret;\
	}\
	\
	static inline TYPE\
	e##FNAME##_flag(int flag, const char *s)\
	{\
		return en##FNAME##_flag(1, flag, s);\
	}\
	\
	static TYPE\
	en##FNAME##_arg(int status, const char *name, const char *s)\
	{\
		TYPE ret = 0;\
		if (FNAME(s, &ret))\
			enprintf(status, "%s must be floating-point value\n", name);\
		return ret;\
	}\
	\
	static inline TYPE\
	e##FNAME##_arg(const char *name, const char *s)\
	{\
		return en##FNAME##_arg(1, name, s);\
	}
DEF_STR_TO_FLOAT(tof, float, strtof)
DEF_STR_TO_FLOAT(tolf, double, strtod)
DEF_STR_TO_FLOAT(tollf, long double, strtold)
#undef DEF_STR_TO_FLOAT
