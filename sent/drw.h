/* See LICENSE file for copyright and license details. */
#define DRW_FONT_CACHE_SIZE 32

typedef struct {
	Cursor cursor;
} Cur;

typedef struct Fnt Fnt;
struct Fnt {
	Display *dpy;
	int ascent;
	int descent;
	unsigned int h;
	XftFont *xfont;
	FcPattern *pattern;
	Fnt *next;
};

typedef struct {
	struct {
		unsigned long pix;
		XftColor rgb;
	} fg, bg;
} Scm;

typedef struct {
	unsigned int w, h;
	Display *dpy;
	int screen;
	Window root;
	Drawable drawable;
	GC gc;
	Scm *scheme;
	Fnt *fonts;
} Drw;

/* Drawable abstraction */
Drw *drw_create(Display *dpy, int screen, Window win, unsigned int w, unsigned int h);
void drw_resize(Drw *drw, unsigned int w, unsigned int h);
void drw_free(Drw *drw);

/* Fnt abstraction */
Fnt *drw_fontset_create(Drw* drw, const char *fonts[], size_t fontcount);
void drw_fontset_free(Fnt* set);
unsigned int drw_fontset_getwidth(Drw *drw, const char *text);
void drw_font_getexts(Fnt *font, const char *text, unsigned int len, unsigned int *w, unsigned int *h);

/* Colorscheme abstraction */
Scm *drw_scm_create(Drw *drw, const char *fgname, const char *bgname);
void drw_scm_free(Scm *scm);

/* Cursor abstraction */
Cur *drw_cur_create(Drw *drw, int shape);
void drw_cur_free(Drw *drw, Cur *cursor);

/* Drawing context manipulation */
void drw_setfontset(Drw *drw, Fnt *set);
void drw_setscheme(Drw *drw, Scm *scm);

/* Drawing functions */
void drw_rect(Drw *drw, int x, int y, unsigned int w, unsigned int h, int filled, int invert);
int drw_text(Drw *drw, int x, int y, unsigned int w, unsigned int h, const char *text, int invert);

/* Map functions */
void drw_map(Drw *drw, Window win, int x, int y, unsigned int w, unsigned int h);
