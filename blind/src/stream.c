/* See LICENSE file for copyright and license details. */
#include "common.h"

static inline int
get_dimension(int status, size_t *out, const char *s, const char *fname, const char *dim)
{
	char *end;
	errno = 0;
	*out = strtoul(s, &end, 10);
	if (errno == ERANGE && *s != '-')
		enprintf(status, "%s: video is too %s\n", fname, dim);
	return -(errno || *end);
}

static inline int
sread(int status, struct stream *stream)
{
	ssize_t r;
	r = read(stream->fd, stream->buf + stream->ptr, sizeof(stream->buf) - stream->ptr);
	if (r < 0)
		enprintf(status, "read %s:", stream->file);
	if (r == 0)
		return 0;
	stream->ptr += (size_t)r;
	return 1;
}

void
eninit_stream(int status, struct stream *stream)
{
	size_t n;
	char *p = NULL, *w, *h, *f;

	fadvise_sequential(stream->fd, 0, 0);

	if (stream->fd >= 0) {
		for (stream->ptr = 0; !p; p = memchr(stream->buf, '\n', stream->ptr))
			if (!sread(status, stream))
				goto bad_format;
	} else {
		p = memchr(stream->buf, '\n', stream->ptr);
	}

	*p = '\0';
	if (!(w = strchr(stream->buf, ' ')) ||
	    !(h = strchr(w + 1, ' ')) ||
	    !(f = strchr(h + 1, ' ')))
		goto bad_format;
	*w++ = *h++ = *f++ = '\0';

	if (strlen(f) >= sizeof(stream->pixfmt))
		goto bad_format;
	strcpy(stream->pixfmt, f);
	if (get_dimension(status, &stream->frames, stream->buf, stream->file, "long") ||
	    get_dimension(status, &stream->width,  w,           stream->file, "wide") ||
	    get_dimension(status, &stream->height, h,           stream->file, "tall"))
		goto bad_format;

	if (!stream->width)
		eprintf("%s: width is zero\n", stream->file);
	if (!stream->height)
		eprintf("%s: height is zero\n", stream->file);

	n = (size_t)(p - stream->buf) + 1;
	memmove(stream->buf, stream->buf + n, stream->ptr -= n);
	while (stream->ptr < 5)
		if (!sread(status, stream))
			goto bad_format;
	if (stream->buf[0] != '\0' ||
	    stream->buf[1] != 'u' || stream->buf[2] != 'i' ||
	    stream->buf[3] != 'v' || stream->buf[4] != 'f')
		goto bad_format;
	memmove(stream->buf, stream->buf + 5, stream->ptr -= 5);
	stream->headlen = n + 5;

	enset_pixel_format(status, stream, NULL);

	stream->xptr = 0;

	return;
bad_format:
	enprintf(status, "%s: file format not supported\n", stream->file);
}


void
enopen_stream(int status, struct stream *stream, const char *file)
{
	stream->file = file ? file : "<stdin>";
	stream->fd = file ? enopen(status, file, O_RDONLY) : STDIN_FILENO;
	eninit_stream(status, stream);
}


