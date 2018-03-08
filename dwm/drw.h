typedef struct {
	Cursor cursor;
} Cur;

typedef XftColor *Scm;

typedef struct {
	unsigned int w, h;
	Display *dpy;
	int screen;
	Window root;
	Drawable drawable;
	GC gc;
	Scm scheme;
} Drw;

/* Drawable abstraction */
Drw *drw_create(Display *dpy, int screen, Window win, unsigned int w, unsigned int h);
void drw_free(Drw *drw);

/* Colorscheme abstraction */
void drw_clr_create(Drw *drw, XftColor *dest, const char *clrname);
Scm drw_scm_create(Drw *drw, const char *clrnames[], size_t clrcount);

/* Cursor abstraction */
Cur *drw_cur_create(Drw *drw, int shape);
void drw_cur_free(Drw *drw, Cur *cursor);
