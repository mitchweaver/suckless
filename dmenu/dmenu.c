#include <ctype.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif
#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

/* macros */
#define INTERSECT(x,y,w,h,r)  (MAX(0, MIN((x)+(w),(r).x_org+(r).width)  - MAX((x),(r).x_org)) \
                             * MAX(0, MIN((y)+(h),(r).y_org+(r).height) - MAX((y),(r).y_org)))
#define LENGTH(X)             (sizeof X / sizeof X[0])
#define TEXTW(X)              (drw_fontset_getwidth(drw, (X)) + lrpad)

/* enums */
enum { SchemeNorm, SchemeSel, SchemeOut, SchemeLast }; /* color schemes */

struct item {
	char *text;
	struct item *left, *right;
	int out;
	double distance;
};

static char text[BUFSIZ] = "";
static char *embed;
static int bh, mw, mh;
//  ------ xyw patch -----
static int dmx = 0; /* put dmenu at this x offset */
static int dmy = 0; /* put dmenu at this y offset (measured from the bottom if topbar is 0) */
static unsigned int dmw = 0; /* make dmenu this wide */
// ---------------------
static int inputw = 0, promptw;
static int lrpad; /* sum of left and right padding */
static size_t cursor;
static struct item *items = NULL;
static struct item *matches, *matchend;
static struct item *prev, *curr, *next, *sel;
static int mon = -1, screen;

static Atom clip, utf8;
static Display *dpy;
static Window root, parentwin, win;
static XIC xic;

static Drw *drw;
static Clr *scheme[SchemeLast];

#include "config.h"

static int (*fstrncmp)(const char *, const char *, size_t) = strncmp;
static char *(*fstrstr)(const char *, const char *) = strstr;

static void appenditem(struct item *item, struct item **list, struct item **last) {
	if (*last) (*last)->right = item;
	else *list = item;
	item->left = *last;
	item->right = NULL;
	*last = item;
}

static void calcoffsets(void) {
	int i, n;

	if (lines > 0) n = lines * bh;
	else n = mw - (promptw + inputw + TEXTW("<") + TEXTW(">"));
	/* calculate which items will begin the next page and previous page */
	for (i = 0, next = curr; next; next = next->right)
		if ((i += (lines > 0) ? bh : MIN(TEXTW(next->text), n)) > n)
			break;
	for (i = 0, prev = curr; prev && prev->left; prev = prev->left)
		if ((i += (lines > 0) ? bh : MIN(TEXTW(prev->left->text), n)) > n) break;
}

static void cleanup(void) {
	size_t i;

	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	for (i = 0; i < SchemeLast; i++) free(scheme[i]);
	drw_free(drw);
	XSync(dpy, False);
	XCloseDisplay(dpy);
}

static char * cistrstr(const char *s, const char *sub) {
	size_t len;

	for (len = strlen(sub); *s; s++)
		if (!strncasecmp(s, sub, len))
			return (char *)s;
	return NULL;
}

static int drawitem(struct item *item, int x, int y, int w) {
	if (item == sel) drw_setscheme(drw, scheme[SchemeSel]);
	else if (item->out) drw_setscheme(drw, scheme[SchemeOut]);
	else drw_setscheme(drw, scheme[SchemeNorm]);
	return drw_text(drw, x, y, w, bh, lrpad / 2, item->text, 0);
}