int
set_pixel_format(struct stream *stream, const char *pixfmt)
{
#define TEST_ENCODING_AGNOSTIC(FMT) (!strcmp(stream->pixfmt, FMT) || !strcmp(stream->pixfmt, FMT" f"))

	if (pixfmt) {
		pixfmt = get_pixel_format(pixfmt, stream->pixfmt[0] ? stream->pixfmt : "xyza");
		if (strlen(pixfmt) >= sizeof(stream->pixfmt))
			return -1;
		strcpy(stream->pixfmt, pixfmt);
	}

	stream->n_chan = 4;
	stream->alpha = UNPREMULTIPLIED;
	stream->encoding = DOUBLE;
	stream->endian = HOST;
	stream->alpha_chan = 3;
	stream->luma_chan = -1;

	if (!strcmp(stream->pixfmt, "xyza")) {
		stream->space = CIEXYZ;
	} else if (!strcmp(stream->pixfmt, "xyza f")) {
		stream->space = CIEXYZ;
		stream->encoding = FLOAT;
	} else if (!strcmp(stream->pixfmt, "raw0")) {
		stream->space = YUV_NONLINEAR;
		stream->encoding = UINT16;
		stream->endian = LITTLE;
		stream->alpha_chan = 0;
		stream->luma_chan = 1;
	} else if (!strcmp(stream->pixfmt, "raw1")) {
		stream->space = YUV_NONLINEAR;
		stream->encoding = UINT16;
		stream->endian = LITTLE;
	} else if (!strcmp(stream->pixfmt, "raw2a") || !strcmp(stream->pixfmt, "raw2")) {
		stream->space = YUV_NONLINEAR;
		stream->alpha = stream->pixfmt[4] == 'a' ? UNPREMULTIPLIED : NO_ALPHA;
		stream->encoding = UINT16;
	} else if (TEST_ENCODING_AGNOSTIC("raw3") || TEST_ENCODING_AGNOSTIC("raw3a")) {
		stream->space = YUV_NONLINEAR;
		stream->alpha = stream->pixfmt[4] == 'a' ? UNPREMULTIPLIED : NO_ALPHA;
		stream->encoding = strlen(stream->pixfmt) > 5 ? FLOAT : DOUBLE;
	} else if (TEST_ENCODING_AGNOSTIC("raw4") || TEST_ENCODING_AGNOSTIC("raw4a")) {
		stream->space = SRGB_NONLINEAR;
		stream->alpha = stream->pixfmt[4] == 'a' ? UNPREMULTIPLIED : NO_ALPHA;
		stream->encoding = strlen(stream->pixfmt) > 5 ? FLOAT : DOUBLE;
	} else if (TEST_ENCODING_AGNOSTIC("raw5") || TEST_ENCODING_AGNOSTIC("raw5a")) {
		stream->space = SRGB;
		stream->alpha = stream->pixfmt[4] == 'a' ? UNPREMULTIPLIED : NO_ALPHA;
		stream->encoding = strlen(stream->pixfmt) > 5 ? FLOAT : DOUBLE;
	} else {
		return -1;
	}

	if (stream->alpha == NO_ALPHA) {
		stream->n_chan -= 1;
		stream->alpha_chan = -1;
	}

	if (stream->luma_chan == -1) {
		if (stream->space == CIEXYZ)
			stream->luma_chan = 1;
		else if (stream->space == YUV_NONLINEAR)
			stream->luma_chan = 0;
	}

	switch (stream->encoding) {
	case FLOAT:
		stream->chan_size = sizeof(float);
		break;
	case DOUBLE:
		stream->chan_size = sizeof(double);
		break;
	case LONG_DOUBLE:
		stream->chan_size = sizeof(long double);
		break;
	case UINT8:
		stream->chan_size = sizeof(uint8_t);
		break;
	case UINT16:
		stream->chan_size = sizeof(uint16_t);
		break;
	case UINT32:
		stream->chan_size = sizeof(uint32_t);
		break;
	case UINT64:
		stream->chan_size = sizeof(uint64_t);
		break;
	default:
		abort();
	}

	stream->pixel_size = stream->n_chan * stream->chan_size;
	stream->row_size   = stream->pixel_size * stream->width;
	stream->col_size   = stream->pixel_size * stream->height;
	stream->frame_size = stream->pixel_size * stream->height * stream->width;
	return 0;

#undef TEST_ENCODING_AGNOSTIC
}

void
enset_pixel_format(int status, struct stream *stream, const char *pixfmt)
{
	if (set_pixel_format(stream, pixfmt)) {
		if (pixfmt)
			enprintf(status, "pixel format %s is not supported, try xyza\n", pixfmt);
		else
			enprintf(status, "%s: unsupported pixel format: %s\n",
			         stream->file, stream->pixfmt);
	}
}


void
fprint_stream_head(FILE *fp, struct stream *stream)
{
	FPRINTF_HEAD(fp, stream->frames, stream->width, stream->height, stream->pixfmt);
}


int
dprint_stream_head(int fd, struct stream *stream)
{
	return DPRINTF_HEAD(fd, stream->frames, stream->width, stream->height, stream->pixfmt);
}


