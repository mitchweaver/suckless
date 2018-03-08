/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

#define UTF_INVALID 0xFFFD
#define UTF_SIZ 4

static void drw_xfont_free(Fnt *font);

static const unsigned char utfbyte[UTF_SIZ + 1] = {0x80,    0, 0xC0, 0xE0, 0xF0};
static const unsigned char utfmask[UTF_SIZ + 1] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
static const long utfmin[UTF_SIZ + 1] = {       0,    0,  0x80,  0x800,  0x10000};
static const long utfmax[UTF_SIZ + 1] = {0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF};

static long
utf8decodebyte(const char c, size_t *i) {
	for(*i = 0; *i < (UTF_SIZ + 1); ++(*i))
		if(((unsigned char)c & utfmask[*i]) == utfbyte[*i])
			return (unsigned char)c & ~utfmask[*i];
	return 0;
}

static size_t
utf8validate(long *u, size_t i) {
	if(!BETWEEN(*u, utfmin[i], utfmax[i]) || BETWEEN(*u, 0xD800, 0xDFFF))
		*u = UTF_INVALID;
	for(i = 1; *u > utfmax[i]; ++i)
		;
	return i;
}

static size_t
utf8decode(const char *c, long *u, size_t clen) {
	size_t i, j, len, type;
	long udecoded;

	*u = UTF_INVALID;
	if(!clen)
		return 0;
	udecoded = utf8decodebyte(c[0], &len);
	if(!BETWEEN(len, 1, UTF_SIZ))
		return 1;
	for(i = 1, j = 1; i < clen && j < len; ++i, ++j) {
		udecoded = (udecoded << 6) | utf8decodebyte(c[i], &type);
		if(type != 0)
			return j;
	}
	if(j < len)
		return 0;
	*u = udecoded;
	utf8validate(u, len);
	return len;
}

Drw *
drw_create(Display *dpy, int screen, Window root, unsigned int w, unsigned int h) {
	Drw *drw = (Drw *)calloc(1, sizeof(Drw));
	if(!drw)
		return NULL;
	drw->dpy = dpy;
	drw->screen = screen;
	drw->root = root;
	drw->w = w;
	drw->h = h;
	drw->drawable = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));
	drw->gc = XCreateGC(dpy, root, 0, NULL);
	XSetLineAttributes(dpy, drw->gc, 1, LineSolid, CapButt, JoinMiter);
	return drw;
}

void
drw_resize(Drw *drw, unsigned int w, unsigned int h) {
	if(!drw)
		return;
	drw->w = w;
	drw->h = h;
	if(drw->drawable != 0)
		XFreePixmap(drw->dpy, drw->drawable);
	drw->drawable = XCreatePixmap(drw->dpy, drw->root, w, h, DefaultDepth(drw->dpy, drw->screen));
}

void
drw_free(Drw *drw) {
	XFreePixmap(drw->dpy, drw->drawable);
	XFreeGC(drw->dpy, drw->gc);
	free(drw);
}

/* This function is an implementation detail. Library users should use
 * drw_fontset_create instead.
 */
static Fnt *
drw_font_xcreate(Drw *drw, const char *fontname, FcPattern *fontpattern) {
	Fnt *font;

	if (!(fontname || fontpattern))
		die("No font specified.\n");

	if (!(font = (Fnt *)calloc(1, sizeof(Fnt))))
		return NULL;

	if (fontname) {
		/* Using the pattern found at font->xfont->pattern does not yield the
		 * same substitution results as using the pattern returned by
		 * FcNameParse; using the latter results in the desired fallback
		 * behaviour whereas the former just results in
		 * missing-character-rectangles being drawn, at least with some fonts.
		 */
		if (!(font->xfont = XftFontOpenName(drw->dpy, drw->screen, fontname)) ||
		    !(font->pattern = FcNameParse((const FcChar8 *) fontname))) {
			if (font->xfont) {
				XftFontClose(drw->dpy, font->xfont);
				font->xfont = NULL;
			}
			fprintf(stderr, "error, cannot load font: '%s'\n", fontname);
		}
	} else if (fontpattern) {
		if (!(font->xfont = XftFontOpenPattern(drw->dpy, fontpattern))) {
			fprintf(stderr, "error, cannot load font pattern.\n");
		} else {
			font->pattern = NULL;
		}
	}

	if (!font->xfont) {
		free(font);
		return NULL;
	}

	font->ascent = font->xfont->ascent;
	font->descent = font->xfont->descent;
	font->h = font->ascent + font->descent;
	font->dpy = drw->dpy;
	return font;
}