static void drawmenu(void) {
	unsigned int curpos;
	struct item *item;
	int x = 0, y = 0, w;

	drw_setscheme(drw, scheme[SchemeNorm]);
	drw_rect(drw, 0, 0, mw, mh, 1, 1);

	if (prompt && *prompt) {
		drw_setscheme(drw, scheme[SchemeSel]);
		x = drw_text(drw, x, 0, promptw, bh, lrpad / 2, prompt, 0);
	}
	/* draw input field */
	w = (lines > 0 || !matches) ? mw - x : inputw;
	drw_setscheme(drw, scheme[SchemeNorm]);
	drw_text(drw, x, 0, w, bh, lrpad / 2, text, 0);

	drw_font_getexts(drw->fonts, text, cursor, &curpos, NULL);
	if ((curpos += lrpad / 2 - 1) < w) {
		drw_setscheme(drw, scheme[SchemeNorm]);
		drw_rect(drw, x + curpos, 2, 2, bh - 4, 1, 0);
	}

	if (lines > 0) {
		/* draw vertical list */
		for (item = curr; item != next; item = item->right)
			drawitem(item, x, y += bh, mw - x);
	} else if (matches) {
		/* draw horizontal list */
		x += inputw;
		w = TEXTW("<");
		if (curr->left) {
			drw_setscheme(drw, scheme[SchemeNorm]);
			drw_text(drw, x, 0, w, bh, lrpad / 2, "<", 0);
		}
		x += w;
		for (item = curr; item != next; item = item->right)
			x = drawitem(item, x, 0, MIN(TEXTW(item->text), mw - x - TEXTW(">")));
		if (next) {
			w = TEXTW(">");
			drw_setscheme(drw, scheme[SchemeNorm]);
			drw_text(drw, mw - w, 0, w, bh, lrpad / 2, ">", 0);
		}
	}
	drw_map(drw, win, 0, 0, mw, mh);
}

static void grabfocus(void) {
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 10000000  };
	Window focuswin;
	int i, revertwin;

	for (i = 0; i < 100; ++i) {
		XGetInputFocus(dpy, &focuswin, &revertwin);
		if (focuswin == win)
			return;
		XSetInputFocus(dpy, win, RevertToParent, CurrentTime);
		nanosleep(&ts, NULL);
	}
	die("cannot grab focus");
}

static void grabkeyboard(void) {
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 1000000  };
	int i;
	if (embed) return;
	/* try to grab keyboard, we may have to wait for another process to ungrab */
	for (i = 0; i < 1000; i++) {
		if (XGrabKeyboard(dpy, DefaultRootWindow(dpy), True, GrabModeAsync,
		                  GrabModeAsync, CurrentTime) == GrabSuccess)
			return;
		nanosleep(&ts, NULL);
	}
	die("cannot grab keyboard");
}

int compare_distance(const void *a, const void *b) {
	struct item *da = *(struct item **) a;
	struct item *db = *(struct item **) b;
	if (!db) return 1;
	if (!da) return -1;
	return da->distance == db->distance ? 0 : da->distance < db->distance ? -1 : 1;
}

void fuzzymatch(void) {
	struct item *it;
	struct item **fuzzymatches = NULL;
	char c;
	int number_of_matches = 0, i, pidx, sidx, eidx;
	int text_len = strlen(text), itext_len;

	matches = matchend = NULL;

	/* walk through all items */
	for (it = items; it && it->text; it++) {
		if (text_len) {
			itext_len = strlen(it->text);
			pidx = 0; /* pointer */
			sidx = eidx = -1; /* start of match, end of match */
			/* walk through item text */
			for (i = 0; i < itext_len && (c = it->text[i]); i++) {
        /* -------- case insensitive patch ------------------------------------------------- */ 
				/* if (!fstrncmp(&text[pidx], &c, 1)) { */
				if (tolower(text[pidx]) == c || toupper(text[pidx]) == c) {
        /* -------------------------------------------------------------------------- */ 
					if(sidx == -1) sidx = i;
					pidx++;
					if (pidx == text_len) {
						eidx = i;
						break;
					}
				}
			}
			/* build list of matches */
			if (eidx != -1) {
				/* compute distance */
				/* add penalty if match starts late (log(sidx+2))
				 * add penalty for long a match without many matching characters */
				it->distance = log(sidx + 2) + (double)(eidx - sidx - text_len);
				/* fprintf(stderr, "distance %s %f\n", it->text, it->distance); */
				appenditem(it, &matches, &matchend);
				number_of_matches++;
			}
		} else appenditem(it, &matches, &matchend);
	}

	if (number_of_matches) {
		/* initialize array with matches */
		if (!(fuzzymatches = realloc(fuzzymatches, number_of_matches * sizeof(struct item*))))
			die("cannot realloc %u bytes:", number_of_matches * sizeof(struct item*));
		for (i = 0, it = matches; it && i < number_of_matches; i++, it = it->right) {
			fuzzymatches[i] = it;
		}
		/* sort matches according to distance */
		qsort(fuzzymatches, number_of_matches, sizeof(struct item*), compare_distance);
		/* rebuild list of matches */
		matches = matchend = NULL;
		for (i = 0, it = fuzzymatches[i];  i < number_of_matches && it && \
				it->text; i++, it = fuzzymatches[i]) {
			appenditem(it, &matches, &matchend);
		}
		free(fuzzymatches);
	}
	curr = sel = matches;
	calcoffsets();
}