size_t
enread_stream(int status, struct stream *stream, size_t n)
{
	ssize_t r = read(stream->fd, stream->buf + stream->ptr,
			 MIN(sizeof(stream->buf) - stream->ptr, n));
	if (r < 0)
		enprintf(status, "read %s:", stream->file);
	stream->ptr += (size_t)r;
	return (size_t)r;
}


void
eninf_check_fd(int status, int fd, const char *file)
{
	struct stat st;
	if (fstat(fd, &st))
		enprintf(status, "fstat %s:", file);
	if (S_ISREG(st.st_mode))
		enprintf(status, "%s is a regular file, refusing infinite write\n", file);
}


void
encheck_dimensions(int status, const struct stream *stream, enum dimension dimensions, const char *prefix)
{
	size_t n;

	if (!stream->pixel_size)
		enprintf(status, "%s: %s%svideo frame doesn't have a pixel size\n",
			 stream->file, prefix ? prefix : "",
			 (prefix && *prefix) ? " " : "");

	n = SIZE_MAX / stream->pixel_size;

	if ((dimensions & WIDTH) && stream->width > n)
		enprintf(status, "%s: %s%svideo frame is too wide\n",
			 stream->file, prefix ? prefix : "",
			 (prefix && *prefix) ? " " : "");

	if ((dimensions & HEIGHT) && stream->height > n)
		enprintf(status, "%s: %s%svideo frame is too wide\n",
			 stream->file, prefix ? prefix : "",
			 (prefix && *prefix) ? " " : "");

	if (!stream->width || !stream->height)
		return;

	if ((dimensions & (WIDTH | HEIGHT)) == (WIDTH | HEIGHT)) {
		if (stream->width > n / stream->height)
			enprintf(status, "%s: %s%svideo frame is too large\n",
				 stream->file, prefix ? prefix : "",
				 (prefix && *prefix) ? " " : "");
	}

	if (!(dimensions & LENGTH))
		return;
	if (dimensions & WIDTH)
		n /= stream->width;
	if (dimensions & HEIGHT)
		n /= stream->height;

	if (stream->frames > n)
		enprintf(status, "%s: %s%svideo is too large\n",
			 stream->file, prefix ? prefix : "",
			 (prefix && *prefix) ? " " : "");
}


void
encheck_compat(int status, const struct stream *a, const struct stream *b)
{
	if (a->width != b->width || a->height != b->height)
		enprintf(status, "videos do not have the same geometry\n");
	if (strcmp(a->pixfmt, b->pixfmt))
		enprintf(status, "videos use incompatible pixel formats\n");
}