void
drw_xfont_free(Fnt *font) {
	if(!font)
		return;
	if(font->pattern)
		FcPatternDestroy(font->pattern);
	XftFontClose(font->dpy, font->xfont);
	free(font);
}

Fnt*
drw_fontset_create(Drw* drw, const char *fonts[], size_t fontcount) {
	Fnt *ret = NULL;
	Fnt *cur = NULL;
	ssize_t i;
	for (i = fontcount - 1; i >= 0; i--) {
		if ((cur = drw_font_xcreate(drw, fonts[i], NULL))) {
			cur->next = ret;
			ret = cur;
		}
	}
	return ret;
}

void
drw_fontset_free(Fnt *font) {
	Fnt *nf = font;

	while ((font = nf)) {
		nf = font->next;
		drw_xfont_free(font);
	}
}

Scm *
drw_scm_create(Drw *drw, const char *fgname, const char *bgname) {
	Scm *scm;
	Colormap cmap;
	Visual *vis;

	if (!drw || !(scm = (Scm *)calloc(1, sizeof(Scm))))
		return NULL;

	cmap = DefaultColormap(drw->dpy, drw->screen);
	vis = DefaultVisual(drw->dpy, drw->screen);
	if (!XftColorAllocName(drw->dpy, vis, cmap, fgname, &scm->fg.rgb))
		die("error, cannot allocate color '%s'\n", fgname);
	if (!XftColorAllocName(drw->dpy, vis, cmap, bgname, &scm->bg.rgb))
		die("error, cannot allocate color '%s'\n", bgname);
	scm->fg.pix = scm->fg.rgb.pixel;
	scm->bg.pix = scm->bg.rgb.pixel;
	return scm;
}

void
drw_scm_free(Scm *scm) {
	if (scm)
		free(scm);
}

void
drw_setfontset(Drw *drw, Fnt *set) {
	if (drw)
		drw->fonts = set;
}

void
drw_setscheme(Drw *drw, Scm *scm) {
	if (drw && scm)
		drw->scheme = scm;
}

void
drw_rect(Drw *drw, int x, int y, unsigned int w, unsigned int h, int filled, int invert)
{
	if (!drw || !drw->scheme)
		return;
	XSetForeground(drw->dpy, drw->gc, invert ? drw->scheme->bg.pix : drw->scheme->fg.pix);
	if (filled)
		XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w, h);
	else
		XDrawRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w, h);
}