static void match(void) {
	if (fuzzy) { fuzzymatch(); return; }
	static char **tokv = NULL;
	static int tokn = 0;

	char buf[sizeof text], *s;
	int i, tokc = 0;
	size_t len, textsize;
	struct item *item, *lprefix, *lsubstr, *prefixend, *substrend;

	strcpy(buf, text);
	/* separate input text into tokens to be matched individually */
	for (s = strtok(buf, " "); s; tokv[tokc - 1] = s, s = strtok(NULL, " "))
		if (++tokc > tokn && !(tokv = realloc(tokv, ++tokn * sizeof *tokv)))
			die("cannot realloc %u bytes:", tokn * sizeof *tokv);
	len = tokc ? strlen(tokv[0]) : 0;

	matches = lprefix = lsubstr = matchend = prefixend = substrend = NULL;
	textsize = strlen(text) + 1;
	for (item = items; item && item->text; item++) {
		for (i = 0; i < tokc; i++)
			if (!fstrstr(item->text, tokv[i]))
				break;
		if (i != tokc) /* not all tokens match */
			continue;
		/* exact matches go first, then prefixes, then substrings */
		if (!tokc || !fstrncmp(text, item->text, textsize))
			appenditem(item, &matches, &matchend);
		else if (!fstrncmp(tokv[0], item->text, len))
			appenditem(item, &lprefix, &prefixend);
		else appenditem(item, &lsubstr, &substrend);
	}
	if (lprefix) {
		if (matches) {
			matchend->right = lprefix;
			lprefix->left = matchend;
		} else matches = lprefix;
		matchend = prefixend;
	}
	if (lsubstr) {
		if (matches) {
			matchend->right = lsubstr;
			lsubstr->left = matchend;
		} else matches = lsubstr;
		matchend = substrend;
	}
	curr = sel = matches;
	calcoffsets();
}

static void insert(const char *str, ssize_t n) {
	if (strlen(text) + n > sizeof text - 1)
		return;
	/* move existing text out of the way, insert new text, and update cursor */
	memmove(&text[cursor + n], &text[cursor], sizeof text - cursor - MAX(n, 0));
	if (n > 0) memcpy(&text[cursor], str, n);
	cursor += n;
	match();
}

static size_t nextrune(int inc) {
	ssize_t n;
	/* return location of next utf8 rune in the given direction (+1 or -1) */
	for (n = cursor + inc; n + inc >= 0 && (text[n] & 0xc0) == 0x80; n += inc)
		;
	return n;
}

static void movewordedge(int dir) {
	if (dir < 0) { /* move cursor to the start of the word*/
		while (cursor > 0 && strchr(worddelimiters, text[nextrune(-1)]))
			cursor = nextrune(-1);
		while (cursor > 0 && !strchr(worddelimiters, text[nextrune(-1)]))
			cursor = nextrune(-1);
	} else { /* move cursor to the end of the word */
		while (text[cursor] && strchr(worddelimiters, text[cursor]))
			cursor = nextrune(+1);
		while (text[cursor] && !strchr(worddelimiters, text[cursor]))
			cursor = nextrune(+1);
	}
}