const char *
get_pixel_format(const char *specified, const char *current)
{
	enum colour_space space = CIEXYZ;
	enum alpha alpha = UNPREMULTIPLIED;
	enum encoding encoding = UINT16;
	int level = -1;
	size_t n = strlen(specified);

	if ((n >= 2 && !strcmp(specified - 2, " f")) ||
	    !strcmp(specified, "raw0") || !strcmp(specified, "raw1") ||
	    !strcmp(specified, "raw2") || !strcmp(specified, "raw2a"))
		return specified;

	if      (!strcmp(current, "xyza"))    space = CIEXYZ, encoding = DOUBLE;
	else if (!strcmp(current, "xyza f"))  space = CIEXYZ, encoding = FLOAT;
	else if (!strcmp(current, "raw0"))    level = 0;
	else if (!strcmp(current, "raw1"))    level = 1;
	else if (!strcmp(current, "raw2"))    level = 2, alpha = NO_ALPHA;
	else if (!strcmp(current, "raw2a"))   level = 2;
	else if (!strcmp(current, "raw3"))    level = 3, encoding = DOUBLE, alpha = NO_ALPHA;
	else if (!strcmp(current, "raw3a"))   level = 3, encoding = DOUBLE;
	else if (!strcmp(current, "raw3 f"))  level = 3, encoding = FLOAT, alpha = NO_ALPHA;
	else if (!strcmp(current, "raw3a f")) level = 3, encoding = FLOAT;
	else if (!strcmp(current, "raw4"))    level = 4, encoding = DOUBLE, alpha = NO_ALPHA;
	else if (!strcmp(current, "raw4a"))   level = 4, encoding = DOUBLE;
	else if (!strcmp(current, "raw4 f"))  level = 4, encoding = FLOAT, alpha = NO_ALPHA;
	else if (!strcmp(current, "raw4a f")) level = 4, encoding = FLOAT;
	else if (!strcmp(current, "raw5"))    level = 5, encoding = DOUBLE, alpha = NO_ALPHA;
	else if (!strcmp(current, "raw5a"))   level = 5, encoding = DOUBLE;
	else if (!strcmp(current, "raw5 f"))  level = 5, encoding = FLOAT, alpha = NO_ALPHA;
	else if (!strcmp(current, "raw5a f")) level = 5, encoding = FLOAT;
	else
		return specified;

	if      (!strcmp(specified, "f"))        encoding = FLOAT;
	else if (!strcmp(specified, "!f"))       encoding = DOUBLE;
	else if (!strcmp(specified, "xyza"))     level = -1, alpha = UNPREMULTIPLIED, space = CIEXYZ;
	else if (!strcmp(specified, "raw3"))     level = 3, alpha = NO_ALPHA;
	else if (!strcmp(specified, "raw3a"))    level = 3, alpha = UNPREMULTIPLIED;
	else if (!strcmp(specified, "raw4"))     level = 4, alpha = NO_ALPHA;
	else if (!strcmp(specified, "raw4a"))    level = 4, alpha = UNPREMULTIPLIED;
	else if (!strcmp(specified, "raw5"))     level = 5, alpha = NO_ALPHA;
	else if (!strcmp(specified, "raw5a"))    level = 5, alpha = UNPREMULTIPLIED;
	else if (!strcmp(specified, "xyza !f"))  return "xyza";
	else if (!strcmp(specified, "raw3 !f"))  return "raw3";
	else if (!strcmp(specified, "raw3a !f")) return "raw3a";
	else if (!strcmp(specified, "raw4 !f"))  return "raw4";
	else if (!strcmp(specified, "raw4a !f")) return "raw4a";
	else if (!strcmp(specified, "raw5 !f"))  return "raw5";
	else if (!strcmp(specified, "raw5a !f")) return "raw5a";
	else
		return specified;

	if      (level == 0 && encoding == UINT16) return "raw0";
	else if (level == 1 && encoding == UINT16) return "raw1";
	else if (level == 2 && encoding == UINT16) return alpha ? "raw2a"   : "raw2";
	else if (level == 3 && encoding == DOUBLE) return alpha ? "raw3a"   : "raw3";
	else if (level == 3 && encoding == FLOAT)  return alpha ? "raw3a f" : "raw3 f";
	else if (level == 4 && encoding == DOUBLE) return alpha ? "raw4a"   : "raw4";
	else if (level == 4 && encoding == FLOAT)  return alpha ? "raw4a f" : "raw4 f";
	else if (level == 5 && encoding == DOUBLE) return alpha ? "raw5a"   : "raw5";
	else if (level == 5 && encoding == FLOAT)  return alpha ? "raw5a f" : "raw5 f";
	else if (level < 0 && space == CIEXYZ && alpha == UNPREMULTIPLIED)
		return encoding == FLOAT ? "xyza f" : encoding == DOUBLE ? "xyza" : specified;
	else
		return specified;
}


