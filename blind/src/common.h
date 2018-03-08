/* See LICENSE file for copyright and license details. */
#if defined(__clang__)
# pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
# pragma clang diagnostic ignored "-Wcomma"
# pragma clang diagnostic ignored "-Wcast-align"
# pragma clang diagnostic ignored "-Wassign-enum"
# pragma clang diagnostic ignored "-Wfloat-equal"
# pragma clang diagnostic ignored "-Wformat-nonliteral"
# pragma clang diagnostic ignored "-Wcovered-switch-default"
# pragma clang diagnostic ignored "-Wfloat-conversion"
# pragma clang diagnostic ignored "-Wabsolute-value"
# pragma clang diagnostic ignored "-Wconditional-uninitialized"
# pragma clang diagnostic ignored "-Wunreachable-code-return"
#elif defined(__GNUC__)
# pragma GCC diagnostic ignored "-Wfloat-equal"
# pragma GCC diagnostic ignored "-Wunsafe-loop-optimizations"
# pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include "../build/platform.h"
#include "stream.h"
#include "util.h"
#include "video-math.h"

#include <arpa/inet.h>
#if defined(HAVE_EPOLL)
# include <sys/epoll.h>
#endif
#include <sys/mman.h>
#if defined(HAVE_SENDFILE)
# include <sys/sendfile.h>
#endif
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <alloca.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#ifndef CMSG_ALIGN
# ifdef __sun__
#  define CMSG_ALIGN _CMSG_DATA_ALIGN
# else
#  define CMSG_ALIGN(len) (((len) + sizeof(long) - 1) & ~(sizeof(long) - 1))
# endif
#endif

#ifndef CMSG_SPACE
# define CMSG_SPACE(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + CMSG_ALIGN(len))
#endif

#ifndef CMSG_LEN
# define CMSG_LEN(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + (len))
#endif

#if !defined(PIPE_BUF)
# define PIPE_BUF 512
#endif

#ifndef DONT_INCLUDE_FLOAT
# define SELECT_PROCESS_FUNCTION_FLOAT(stream) else if ((stream)->encoding == FLOAT) process = process_f
#else
# define SELECT_PROCESS_FUNCTION_FLOAT(stream) else if (0)
#endif

#ifndef DONT_INCLUDE_DOUBLE
# define SELECT_PROCESS_FUNCTION_DOUBLE(stream) else if ((stream)->encoding == DOUBLE) process = process_lf
#else
# define SELECT_PROCESS_FUNCTION_DOUBLE(stream) else if (0)
#endif

#ifdef INCLUDE_LONG_DOUBLE
# define SELECT_PROCESS_FUNCTION_LONG_DOUBLE(stream) else if ((stream)->encoding == LONG_DOUBLE) process = process_llf
#else
# define SELECT_PROCESS_FUNCTION_LONG_DOUBLE(stream) else if (0)
#endif

#ifdef INCLUDE_UINT8
# define SELECT_PROCESS_FUNCTION_UINT8(stream) else if ((stream)->encoding == UINT8) process = process_u8
#else
# define SELECT_PROCESS_FUNCTION_UINT8(stream) else if (0)
#endif

#ifdef INCLUDE_UINT16
# define SELECT_PROCESS_FUNCTION_UINT16(stream) else if ((stream)->encoding == UINT16) process = process_u16
#else
# define SELECT_PROCESS_FUNCTION_UINT16(stream) else if (0)
#endif

#ifdef INCLUDE_UINT32
# define SELECT_PROCESS_FUNCTION_UINT32(stream) else if ((stream)->encoding == UINT32) process = process_u32
#else
# define SELECT_PROCESS_FUNCTION_UINT32(stream) else if (0)
#endif

#ifdef INCLUDE_UINT64
# define SELECT_PROCESS_FUNCTION_UINT64(stream) else if ((stream)->encoding == UINT64) process = process_u64
#else
# define SELECT_PROCESS_FUNCTION_UINT64(stream) else if (0)
#endif

#define SELECT_PROCESS_FUNCTION(stream)\
	do {\
		if ((stream)->endian != HOST)\
			eprintf("pixel format %s is not supported, try xyza\n", (stream)->pixfmt);\
		SELECT_PROCESS_FUNCTION_FLOAT(stream);\
		SELECT_PROCESS_FUNCTION_DOUBLE(stream);\
		SELECT_PROCESS_FUNCTION_LONG_DOUBLE(stream);\
		SELECT_PROCESS_FUNCTION_UINT8(stream);\
		SELECT_PROCESS_FUNCTION_UINT16(stream);\
		SELECT_PROCESS_FUNCTION_UINT32(stream);\
		SELECT_PROCESS_FUNCTION_UINT64(stream);\
		else\
			eprintf("pixel format %s is not supported, try xyza\n", (stream)->pixfmt);\
	} while (0)

#define CHECK_ALPHA_CHAN(stream)\
	do {\
		if ((stream)->alpha_chan != 3)\
			eprintf("pixel format %s is not supported, try xyza\n", (stream)->pixfmt);\
	} while (0)

#define CHECK_CHANS(stream, ALPHA, LUMA)\
	do {\
		if (!(((stream)->alpha_chan ALPHA) && ((stream)->luma_chan LUMA)))\
			eprintf("pixel format %s is not supported, try xyza\n", (stream)->pixfmt);\
	} while (0)

#define CHECK_ALPHA(stream)\
	do {\
		if ((stream)->alpha != UNPREMULTIPLIED)\
			eprintf("pixel format %s is not supported, try xyza\n", (stream)->pixfmt);\
	} while (0)

#define CHECK_COLOUR_SPACE(stream, colour_space)\
	do {\
		if ((stream)->space != (colour_space))\
			eprintf("pixel format %s is not supported, try xyza\n", (stream)->pixfmt);\
	} while (0)

#define CHECK_N_CHAN(stream, low, high)\
	do {\
		if ((stream)->n_chan < (low) || (stream)->n_chan > (high))\
			eprintf("pixel format %s is not supported, try xyza\n", (stream)->pixfmt);\
	} while (0)