static void keypress(XKeyEvent *ev) {
	char buf[32];
	int len;
	KeySym ksym = NoSymbol;
	Status status;

	len = XmbLookupString(xic, ev, buf, sizeof buf, &ksym, &status);
	if (status == XBufferOverflow) return;
	if (ev->state & ControlMask)
		switch(ksym) {
		case XK_a: ksym = XK_Home;      break;
		case XK_b: ksym = XK_Left;      break;
		case XK_c: ksym = XK_Escape;    break;
		case XK_d: ksym = XK_Delete;    break;
		case XK_e: ksym = XK_End;       break;
		case XK_f: ksym = XK_Right;     break;
		case XK_g: ksym = XK_Escape;    break;
		case XK_h: ksym = XK_BackSpace; break;
		case XK_i: ksym = XK_Tab;       break;
		case XK_j: /* fallthrough */
		case XK_J: /* fallthrough */
		case XK_m: /* fallthrough */
		case XK_M: ksym = XK_Return; ev->state &= ~ControlMask; break;
		case XK_n: ksym = XK_Down;      break;
		case XK_p: ksym = XK_Up;        break;
		case XK_k: /* delete right */
			text[cursor] = '\0';
			match();
			break;
		case XK_u: /* delete left */
			insert(NULL, 0 - cursor);
			break;
		case XK_w: /* delete word */
			while (cursor > 0 && strchr(worddelimiters, text[nextrune(-1)]))
				insert(NULL, nextrune(-1) - cursor);
			while (cursor > 0 && !strchr(worddelimiters, text[nextrune(-1)]))
				insert(NULL, nextrune(-1) - cursor);
			break;
		case XK_y: /* paste selection */
		case XK_Y:
			XConvertSelection(dpy, (ev->state & ShiftMask) ? clip : XA_PRIMARY,
			                  utf8, utf8, win, CurrentTime);
			return;
		case XK_Left:
			movewordedge(-1);
			ksym = NoSymbol;
			break;
		case XK_Right:
			movewordedge(+1);
			ksym = NoSymbol;
			break;

		case XK_Return:
		case XK_KP_Enter: break;
		case XK_bracketleft:
			cleanup();
			exit(1);
		default: return;
		}
	else if (ev->state & Mod1Mask)
		switch(ksym) {
		case XK_b:
			movewordedge(-1);
			ksym = NoSymbol;
			break;
		case XK_f:
			movewordedge(+1);
			ksym = NoSymbol;
			break;
		case XK_g: ksym = XK_Home;  break;
		case XK_G: ksym = XK_End;   break;
		case XK_h: ksym = XK_Up;    break;
		case XK_j: ksym = XK_Next;  break;
		case XK_k: ksym = XK_Prior; break;
		case XK_l: ksym = XK_Down;  break;
		default:
			return;
		}
	switch(ksym) {
	default:
		if (!iscntrl(*buf))
			insert(buf, len);
		break;
	case NoSymbol:
		break;
	case XK_Delete:
		if (text[cursor] == '\0')
			return;
		cursor = nextrune(+1);
		/* fallthrough */
	case XK_BackSpace:
		if (cursor == 0)
			return;
		insert(NULL, nextrune(-1) - cursor);
		break;
	case XK_End:
		if (text[cursor] != '\0') {
			cursor = strlen(text);
			break;
		}
		if (next) {
			/* jump to end of list and position items in reverse */
			curr = matchend;
			calcoffsets();
			curr = prev;
			calcoffsets();
			while (next && (curr = curr->right))
				calcoffsets();
		}
		sel = matchend;
		break;
	case XK_Escape:
		cleanup();
		exit(1);
	case XK_Home:
		if (sel == matches) {
			cursor = 0;
			break;
		}
		sel = curr = matches;
		calcoffsets();
		break;
	case XK_Left:
		if (cursor > 0 && (!sel || !sel->left || lines > 0)) {
			cursor = nextrune(-1);
			break;
		}
		if (lines > 0)
			return;
		/* fallthrough */
	case XK_Up:
		if (sel && sel->left && (sel = sel->left)->right == curr) {
			curr = prev;
			calcoffsets();
		}
		break;
	case XK_Next:
		if (!next)
			return;
		sel = curr = next;
		calcoffsets();
		break;
	case XK_Prior:
		if (!prev)
			return;
		sel = curr = prev;
		calcoffsets();
		break;
	case XK_Return:
	case XK_KP_Enter:
		puts((sel && !(ev->state & ShiftMask)) ? sel->text : text);
		if (!(ev->state & ControlMask)) {
			cleanup();
			exit(0);
		}
		if (sel) sel->out = 1;
		break;
	case XK_Right:
		if (text[cursor] != '\0') {
			cursor = nextrune(+1);
			break;
		}
		if (lines > 0)
			return;
		/* fallthrough */
	case XK_Down:
		if (sel && sel->right && (sel = sel->right) == next) {
			curr = next;
			calcoffsets();
		}
		break;
	case XK_Tab:
		if (!sel) return;
		strncpy(text, sel->text, sizeof text - 1);
		text[sizeof text - 1] = '\0';
		cursor = strlen(text);
		match();
		break;
	}
	drawmenu();
}