const char *
nselect_print_format(int status, const char *format, enum encoding encoding, const char *fmt)
{
	static char retbuf[512];
	int with_plus = 0, inttyped = -1;
	const char *f = "", *orig = fmt;
	char *proto = alloca((fmt ? strlen(fmt) : 0) + sizeof("%+#.50llx")), *p;
	char *ret = retbuf;
	size_t n, len;

	if (!orig)
		goto check_done;

	for (; *fmt == '+'; fmt++)
		with_plus = 1;
	f = fmt + strspn(fmt, "0123456789");
	if (f[0] && f[1])
		enprintf(status, "invalid format: %s\n", orig);

	switch (*f) {
	case '\0':
		inttyped = -1;
		break;
	case 'd': case 'i':
		inttyped = 1;
		break;
	case 'a': case 'A':
	case 'e': case 'E':
	case 'f': case 'F':
	case 'g': case 'G':
		inttyped = 0;
		break;
	default:
		enprintf(status, "invalid format: %s\n", orig);
	}

	switch (encoding) {
	case FLOAT:
	case DOUBLE:
	case LONG_DOUBLE:
		if (inttyped == 1)
			enprintf(status, "invalid format `%s' is incompatible with the video format\n", orig);
		inttyped = 0;
		break;
	case UINT8:
	case UINT16:
	case UINT32:
	case UINT64:
		if (*f != *fmt)
			enprintf(status, "invalid format: %s\n", orig);
		if (inttyped == 0)
			enprintf(status, "invalid format `%s' is incompatible with the video format\n", orig);
		inttyped = 1;
		break;
	default:
		abort();
	}
check_done:

	p = proto;
	*p++ = '%';
	if (with_plus)
		*p++ = '+';

	if (orig && *f != *fmt) {
		*p++ = '.';
		p = stpncpy(p, fmt, (size_t)(f - fmt));
	} else if (orig && inttyped && *f != 'a' && *f != 'A') {
		*p++ = '.';
		*p++ = '2';
		*p++ = '5';
	}

	inttyped = 1;
	switch (encoding) {
	case FLOAT:
		inttyped = 0;
		break;
	case DOUBLE:
		*p++ = 'l';
		inttyped = 0;
		break;
	case LONG_DOUBLE:
		*p++ = 'L';
		inttyped = 0;
		break;
	case UINT8:
		fmt = PRIi8;
		break;
	case UINT16:
		fmt = PRIi16;
		break;
	case UINT32:
		fmt = PRIi32;
		break;
	case UINT64:
		fmt = PRIi64;
		break;
	default:
		abort();
	}

	if (inttyped)
		while (*fmt == 'l' || *fmt == 'L')
			*p++ = *fmt++;

	switch (orig ? *f : '\0') {
	case '\0':
		*p++ = inttyped ? 'i' : 'f';
		break;
	case 'd': case 'i':
		*p++ = 'i';
		break;
	case 'a': case 'A':
		*p++ = 'a';
		break;
	case 'e': case 'E':
		*p++ = 'e';
		break;
	case 'f': case 'F':
		*p++ = 'f';
		break;
	case 'g': case 'G':
		*p++ = 'g';
		break;
	}

	*p = '\0';

	len = strlen(proto);
	for (n = 1, f = format; *f; f++) {
		if (f[0] == '%' && f[1] == '!') {
			f++;
			n += len;
		} else {
			n++;
		}
	}

	if (n > sizeof(retbuf))
		ret = enmalloc(status, n);
	for (p = ret, f = format; *f; f++) {
		if (f[0] == '%' && f[1] == '!') {
			f++;
			p = stpcpy(p, proto);
		} else {
			*p++ = *f;
		}
	}

	return ret;
}


int
enread_segment(int status, struct stream *stream, void *buf, size_t n)
{
	char *buffer = buf;
	ssize_t r;
	size_t m;

	if (stream->ptr) {
		m = MIN(stream->ptr, n);
		memcpy(buffer + stream->xptr, stream->buf, m);
		memmove(stream->buf, stream->buf + m, stream->ptr -= m);
		stream->xptr += m;
	}

	for (; stream->xptr < n; stream->xptr += (size_t)r) {
		r = read(stream->fd, buffer + stream->xptr, n - stream->xptr);
		if (r < 0) {
			enprintf(status, "read %s:", stream->file);
		} else if (r == 0) {
			if (!stream->xptr)
				break;
			enprintf(status, "%s: incomplete frame", stream->file);
		}
	}

	if (!stream->xptr)
		return 0;
	stream->xptr -= n;
	return 1;
}


size_t
ensend_frames(int status, struct stream *stream, int outfd, size_t frames, const char *outfname)
{
	size_t h, w, p, n, ret;

	for (ret = 0; ret < frames; ret++) {
		for (p = stream->pixel_size; p; p--) {
			for (h = stream->height; h; h--) {
				for (w = stream->width; w; w -= n) {
					if (!stream->ptr && !enread_stream(status, stream, w))
						goto done;
					n = MIN(stream->ptr, w);
					if (outfd >= 0)
						enwriteall(status, outfd, stream->buf, n, outfname);
					memmove(stream->buf, stream->buf + n, stream->ptr -= n);
				}
			}
		}
	}

	return ret;
done:
	if (p != stream->pixel_size || h != stream->height || w != stream->width)
		enprintf(status, "%s: incomplete frame", stream->file);
	return ret;
}


