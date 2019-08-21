#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

Drw * drw_create(Display *dpy, int screen, Window root, unsigned int w, unsigned int h) {
	Drw *drw = ecalloc(1, sizeof(Drw));
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

void drw_free(Drw *drw) {
	XFreePixmap(drw->dpy, drw->drawable);
	XFreeGC(drw->dpy, drw->gc);
	free(drw);
}

void drw_clr_create(Drw *drw, XftColor *dest, const char *clrname) {
	if (!drw || !dest || !clrname) return;
	if (!XftColorAllocName(drw->dpy, DefaultVisual(drw->dpy, drw->screen), DefaultColormap(drw->dpy, drw->screen), clrname, dest))
		die("error, cannot allocate color '%s'", clrname);
}

/* Wrapper to create color schemes. The caller has to call free(3) on the
 * returned color scheme when done using it. */
Scm drw_scm_create(Drw *drw, const char *clrnames[], size_t clrcount) {
	size_t i;
	Scm ret;

	/* need at least two colors for a scheme */
	if (!drw || !clrnames || clrcount < 2 || !(ret = ecalloc(clrcount, sizeof(XftColor)))) return NULL;
	for (i = 0; i < clrcount; i++) drw_clr_create(drw, &ret[i], clrnames[i]);
	return ret;
}

Cur * drw_cur_create(Drw *drw, int shape) {
	Cur *cur;
	if (!drw || !(cur = ecalloc(1, sizeof(Cur)))) return NULL;
	cur->cursor = XCreateFontCursor(drw->dpy, shape);
	return cur;
}

void drw_cur_free(Drw *drw, Cur *cursor) {
	if (!cursor) return;
	XFreeCursor(drw->dpy, cursor->cursor);
	free(cursor);
}