static void paste(void) {
	char *p, *q;
	int di;
	unsigned long dl;
	Atom da;
	/* we have been given the current selection, now insert it into input */
	XGetWindowProperty(dpy, win, utf8, 0, (sizeof text / 4) + 1, False,
	                   utf8, &da, &di, &dl, &dl, (unsigned char **)&p);
	insert(p, (q = strchr(p, '\n')) ? q - p : (ssize_t)strlen(p));
	XFree(p);
	drawmenu();
}

static void readstdin(void) {
	char buf[sizeof text], *p;
	size_t i, imax = 0, size = 0;
	unsigned int tmpmax = 0;

	/* read each line from stdin and add it to the item list */
	for (i = 0; fgets(buf, sizeof buf, stdin); i++) {
		if (i + 1 >= size / sizeof *items)
			if (!(items = realloc(items, (size += BUFSIZ))))
				die("cannot realloc %u bytes:", size);
		if ((p = strchr(buf, '\n'))) *p = '\0';
		if (!(items[i].text = strdup(buf)))
			die("cannot strdup %u bytes:", strlen(buf) + 1);
		items[i].out = 0;
		drw_font_getexts(drw->fonts, buf, strlen(buf), &tmpmax, NULL);
		if (tmpmax > inputw) {
			inputw = tmpmax;
			imax = i;
		}
	}
	if (items) items[i].text = NULL;
	inputw = items ? TEXTW(items[imax].text) : 0;
	lines = MIN(lines, i);
}

static void run(void) {
	XEvent ev;

	while (!XNextEvent(dpy, &ev)) {
		if (XFilterEvent(&ev, win)) continue;
		switch(ev.type) {
		case Expose:
			if (ev.xexpose.count == 0)
				drw_map(drw, win, 0, 0, mw, mh);
			break;
		case FocusIn:
			/* regrab focus from parent window */
			if (ev.xfocus.window != win) grabfocus();
			break;
		case KeyPress: keypress(&ev.xkey); break;
		case SelectionNotify:
			if (ev.xselection.property == utf8) paste();
			break;
		case VisibilityNotify:
			if (ev.xvisibility.state != VisibilityUnobscured)
				XRaiseWindow(dpy, win);
			break;
		}
	}
}