size_t
ensend_rows(int status, struct stream *stream, int outfd, size_t rows, const char *outfname)
{
	size_t w, p, n, ret;

	for (ret = 0; ret < rows; ret++) {
		for (p = stream->pixel_size; p; p--) {
			for (w = stream->width; w; w -= n) {
				if (!stream->ptr && !enread_stream(status, stream, w))
					goto done;
				n = MIN(stream->ptr, w);
				if (outfd >= 0)
					enwriteall(status, outfd, stream->buf, n, outfname);
				memmove(stream->buf, stream->buf + n, stream->ptr -= n);
			}
		}
	}

	return ret;
done:
	if (p != stream->pixel_size || w != stream->width)
		enprintf(status, "%s: incomplete row", stream->file);
	return ret;
}


size_t
ensend_pixels(int status, struct stream *stream, int outfd, size_t pixels, const char *outfname)
{
	size_t p, n, ret;

	for (ret = 0; ret < pixels; ret++) {
		for (p = stream->pixel_size; p; p -= n) {
			if (!stream->ptr && !enread_stream(status, stream, p))
				goto done;
			n = MIN(stream->ptr, p);
			if (outfd >= 0)
				enwriteall(status, outfd, stream->buf, n, outfname);
			memmove(stream->buf, stream->buf + n, stream->ptr -= n);
		}
	}

	return ret;
done:
	if (p != stream->pixel_size)
		enprintf(status, "%s: incomplete pixel", stream->file);
	return ret;
}


int
ensend_stream(int status, struct stream *stream, int outfd, const char *outfname)
{
	do {
		if (writeall(outfd, stream->buf, stream->ptr)) {
			if (outfname)
				eprintf("write %s:", outfname);
			return -1;
		}
		stream->ptr = 0;
	} while (enread_stream(status, stream, SIZE_MAX));
	return 0;
}


void
nprocess_stream(int status, struct stream *stream, void (*process)(struct stream *stream, size_t n))
{
	size_t n;
	do {
		n = stream->ptr - (stream->ptr % stream->pixel_size);
		process(stream, n);
		memmove(stream->buf, stream->buf + n, stream->ptr -= n);
	} while (enread_stream(status, stream, SIZE_MAX));
}


void
nprocess_each_frame_segmented(int status, struct stream *stream, int output_fd, const char* output_fname,
			      void (*process)(struct stream *stream, size_t n, size_t frame))
{
	size_t frame, r, n;
	encheck_dimensions(status, stream, WIDTH | HEIGHT, NULL);
	for (frame = 0; frame < stream->frames; frame++) {
		for (n = stream->frame_size; n; n -= r) {
			if (stream->ptr < n && !enread_stream(status, stream, SIZE_MAX))
				enprintf(status, "%s: file is shorter than expected\n", stream->file);
			r = stream->ptr - (stream->ptr % stream->pixel_size);
			r = MIN(r, n);
			process(stream, r, frame);
			enwriteall(status, output_fd, stream->buf, r, output_fname);
			memmove(stream->buf, stream->buf + r, stream->ptr -= r);
		}
	}
}


