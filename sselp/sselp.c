/* See LICENSE file for license details. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

static unsigned char *
getsel(unsigned long offset, unsigned long *len, unsigned long *remain) {
	Display *dpy;
	Atom utf8_string;
	Atom xa_clip_string;
	Window w;
	XEvent ev;
	Atom typeret;
	int format;
	unsigned char *data;
	unsigned char *result = NULL;

	dpy = XOpenDisplay(NULL);
	if(!dpy)
		return NULL;
	utf8_string = XInternAtom(dpy, "UTF8_STRING", False);
	xa_clip_string = XInternAtom(dpy, "_SSELP_STRING", False);
	w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 10, 10, 200, 200,
			1, CopyFromParent, CopyFromParent);
	XConvertSelection(dpy, XA_PRIMARY, utf8_string, xa_clip_string,
			w, CurrentTime);
	XFlush(dpy);
	XNextEvent(dpy, &ev);
	if(ev.type == SelectionNotify && ev.xselection.property != None) {
		XGetWindowProperty(dpy, w, ev.xselection.property, offset, 4096L, False,
				AnyPropertyType, &typeret, &format, len, remain, &data);
		if(*len) {
			result = malloc(sizeof(unsigned char) * *len);
			memcpy(result, data, *len);
		}
		XDeleteProperty(dpy, w, ev.xselection.property);
	}
	XDestroyWindow(dpy, w);
	XCloseDisplay(dpy);
	return result;
}

int
main(int argc, char **argv) {
	unsigned char *data;
	unsigned long i, offset, len, remain;

	if((argc > 1) && !strncmp(argv[1], "-v", 3)) {
		fputs("sselp-"VERSION", © 2006-2008 Anselm R Garbe\n", stdout);
		exit(EXIT_SUCCESS);
	}
	len = offset = remain = 0;
	do {
		data = getsel(offset, &len, &remain);
		for(i = 0; i < len; i++)
			putchar(data[i]);
		offset += len;
		free(data);
	}
	while(remain);
	if(offset)
		putchar('\n');
	return 0;
}