static void setup(void) {
	int x, y, i = 0;
	unsigned int du;
	XSetWindowAttributes swa;
	XIM xim;
	Window w, dw, *dws;
	XWindowAttributes wa;
#ifdef XINERAMA
	XineramaScreenInfo *info;
	Window pw;
	int a, j, di, n, area = 0;
#endif

	/* init appearance */
	scheme[SchemeNorm] = drw_scm_create(drw, colors[SchemeNorm], 2);
	scheme[SchemeSel] = drw_scm_create(drw, colors[SchemeSel], 2);
	scheme[SchemeOut] = drw_scm_create(drw, colors[SchemeOut], 2);

	clip = XInternAtom(dpy, "CLIPBOARD",   False);
	utf8 = XInternAtom(dpy, "UTF8_STRING", False);

	/* calculate menu geometry */
	bh = drw->fonts->h + 2;
	lines = MAX(lines, 0);
	mh = (lines + 1) * bh;
#ifdef XINERAMA
	if (parentwin == root && (info = XineramaQueryScreens(dpy, &n))) {
		XGetInputFocus(dpy, &w, &di);
		if (mon >= 0 && mon < n)
			i = mon;
		else if (w != root && w != PointerRoot && w != None) {
			/* find top-level window containing current input focus */
			do {
				if (XQueryTree(dpy, (pw = w), &dw, &w, &dws, &du) && dws)
					XFree(dws);
			} while (w != root && w != pw);
			/* find xinerama screen with which the window intersects most */
			if (XGetWindowAttributes(dpy, pw, &wa))
				for (j = 0; j < n; j++)
					if ((a = INTERSECT(wa.x, wa.y, wa.width, wa.height, info[j])) > area) {
						area = a;
						i = j;
					}
		}
		/* no focused window is on screen, so use pointer location instead */
		if (mon < 0 && !area && XQueryPointer(dpy, root, &dw, &dw, &x, &y, &di, &di, &du))
			for (i = 0; i < n; i++)
				if (INTERSECT(x, y, 1, 1, info[i]))
					break;

// ----------- xyw patch ------------------------------------- //
		/* x = info[i].x_org; */
		/* y = info[i].y_org + (topbar ? 0 : info[i].height - mh); */
		/* mw = info[i].width; */
		x = info[i].x_org + dmx;
		y = info[i].y_org + (topbar ? dmy : info[i].height - mh - dmy);
		mw = (dmw>0 ? dmw : info[i].width);
// ---------------------------------------------------------- //
		XFree(info);
	} else
#endif
	{
		if (!XGetWindowAttributes(dpy, parentwin, &wa))
			die("could not get embedding window attributes: 0x%lx",
			    parentwin);

// ----------- xyw patch ----------------------------------- //
		x = dmx;
		y = topbar ? dmy : wa.height - mh - dmy;
		mw = (dmw>0 ? dmw : wa.width);
		/* x = 0; */
		/* y = topbar ? 0 : wa.height - mh; */
		/* mw = wa.width; */
// --------------------------------------------------------- //
	}
	promptw = (prompt && *prompt) ? TEXTW(prompt) - lrpad / 4 : 0;
	inputw = MIN(inputw, mw/3);
	match();

	/* create menu window */
	swa.override_redirect = True;
	swa.background_pixel = scheme[SchemeNorm][ColBg].pixel;
	swa.event_mask = ExposureMask | KeyPressMask | VisibilityChangeMask;
	win = XCreateWindow(dpy, parentwin, x, y, mw, mh, 0,
	                    CopyFromParent, CopyFromParent, CopyFromParent,
	                    CWOverrideRedirect | CWBackPixel | CWEventMask, &swa);

	/* open input methods */
	xim = XOpenIM(dpy, NULL, NULL, NULL);
	xic = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
	                XNClientWindow, win, XNFocusWindow, win, NULL);

	XMapRaised(dpy, win);
	if (embed) {
		XSelectInput(dpy, parentwin, FocusChangeMask);
		if (XQueryTree(dpy, parentwin, &dw, &w, &dws, &du) && dws) {
			for (i = 0; i < du && dws[i] != win; ++i)
				XSelectInput(dpy, dws[i], FocusChangeMask);
			XFree(dws);
		}
		grabfocus();
	}
	drw_resize(drw, mw, mh);
	drawmenu();
}