void
nprocess_two_streams(int status, struct stream *left, struct stream *right, int output_fd, const char* output_fname,
		     void (*process)(struct stream *left, struct stream *right, size_t n))
{
	size_t n;
	int have_both = 1;

	encheck_compat(status, left, right);

	while (have_both) {
		if (left->ptr < sizeof(left->buf) && !enread_stream(status, left, SIZE_MAX)) {
			close(left->fd);
			left->fd = -1;
			have_both = 0;
		}
		if (right->ptr < sizeof(right->buf) && !enread_stream(status, right, SIZE_MAX)) {
			close(right->fd);
			right->fd = -1;
			have_both = 0;
		}

		n = MIN(left->ptr, right->ptr);
		n -= n % left->pixel_size;
		left->ptr -= n;
		right->ptr -= n;

		process(left, right, n);

		enwriteall(status, output_fd, left->buf, n, output_fname);
		if ((n & 3) || left->ptr != right->ptr) {
			memmove(left->buf,  left->buf  + n, left->ptr);
			memmove(right->buf, right->buf + n, right->ptr);
		}
	}

	if (right->fd >= 0)
		close(right->fd);

	enwriteall(status, output_fd, left->buf, left->ptr, output_fname);

	if (left->fd >= 0) {
		for (;;) {
			left->ptr = 0;
			if (!enread_stream(status, left, SIZE_MAX)) {
				close(left->fd);
				left->fd = -1;
				break;
			}
			enwriteall(status, output_fd, left->buf, left->ptr, output_fname);
		}
	}
}


void
nprocess_multiple_streams(int status, struct stream *streams, size_t n_streams, int output_fd, const char* output_fname,
			  int shortest, void (*process)(struct stream *streams, size_t n_streams, size_t n))
{
	size_t closed, i, j, n;

	for (i = 1; i < n_streams; i++)
		encheck_compat(status, streams + i, streams);

	while (n_streams) {
		n = SIZE_MAX;
		for (i = 0; i < n_streams; i++) {
			if (streams[i].ptr < streams->pixel_size && !enread_stream(status, streams + i, SIZE_MAX)) {
				close(streams[i].fd);
				streams[i].fd = -1;
				if (shortest)
					return;
			}
			if (streams[i].ptr && streams[i].ptr < n)
				n = streams[i].ptr;
		}
		if (n == SIZE_MAX)
			break;
		n -= n % streams->pixel_size;

		process(streams, n_streams, n);
		enwriteall(status, output_fd, streams->buf, n, output_fname);

		closed = SIZE_MAX;
		for (i = 0; i < n_streams; i++) {
			if (streams[i].ptr)
				memmove(streams[i].buf, streams[i].buf + n, streams[i].ptr -= n);
			if (streams[i].ptr < streams->pixel_size && streams[i].fd < 0 && closed == SIZE_MAX)
				closed = i;
		}
		if (closed != SIZE_MAX) {
			for (i = (j = closed) + 1; i < n_streams; i++)
				if (streams[i].ptr >= streams->pixel_size || streams[i].fd >= 0)
					streams[j++] = streams[i];
			n_streams = j;
		}
	}
}


void
nprocess_each_frame_two_streams(int status, struct stream *left, struct stream *right, int output_fd, const char* output_fname,
				void (*process)(char *restrict output, char *restrict lbuf, char *restrict rbuf,
						struct stream *left, struct stream *right))
{
	char *lbuf, *rbuf, *image;

	encheck_dimensions(status, left,  WIDTH | HEIGHT, NULL);
	encheck_dimensions(status, right, WIDTH | HEIGHT, NULL);

	if (left->frame_size > SIZE_MAX - left->frame_size ||
	    2 * left->frame_size > SIZE_MAX - right->frame_size)
		enprintf(status, "video frame is too large\n");

	image = mmap(0, 2 * left->frame_size + right->frame_size,
		     PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
	if (image == MAP_FAILED)
		enprintf(status, "mmap:");
	lbuf = image + 1 * left->frame_size;
	rbuf = image + 2 * left->frame_size;

	for (;;) {
		if (!enread_frame(status, left, lbuf)) {
			close(left->fd);
			left->fd = -1;
			break;
		}
		if (!enread_frame(status, right, rbuf)) {
			close(right->fd);
			right->fd = -1;
			break;
		}

		process(image, lbuf, rbuf, left, right);
		enwriteall(status, output_fd, image, left->frame_size, output_fname);
	}

	if (right->fd >= 0)
		close(right->fd);

	if (left->fd >= 0) {
		memcpy(image, lbuf, left->ptr);
		while (enread_frame(status, left, lbuf))
			enwriteall(status, output_fd, image, left->frame_size, output_fname);
	}

	munmap(image, 2 * left->frame_size + right->frame_size);
}
