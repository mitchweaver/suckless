/* (C)opyright MMIV-MMV Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <X11/Xlib.h>

int
main(int argc, char **argv) {
	int x, y;
	Display *dpy;

	if((argc == 2) && !strncmp(argv[1], "-v", 3)) {
		fputs("swarp-"VERSION", (C)opyright MMVI Anselm R. Garbe\n", stdout);
		exit(EXIT_SUCCESS);
	}
	if(argc != 3)
		goto Usage;
	if(!(dpy = XOpenDisplay(0))) {
		fputs("swarp: cannot open display\n", stderr);
		exit(EXIT_FAILURE);
	}
	if((sscanf(argv[1], "%d", &x) != 1) || (sscanf(argv[2], "%d", &y) != 1)) {
Usage:
		fputs("usage: swarp <x> <y> [-v]\n", stderr);
		exit(EXIT_FAILURE);
	}
	XWarpPointer(dpy, None, RootWindow(dpy, DefaultScreen(dpy)), 0, 0, 0, 0, x, y);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XCloseDisplay(dpy);
	return 0;
}