static void usage(void) {
	fputs("usage: dmenu [-bfiv] [-l lines] [-p prompt] [-fn font] [-m monitor]\n"
	      "             [-x xoffset] [-y yoffset] [-wi width]\n"
	      "             [-nb color] [-nf color] [-sb color] [-sf color] [-w windowid]\n", stderr);
	exit(1);
}

int main(int argc, char *argv[]) {
	XWindowAttributes wa;
	int i, fast = 0;

	for (i = 1; i < argc; i++)
		/* these options take no arguments */
		if (!strcmp(argv[i], "-v")) {      /* prints version information */
			puts("dmenu-"VERSION);
			exit(0);
		} else if (!strcmp(argv[i], "-b")) /* appears at the bottom of the screen */
			topbar = 0;
		else if (!strcmp(argv[i], "-f"))   /* grabs keyboard before reading stdin */
			fast = 1;
		else if (!strcmp(argv[i], "-F"))   /* grabs keyboard before reading stdin */
			fuzzy = 0;
		else if (!strcmp(argv[i], "-i")) { /* case-insensitive item matching */
			fstrncmp = strncasecmp;
			fstrstr = cistrstr;
		} else if (i + 1 == argc)
			usage();
		/* these options take one argument */
		else if (!strcmp(argv[i], "-l"))   /* number of lines in vertical list */
			lines = atoi(argv[++i]);

    /* --------- xyw patch ----------------------------------------------------- */ 
		else if (!strcmp(argv[i], "-x"))   /* window x offset */
			dmx = atoi(argv[++i]);
		else if (!strcmp(argv[i], "-y"))   /* window y offset (from bottom up if -b) */
			dmy = atoi(argv[++i]);
		else if (!strcmp(argv[i], "-wi"))   /* make dmenu this wide */
			dmw = atoi(argv[++i]);
    /* -------------------------------------------------------------------------- */ 


		else if (!strcmp(argv[i], "-m"))
			mon = atoi(argv[++i]);
		else if (!strcmp(argv[i], "-p"))   /* adds prompt to left of input field */
			prompt = argv[++i];
		else if (!strcmp(argv[i], "-fn"))  /* font or font set */
			fonts[0] = argv[++i];
		else if (!strcmp(argv[i], "-nb"))  /* normal background color */
			colors[SchemeNorm][ColBg] = argv[++i];
		else if (!strcmp(argv[i], "-nf"))  /* normal foreground color */
			colors[SchemeNorm][ColFg] = argv[++i];
		else if (!strcmp(argv[i], "-sb"))  /* selected background color */
			colors[SchemeSel][ColBg] = argv[++i];
		else if (!strcmp(argv[i], "-sf"))  /* selected foreground color */
			colors[SchemeSel][ColFg] = argv[++i];
		else if (!strcmp(argv[i], "-w"))   /* embedding window id */
			embed = argv[++i];
		else
			usage();

	if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("warning: no locale support\n", stderr);
	if (!(dpy = XOpenDisplay(NULL)))
		die("cannot open display");
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	if (!embed || !(parentwin = strtol(embed, NULL, 0)))
		parentwin = root;
	if (!XGetWindowAttributes(dpy, parentwin, &wa))
		die("could not get embedding window attributes: 0x%lx",
		    parentwin);
	drw = drw_create(dpy, screen, root, wa.width, wa.height);
	if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
		die("no fonts could be loaded.");
	lrpad = drw->fonts->h;

	if (fast) {
		grabkeyboard();
		readstdin();
	} else {
		readstdin();
		grabkeyboard();
	}
	setup();
	run();
}