int
drw_text(Drw *drw, int x, int y, unsigned int w, unsigned int h, const char *text, int invert) {
	char buf[1024];
	int tx, ty, th;
	unsigned int ew;
	Colormap cmap;
	Visual *vis;
	XftDraw *d;
	Fnt *usedfont, *curfont, *nextfont;
	size_t i, len;
	int utf8strlen, utf8charlen, render;
	long utf8codepoint = 0;
	const char *utf8str;
	FcCharSet *fccharset;
	FcPattern *fcpattern;
	FcPattern *match;
	XftResult result;
	int charexists = 0;

	if (!(render = x || y || w || h)) {
		w = ~w;
	}

	if (!drw || !drw->scheme) {
		return 0;
	} else if (render) {
		XSetForeground(drw->dpy, drw->gc, invert ? drw->scheme->fg.pix : drw->scheme->bg.pix);
		XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w, h);
	}

	if (!text || !drw->fonts) {
		return 0;
	} else if (render) {
		cmap = DefaultColormap(drw->dpy, drw->screen);
		vis = DefaultVisual(drw->dpy, drw->screen);
		d = XftDrawCreate(drw->dpy, drw->drawable, vis, cmap);
	}

	usedfont = drw->fonts;
	while (1) {
		utf8strlen = 0;
		utf8str = text;
		nextfont = NULL;
		while (*text) {
			utf8charlen = utf8decode(text, &utf8codepoint, UTF_SIZ);
			for (curfont = drw->fonts; curfont; curfont = curfont->next) {
				charexists = charexists || XftCharExists(drw->dpy, curfont->xfont, utf8codepoint);
				if (charexists) {
					if (curfont == usedfont) {
						utf8strlen += utf8charlen;
						text += utf8charlen;
					} else {
						nextfont = curfont;
					}
					break;
				}
			}

			if (!charexists || nextfont) {
				break;
			} else {
				charexists = 0;
			}
		}

		if (utf8strlen) {
			drw_font_getexts(usedfont, utf8str, utf8strlen, &ew, NULL);
			/* shorten text if necessary */
			for(len = MIN(utf8strlen, (sizeof buf) - 1); len && (ew > w - drw->fonts->h || w < drw->fonts->h); len--)
				drw_font_getexts(usedfont, utf8str, len, &ew, NULL);

			if (len) {
				memcpy(buf, utf8str, len);
				buf[len] = '\0';
				if(len < utf8strlen)
					for(i = len; i && i > len - 3; buf[--i] = '.');

				if (render) {
					th = usedfont->ascent + usedfont->descent;
					ty = y + (h / 2) - (th / 2) + usedfont->ascent;
					tx = x + (h / 2);
					XftDrawStringUtf8(d, invert ? &drw->scheme->bg.rgb : &drw->scheme->fg.rgb, usedfont->xfont, tx, ty, (XftChar8 *)buf, len);
				}

				x += ew;
				w -= ew;
			}
		}

		if (!*text) {
			break;
		} else if (nextfont) {
			charexists = 0;
			usedfont = nextfont;
		} else {
			/* Regardless of whether or not a fallback font is found, the
			 * character must be drawn.
			 */
			charexists = 1;

			fccharset = FcCharSetCreate();
			FcCharSetAddChar(fccharset, utf8codepoint);

			if (!drw->fonts->pattern) {
				/* Refer to the comment in drw_font_xcreate for more
				 * information.
				 */
				die("The first font in the cache must be loaded from a font string.\n");
			}

			fcpattern = FcPatternDuplicate(drw->fonts->pattern);
			FcPatternAddCharSet(fcpattern, FC_CHARSET, fccharset);
			FcPatternAddBool(fcpattern, FC_SCALABLE, FcTrue);

			FcConfigSubstitute(NULL, fcpattern, FcMatchPattern);
			FcDefaultSubstitute(fcpattern);
			match = XftFontMatch(drw->dpy, drw->screen, fcpattern, &result);

			FcCharSetDestroy(fccharset);
			FcPatternDestroy(fcpattern);

			if (match) {
				usedfont = drw_font_xcreate(drw, NULL, match);
				if (usedfont && XftCharExists(drw->dpy, usedfont->xfont, utf8codepoint)) {
					for (curfont = drw->fonts; curfont->next; curfont = curfont->next)
						; /* just find the end of the linked list */
					curfont->next = usedfont;
				} else {
					drw_xfont_free(usedfont);
					usedfont = drw->fonts;
				}
			}
		}
	}

	if (render) {
		XftDrawDestroy(d);
	}

	return x;
}

void
drw_map(Drw *drw, Window win, int x, int y, unsigned int w, unsigned int h) {
	if(!drw)
		return;
	XCopyArea(drw->dpy, drw->drawable, win, drw->gc, x, y, w, h, x, y);
	XSync(drw->dpy, False);
}

unsigned int
drw_fontset_getwidth(Drw *drw, const char *text) {
	if (!drw || !drw->fonts || !text)
		return 0;
	return drw_text(drw, 0, 0, 0, 0, text, 0);
}

void
drw_font_getexts(Fnt *font, const char *text, unsigned int len, unsigned int *w, unsigned int *h) {
	XGlyphInfo ext;

	if(!font || !text)
		return;
	XftTextExtentsUtf8(font->dpy, font->xfont, (XftChar8 *)text, len, &ext);
	if (w)
		*w = ext.xOff;
	if (h)
		*h = font->h;
}

Cur *
drw_cur_create(Drw *drw, int shape) {
	Cur *cur;

	if(!drw || !(cur = (Cur *)calloc(1, sizeof(Cur))))
		return NULL;
	cur->cursor = XCreateFontCursor(drw->dpy, shape);
	return cur;
}

void
drw_cur_free(Drw *drw, Cur *cursor) {
	if(!drw || !cursor)
		return;
	XFreeCursor(drw->dpy, cursor->cursor);
	free(cursor);
}
