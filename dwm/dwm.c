#include <assert.h>
#include <libgen.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/shape.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define ISVISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->seltags]) || C->issticky)
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw + gappx)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw + gappx)
#define TAGMASK                 ((1 << LENGTH(tags)) - 1)
#define ColBorder               2

enum { CurNormal, CurResize, CurMove, CurLast }; 
enum { SchemeNorm, SchemeSel, SchemeUrg }; 
enum { NetSupported, NetWMName, NetWMState, NetWMCheck,
       NetWMFullscreen, NetActiveWindow, NetWMWindowType, NetWMWindowTypeDock,
       NetWMWindowTypeDialog, NetClientList, NetLast }; /* EWMH atoms */
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast }; /* default atoms */
enum { ClkClientWin, ClkRootWin };

typedef union {
    int i;
    unsigned int ui;
    float f, sf;
    const void *v;
} Arg;

typedef struct {
    unsigned int click, mask, button;
    void (*func)(const Arg *arg);
    const Arg arg;
} Button;

typedef struct Monitor Monitor;
typedef struct Client Client;
struct Client {
    char name[64];
    float mina, maxa;
    int x, y, w, h;
    int sfx, sfy, sfw, sfh; /* stored float geometry, used on mode revert */
    int oldx, oldy, oldw, oldh;
    int basew, baseh, incw, inch, maxw, maxh, minw, minh;
    int bw, oldbw;
    int tags;
    int isfixed, iscentered, isfloating, isurgent, neverfocus, oldstate, isfullscreen, issticky, needresize;
    Client *next, *snext;
    Monitor *mon;
    Window win;
};

typedef struct {
    unsigned int mod;
    KeyCode keycode;
    void (*func)(const Arg *);
    const Arg arg;
} Key;

typedef struct {
    const char *symbol;
    void (*arrange)(Monitor *);
} Layout;

typedef struct Pertag Pertag;
struct Monitor {
    float mfact, smfact;
    int nmaster, num;
    int by;               /* bar geometry */
    int mx, my, mw, mh;   /* screen size */
    int wx, wy, ww, wh;   /* window area  */
    int seltags, sellt, tagset[2];
    Client *clients, *sel, *stack;
    Monitor *next;
    const Layout *lt[2];
    Pertag *pertag;
};

typedef struct {
    const char *class, *instance, *title;
    int tags, isfloating, iscentered, monitor;
} Rule;

static void arrange(Monitor *m);
static void arrangemon(Monitor *m);
static void buttonpress(XEvent *e);
static void checkotherwm(void);
static void cleanup(void);
static void cleanupmon(Monitor *mon);
static void clientmessage(XEvent *e);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static Monitor *createmon(void);
static void destroynotify(XEvent *e);
static void focus(Client *c);
static void focusin(XEvent *e);
static void focusstack(const Arg *arg);
static void grabbuttons(Client *c, int focused);
static void incnmaster(const Arg *arg);
static void keypress(XEvent *e);
static void killclient(const Arg *arg);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void motionnotify(XEvent *e);
static void movemouse(const Arg *arg);
static Client *nexttiled(Client *c);
static void propertynotify(XEvent *e);
static void quit(const Arg *arg);
static Monitor *recttomon(int x, int y, int w, int h);
static void resize(Client *c, int x, int y, int w, int h, int interact);
static void resizeclient(Client *c, int x, int y, int w, int h);
static void resizemouse(const Arg *arg);
static void restack(Monitor *m);
static int sendevent(Client *c, Atom proto);
static void sendmon(Client *c, Monitor *m);
static void setclientstate(Client *c, long state);
static void setfocus(Client *c);
static void setfullscreen(Client *c, int fullscreen);
static void setlayout(const Arg *arg);
static void setmfact(const Arg *arg);
static void setsmfact(const Arg *arg);
static void setup(void);
static void seturgent(Client *c, int urg);
static void showhide(Client *c);
static void sigchld(int unused);
static void spawn(const Arg *arg);
static void tag(const Arg *arg);
static void tile(Monitor *);
static void togglefloating(const Arg *arg);
static void togglescratch(const Arg *arg);
static void togglesticky(const Arg *arg);
static void togglegaps();
static void toggleborders();
static void toggleview(const Arg *arg);
static void unfocus(Client *c, int setfocus);
static void unmanage(Client *c, int destroyed);
static void unmapnotify(XEvent *e);
static int updategeom(void);
static void updatebarpos(Monitor *m);
static void updateclientlist(void);
static void updatenumlockmask(void);
static void updatewindowtype(Client *c);
static void updatetitle(Client *c);
static void updatewmhints(Client *c);
static void view(const Arg *arg);
static void warp(const Client *c);
static Client *wintoclient(Window w);
static Monitor *wintomon(Window w);
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);
/* static void drawroundedcorners(Client *c); */
static void update_ws_bools(Monitor *m); 
void tagall(const Arg *arg);

static int gappx = 0;
static int borderpx = 0;
static int screen, sw, sh; /* X display screen geometry width, height */
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent]) (XEvent *) = {
    [ButtonPress] = buttonpress,
    [ClientMessage] = clientmessage,
    [ConfigureRequest] = configurerequest,
    [ConfigureNotify] = configurenotify,
    [DestroyNotify] = destroynotify,
    [FocusIn] = focusin,
    [KeyPress] = keypress,
    [MappingNotify] = mappingnotify,
    [MapRequest] = maprequest,
    [MotionNotify] = motionnotify,
    [PropertyNotify] = propertynotify,
    [UnmapNotify] = unmapnotify
};
static Atom wmatom[WMLast], netatom[NetLast];
static int running = 1;
static Cur *cursor[CurLast];
static Scm *scheme;
static Display *dpy;
static Drw *drw;
static Monitor *mons, *selmon;
static Window root, wmcheckwin;
#include "config.h"
#include "dwm_info.c"

static unsigned int scratchtag = 1 << LENGTH(tags);

struct Pertag {
    int curtag, prevtag; /* current and previous tag */
    int nmasters[LENGTH(tags) + 1]; /* number of windows in master area */
    float mfacts[LENGTH(tags) + 1]; /* mfacts per tag */
    int sellts[LENGTH(tags) + 1]; /* selected layouts */
    const Layout *ltidxs[LENGTH(tags) + 1][2]; /* matrix of tags and layouts indexes  */
};

struct NumTags { char limitexceeded[LENGTH(tags) > 31 ? -1 : 1]; };

void applyrules(Client *c) {
    const char *class, *instance;
    const Rule *r;
    Monitor *m;
    XClassHint ch = { NULL, NULL };
    c->isfloating = 0;
    c->tags = 0;
    XGetClassHint(dpy, c->win, &ch);
    class    = ch.res_class ? ch.res_class : "broken";
    instance = ch.res_name  ? ch.res_name  : "broken";

    for (int i = 0; i < LENGTH(rules); i++) {
        r = &rules[i];
        if ((!r->title || strstr(c->name, r->title))
        && (!r->class || strstr(class, r->class))
        && (!r->instance || strstr(instance, r->instance))) {
            c->isfloating = r->isfloating;
            c->iscentered = r->iscentered;
            c->tags |= r->tags;
            for (m = mons; m && m->num != r->monitor; m = m->next);
            if (m) c->mon = m;
        }
    }
    if (ch.res_class) XFree(ch.res_class);
    if (ch.res_name) XFree(ch.res_name);
    c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags];
}

void arrange(Monitor *m) {
    if (m) showhide(m->stack);
    else for (m = mons; m; m = m->next) showhide(m->stack);
    if (m) {
        arrangemon(m);
        restack(m);
    } else for (m = mons; m; m = m->next)
        arrangemon(m);
}

void arrangemon(Monitor *m) {
    if (m->lt[m->sellt]->arrange)
        m->lt[m->sellt]->arrange(m);
}

void attach(Client *c) {
    c->next = c->mon->clients;
    c->mon->clients = c;
}

void attachabove(Client *c) {
    if(c->mon->sel == NULL || c->mon->sel == c->mon->clients || c->mon->sel->isfloating) {
        attach(c);
        return;
    }
    Client *at;
    for(at = c->mon->clients; at->next != c->mon->sel; at = at->next);
    c->next = at->next;
    at->next = c;
}

void attachstack(Client *c) {
    c->snext = c->mon->stack;
    c->mon->stack = c;
}

void buttonpress(XEvent *e) {
    unsigned int i, click;
    Arg arg = {0};
    Client *c;
    Monitor *m;
    XButtonPressedEvent *ev = &e->xbutton;

    click = ClkRootWin;
    /* focus monitor if necessary */
    if ((m = wintomon(ev->window)) && m != selmon) {
        unfocus(selmon->sel, 1);
        selmon = m;
        focus(NULL);
    }
    if((c = wintoclient(ev->window))) {
        focus(c);
        restack(selmon);
        XAllowEvents(dpy, ReplayPointer, CurrentTime);
        click = ClkClientWin;
    }
    for (i = 0; i < LENGTH(buttons); i++)
        if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
        && CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state)){
                buttons[i].func((buttons[i].arg.i == 0) ? &arg : &buttons[i].arg);
        }
}

void checkotherwm(void) {
    xerrorxlib = XSetErrorHandler(xerrorstart);
    /* this causes an error if some other window manager is running */
    XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
    XSync(dpy, False);
    XSetErrorHandler(xerror);
    XSync(dpy, False);
}

void cleanup(void) {
    Arg a = {.ui = ~0};
    Layout foo = { "", NULL };
    Monitor *m;
    size_t i;
    view(&a);
    selmon->lt[selmon->sellt] = &foo;
    for (m = mons; m; m = m->next)
        while (m->stack)
            unmanage(m->stack, 0);
    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    while (mons) cleanupmon(mons);
    for (i = 0; i < CurLast; i++) drw_cur_free(drw, cursor[i]);
    for (i = 0; i < LENGTH(colors); i++) free(scheme[i]);
    XDestroyWindow(dpy, wmcheckwin);
    drw_free(drw);
    XSync(dpy, False);
    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
}

void cleanupmon(Monitor *mon) {
    Monitor *m;
    if (mon == mons) mons = mons->next;
    else {
        for (m = mons; m && m->next != mon; m = m->next);
        m->next = mon->next;
    }
    free(mon);
}

void clientmessage(XEvent *e) {
    XClientMessageEvent *cme = &e->xclient;
    Client *c = wintoclient(cme->window);
    if (!c)
        return;

    if (cme->message_type == netatom[NetWMState]) {
        if (cme->data.l[1] == netatom[NetWMFullscreen]
        || cme->data.l[2] == netatom[NetWMFullscreen])
            setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
                || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && !c->isfullscreen)));
    } else if (cme->message_type == netatom[NetActiveWindow]) {
        if (c != selmon->sel && !c->isurgent)
            seturgent(c, 1);
    }
}

void configure(Client *c) {
    XConfigureEvent ce;
    ce.type = ConfigureNotify;
    ce.display = dpy;
    ce.event = c->win;
    ce.window = c->win;
    ce.x = c->x;
    ce.y = c->y;
    ce.width = c->w;
    ce.height = c->h;
    ce.border_width = c->bw;
    ce.above = None;
    ce.override_redirect = False;
    XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void configurenotify(XEvent *e) {
    Monitor *m;
    Client *c;
    XConfigureEvent *ev = &e->xconfigure;
    int dirty;
    if (ev->window == root) {
        dirty = (sw != ev->width || sh != ev->height);
        sw = ev->width;
        sh = ev->height;
        if (updategeom() || dirty) {
            for (m = mons; m; m = m->next) {
                for (c = m->clients; c; c = c->next)
                    if (c->isfullscreen)
                        resizeclient(c, m->mx, m->my, m->mw, m->mh);
            }
            focus(NULL);
            arrange(NULL);
        }
    }
}

void configurerequest(XEvent *e) {
    Client *c;
    Monitor *m;
    XConfigureRequestEvent *ev = &e->xconfigurerequest;
    XWindowChanges wc;

    if (c = wintoclient(ev->window)) {
        if (ev->value_mask & CWBorderWidth) c->bw = ev->border_width;
        else if (c->isfloating || !selmon->lt[selmon->sellt]->arrange) {
            m = c->mon;
            if (ev->value_mask & CWX) {
                c->oldx = c->x;
                c->x = m->mx + ev->x;
            }
            if (ev->value_mask & CWY) {
                c->oldy = c->y;
                c->y = m->my + ev->y;
            }
            if (ev->value_mask & CWWidth) {
                c->oldw = c->w;
                c->w = ev->width;
            }
            if (ev->value_mask & CWHeight) {
                c->oldh = c->h;
                c->h = ev->height;
            }
            if ((c->x + c->w) > m->mx + m->mw && c->isfloating)
                c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */
            if ((c->y + c->h) > m->my + m->mh && c->isfloating)
                c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
            if ((ev->value_mask & (CWX|CWY)) && !(ev->value_mask & (CWWidth|CWHeight)))
                configure(c);
            if (ISVISIBLE(c)) 
                XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
            else 
                c->needresize = 1;
        } else configure(c);
    } else {
        wc.x = ev->x;
        wc.y = ev->y;
        wc.width = ev->width;
        wc.height = ev->height;
        wc.border_width = ev->border_width;
        wc.sibling = ev->above;
        wc.stack_mode = ev->detail;
        XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
    }
    XSync(dpy, False);
}

Monitor * createmon(void) {
    Monitor *m;

    m = ecalloc(1, sizeof(Monitor));
    m->tagset[0] = m->tagset[1] = 1;
    m->mfact = 0.50;
    m->smfact = 0.00;
    m->nmaster = 1;
    m->lt[0] = &layouts[0];
    m->lt[1] = &layouts[1 % LENGTH(layouts)];
    m->pertag = ecalloc(1, sizeof(Pertag));
    m->pertag->curtag = m->pertag->prevtag = 1;

    for (int i = 0; i <= LENGTH(tags); i++) {
        m->pertag->nmasters[i] = m->nmaster;
        m->pertag->mfacts[i] = m->mfact;
        m->pertag->ltidxs[i][0] = m->lt[0];
        m->pertag->ltidxs[i][1] = m->lt[1];
        m->pertag->sellts[i] = m->sellt;
    }
    return m;
}

void destroynotify(XEvent *e) {
    Client *c;
    XDestroyWindowEvent *ev = &e->xdestroywindow;

    if (c = wintoclient(ev->window))
 		unmanage(c, 1);
}

void detach(Client *c) {
    Client **tc;
    for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next);
    *tc = c->next;
}

void detachstack(Client *c) {
    Client **tc, *t;
    for (tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext);
    *tc = c->snext;
    if (c == c->mon->sel) {
        for (t = c->mon->stack; t && !ISVISIBLE(t); t = t->snext);
        c->mon->sel = t;
    }
}

void focus(Client *c) {
/* ------------------------------------------------------- */ 
	XWindowChanges wc;
/* ------------------------------------------------------- */ 
    if (!c || !ISVISIBLE(c)) for (c = selmon->stack; c && !ISVISIBLE(c); c = c->snext);
    if (selmon->sel && selmon->sel != c) unfocus(selmon->sel, 0);
    if (c) {
        if (c->mon != selmon) selmon = c->mon;
        if (c->isurgent) seturgent(c, 0);
        detachstack(c);
        attachstack(c);
        grabbuttons(c, 1);
        XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
        /* ------------------------------------------------------- */ 
		if(!c->isfloating) {
			wc.stack_mode = Below;
			XConfigureWindow(dpy, c->win, CWSibling | CWStackMode, &wc);
		}
        /* ------------------------------------------------------- */ 
        setfocus(c);
    } else {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }
    selmon->sel = c;
}

void focusin(XEvent *e) {
    XFocusChangeEvent *ev = &e->xfocus;
    if (selmon->sel && ev->window != selmon->sel->win) setfocus(selmon->sel);
}

void focusstack(const Arg *arg) {
    Client *c = NULL, *i;
    if (!selmon->sel) return;
    if (arg->i > 0) {
        for (c = selmon->sel->next; c && !ISVISIBLE(c); c = c->next);
        if (!c) for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next);
    } else {
        for (i = selmon->clients; i != selmon->sel; i = i->next)
            if (ISVISIBLE(i))
                c = i;
        if (!c)
            for (; i; i = i->next)
                if (ISVISIBLE(i))
                    c = i;
    }
    if (c) {
        focus(c);
        restack(selmon);
    }
}

Atom getatomprop(Client *c, Atom prop) {
    int di;
    unsigned long dl;
    unsigned char *p = NULL;
    Atom da, atom = None;
    if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, XA_ATOM,
        &da, &di, &dl, &dl, &p) == Success && p) {
        atom = *(Atom *)p;
        XFree(p);
    }
    return atom;
}

int getrootptr(int *x, int *y) {
    int di;
    unsigned int dui;
    Window dummy;
    return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

void toggleborders() {
    if(borderpx > 0)
        borderpx = 0;
    else
        borderpx = BORDER_PX;
}

long getstate(Window w) {
    int format;
    long result = -1;
    unsigned char *p = NULL;
    unsigned long n, extra;
    Atom real;
    if (XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState],
        &real, &format, &n, &extra, (unsigned char **)&p) != Success)
        return -1;
    if (n != 0) result = *p;
    XFree(p);
    return result;
}

int gettextprop(Window w, Atom atom, char *text, unsigned int size) {
    char **list = NULL;
    int n;
    XTextProperty name;

    if (!text || size == 0) return 0;
    text[0] = '\0';
    XGetTextProperty(dpy, w, &name, atom);
    if (!name.nitems) return 0;
    if (name.encoding == XA_STRING)
        strncpy(text, (char *)name.value, size - 1);
    else {
        if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
            strncpy(text, *list, size - 1);
            XFreeStringList(list);
        }
    }
    text[size - 1] = '\0';
    XFree(name.value);
    return 1;
}

void grabbuttons(Client *c, int focused) {
    updatenumlockmask(); {
        unsigned int i, j;
        unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        if (!focused)
            XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
                BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
        for (i = 0; i < LENGTH(buttons); i++)
            if (buttons[i].click == ClkClientWin)
                for (j = 0; j < LENGTH(modifiers); j++)
                    XGrabButton(dpy, buttons[i].button,
                        buttons[i].mask | modifiers[j],
                        c->win, False, BUTTONMASK,
                        GrabModeAsync, GrabModeSync, None, None);
    }
}

void grabkeys(void) {
        unsigned int i, j;
        unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
        XUngrabKey(dpy, AnyKey, AnyModifier, root);
        for (i = 0; i < LENGTH(keys); i++)
            for (j = 0; j < LENGTH(modifiers); j++)
                XGrabKey(dpy, keys[i].keycode, keys[i].mod | modifiers[j], root,
                     True, GrabModeAsync, GrabModeAsync);
}

void incnmaster(const Arg *arg) {
    selmon->nmaster = MAX(selmon->nmaster + arg->i, 0);
    arrange(selmon);
}

#ifdef XINERAMA
static int isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info) {
    while (n--)
        if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org
        && unique[n].width == info->width && unique[n].height == info->height)
            return 0;
    return 1;
}
#endif /* XINERAMA */

void keypress(XEvent *e) {
    unsigned int i;
    XKeyEvent *ev;
 
    ev = &e->xkey;
    for (i = 0; i < LENGTH(keys); i++)
        if (ev->keycode == keys[i].keycode
        && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
        && keys[i].func)
            keys[i].func(&(keys[i].arg));
}

void killclient(const Arg *arg) {
    if (!selmon->sel) return;
    if (!sendevent(selmon->sel, wmatom[WMDelete])) {
        XGrabServer(dpy);
        XSetErrorHandler(xerrordummy);
        XSetCloseDownMode(dpy, DestroyAll);
        XKillClient(dpy, selmon->sel->win);
        XSync(dpy, False);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
    }
}

void manage(Window w, XWindowAttributes *wa) {
    Client *c, *t = NULL;
    Window trans = None;
    XWindowChanges wc;
    c = ecalloc(1, sizeof(Client));
    c->win = w;
    /* geometry */
    c->sfx = c->x = c->oldx = wa->x;
    c->sfy = c->y = c->oldy = wa->y;
    c->sfw = c->w = c->oldw = wa->width;
    c->sfh = c->h = c->oldh = wa->height;
    c->oldbw = wa->border_width;
    updatetitle(c);
    if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
        c->mon = t->mon;
        c->tags = t->tags;
    } else {
        c->mon = selmon;
        applyrules(c);
    }
    if (c->x + WIDTH(c) > c->mon->mx + c->mon->mw) c->x = c->mon->mx + c->mon->mw - WIDTH(c);
    if (c->y + HEIGHT(c) > c->mon->my + c->mon->mh) c->y = c->mon->my + c->mon->mh - HEIGHT(c);
    c->x = MAX(c->x, c->mon->mx);
    /* only fix client y-offset, if the client center might cover the bar */
    c->y = MAX(c->y, ((c->mon->by == c->mon->my) && (c->x + (c->w / 2) >= c->mon->wx)
        && (c->x + (c->w / 2) < c->mon->wx + c->mon->ww)) ? BAR_HEIGHT : c->mon->my);
    c->bw = borderpx;

    if(c->iscentered) {
        c->x = (c->mon->mw - WIDTH(c)) / 2;
        c->y = (c->mon->mh - HEIGHT(c)) / 2;
        /* if(c->isfloating) drawroundedcorners(c); */
    }

	if (!strcmp(c->name, scratchpadname)) {
		c->mon->tagset[c->mon->seltags] |= c->tags = scratchtag;
		c->isfloating = True;
		c->x = c->mon->wx + (c->mon->ww / 2 - WIDTH(c) / 2);
		c->y = c->mon->wy + (c->mon->wh / 2 - HEIGHT(c) / 2);
	}

    wc.border_width = c->bw;
    XConfigureWindow(dpy, w, CWBorderWidth, &wc);
    XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColBorder].pixel);
    configure(c); /* propagates border_width, if size doesn't change */
    updatewindowtype(c);
    updatewmhints(c);

    c->sfx = c->x; // restore old float dimensions
    c->sfy = c->y;
    c->sfw = c->w;
    c->sfh = c->h;

    XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
    grabbuttons(c, 0);
    if (!c->isfloating) c->isfloating = c->oldstate = trans != None || c->isfixed;
    if (c->isfloating) XRaiseWindow(dpy, c->win);
    attachabove(c);
    attachstack(c);
    XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
        (unsigned char *) &(c->win), 1);
    XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h); /* some windows require this */
    setclientstate(c, NormalState);
    if (c->mon == selmon) unfocus(selmon->sel, 0);
    c->mon->sel = c;
    arrange(c->mon);
    XMapWindow(dpy, c->win);
    focus(NULL);
}
 void togglescratch(const Arg *arg) {
	Client *c;
	unsigned int found = 0;

	for (c = selmon->clients; c && !(found = c->tags & scratchtag); c = c->next);
	if (found) {
		unsigned int newtagset = selmon->tagset[selmon->seltags] ^ scratchtag;
		if (newtagset) {
			selmon->tagset[selmon->seltags] = newtagset;
			focus(NULL);
			arrange(selmon);
		}
		if (ISVISIBLE(c)) {
			focus(c);
			restack(selmon);
		}
	} else
		spawn(arg);
}
void tagall(const Arg *arg) {
	if (!selmon->clients)
		return;
	/* if parameter starts with F, just move floating windows */
	int floating_only = (char *)arg->v && ((char *)arg->v)[0] == 'F' ? 1 : 0;
	int tag = (char *)arg->v ? atoi(((char *)arg->v) + floating_only) : 0;
	int j;
	Client* c;
	if(tag >= 0 && tag < LENGTH(tags))
		for(c = selmon->clients; c; c = c->next) {
			if(!floating_only || c->isfloating)
				for(j = 0; j < LENGTH(tags); j++) {
					if(c->tags & 1 << j && selmon->tagset[selmon->seltags] & 1 << j) {
						c->tags = c->tags ^ (1 << j & TAGMASK);
						c->tags = c->tags | 1 << (tag-1);
					}
				}
		}
	arrange(selmon);
}

void mappingnotify(XEvent *e) {
    XMappingEvent *ev = &e->xmapping;
    XRefreshKeyboardMapping(ev);
    if (ev->request == MappingKeyboard) grabkeys();
}

void maprequest(XEvent *e) {
    static XWindowAttributes wa;
    XMapRequestEvent *ev = &e->xmaprequest;
    if (!XGetWindowAttributes(dpy, ev->window, &wa)) return;
    if (wa.override_redirect) return;
    if (!wintoclient(ev->window))
        manage(ev->window, &wa);
}

void motionnotify(XEvent *e) {
    static Monitor *mon = NULL;
    Monitor *m;
    XMotionEvent *ev = &e->xmotion;

    if (ev->window != root) return;
    if ((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
        unfocus(selmon->sel, 1);
        selmon = m;
        focus(NULL);
    }
    mon = m;
}

void movemouse(const Arg *arg) {
    int x, y, ocx, ocy, nx, ny;
    Client *c;
    Monitor *m;
    XEvent ev;
    Time lasttime = 0;
    if (!(c = selmon->sel) || c->isfullscreen) return;
    restack(selmon);
    ocx = c->x;
    ocy = c->y;
    if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
        None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
        return;
    if (!getrootptr(&x, &y)) return;
    do {
        XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
        switch(ev.type) {
        case ConfigureRequest:
        case MapRequest:
            handler[ev.type](&ev);
            break;
        case MotionNotify:
            if ((ev.xmotion.time - lasttime) <= (1000 / 60)) continue;
            lasttime = ev.xmotion.time;

            nx = ocx + (ev.xmotion.x - x);
            ny = ocy + (ev.xmotion.y - y);
            if (abs(selmon->wx - nx) < snap)
                nx = selmon->wx;
            else if (abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < snap)
                nx = selmon->wx + selmon->ww - WIDTH(c);
            if (abs(selmon->wy - ny) < snap)
                ny = selmon->wy;
            else if (abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < snap)
                ny = selmon->wy + selmon->wh - HEIGHT(c);
            if (!c->isfloating && selmon->lt[selmon->sellt]->arrange && (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
                togglefloating(NULL);
            if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
                resize(c, nx, ny, c->w, c->h, 1);
            break;
        }
    } while (ev.type != ButtonRelease);
    XUngrabPointer(dpy, CurrentTime);
    if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
        sendmon(c, m);
        selmon = m;
        focus(NULL);
    }
}

Client * nexttiled(Client *c) {
    for (; c && (c->isfloating || !ISVISIBLE(c)); c = c->next);
    return c;
}

void propertynotify(XEvent *e) {
    Client *c;
    Window trans;
    XPropertyEvent *ev = &e->xproperty;

    if (ev->state == PropertyDelete) return; /* ignore */
    else if (c = wintoclient(ev->window)) {
        switch(ev->atom) {
        default: break;
        case XA_WM_TRANSIENT_FOR:
            if (!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) &&
                (c->isfloating = (wintoclient(trans)) != NULL))
                arrange(c->mon);
            break;
        case XA_WM_HINTS:
            updatewmhints(c);
            break;
        }
        if(ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) updatetitle(c);
        if (ev->atom == netatom[NetWMWindowType]) updatewindowtype(c);
    }
}

void quit(const Arg *arg) { running = 0; }

Monitor * recttomon(int x, int y, int w, int h) {
    Monitor *m, *r = selmon;
    int a, area = 0;

    for (m = mons; m; m = m->next)
        if ((a = INTERSECT(x, y, w, h, m)) > area) {
            area = a;
            r = m;
        }
    return r;
}

void resize(Client *c, int x, int y, int w, int h, int interact) {
    resizeclient(c, x, y, w, h);
}

void resizeclient(Client *c, int x, int y, int w, int h) {
    XWindowChanges wc;
    int n; // number of clients on tag
    int gapoffset; // the distance away from the 0,0
    int gapincr;
    Client *nbc;

    wc.border_width = c->bw;

    // find number of clients
    for (n = 0, nbc = nexttiled(selmon->clients); nbc; nbc = nexttiled(nbc->next), n++);

    if (c->isfloating || selmon->lt[selmon->sellt]->arrange == NULL) {
        gapincr = gapoffset = 0;
        if(n <= 1) wc.border_width = 0;
    } else {
        // Uncomment these if you want terminals to keep their border,
        // regardless of whether they are the only window on the tag or not
        if (n == 1) {
            /* if(terminals_keep_border == 1) { */
                /* if(strcmp(c->name, terminal) != 0 && strcmp(c->name, "ranger") != 0 && strcmp(c->name, "nvim") != 0) { */
                    /* then set no border with appropriate gaps */
                    /* wc.border_width = 0; */
                    /* gapincr = 2 * gappx - 2*borderpx; */
                /* } */
            /* } else { */
                /* then set no border with appropriate gaps */
                wc.border_width = 0;
                gapincr = 2 * gappx - 2*borderpx;
            /* } */
        } else gapincr = 2 * gappx;
        gapoffset = gappx;
    }

    c->oldx = c->x; c->oldy = c->y;
    c->oldw = c->w; c->oldh = c->h;

    c->x = wc.x = x + gapoffset;
    c->y = wc.y = y + gapoffset;
    c->w = wc.width = w - gapincr;
    c->h = wc.height = h - gapincr;

    // if it is floating, then it is handled in resizemouse
    /* if(!c->isfloating && round_non_floating == 1) drawroundedcorners(c); */

    XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
    configure(c);
    XSync(dpy, False);
}

/* void drawroundedcorners(Client *c) { */
/*     // NOTE: this is extremely hacky and surely could be optimized. */
/*     //       Any X wizards out there reading this, please pull request. */
/*     if(CORNER_RADIUS > 0 && c && !c->isfullscreen){ */
/*         Window win; */
/*         win = c->win; */
/*         if(!win) return; */

/*         XWindowAttributes win_attr; */
/*         if(!XGetWindowAttributes(dpy, win, &win_attr)) return; */

/*         const int w = c->w; */
/*         const int h = c->h; */

/*         const int dia = 2 * CORNER_RADIUS; // set in config.h */
/*         if(w < dia || h < dia) return; */

/*         Pixmap mask; */
/*         mask = XCreatePixmap(dpy, win, w, h, 1); */
/*         if(!mask) return; */

/*         XGCValues xgcv; */
/*         GC shape_gc; */
/*         shape_gc = XCreateGC(dpy, mask, 0, &xgcv); */

/*         if(!shape_gc) { */
/*             XFreePixmap(dpy, mask); */
/*             free(shape_gc); */
/*             return; */
/*         } */

/*         XSetForeground(dpy, shape_gc, 0); */
/*         XFillRectangle(dpy, mask, shape_gc, 0, 0, w, h); */
/*         XSetForeground(dpy, shape_gc, 1); */
/*         XFillArc(dpy, mask, shape_gc, 0, 0, dia, dia, 0, 23040); */
/*         XFillArc(dpy, mask, shape_gc, w-dia-1, 0, dia, dia, 0, 23040); */
/*         XFillArc(dpy, mask, shape_gc, 0, h-dia-1, dia, dia, 0, 23040); */
/*         XFillArc(dpy, mask, shape_gc, w-dia-1, h-dia-1, dia, dia, 0, 23040); */
/*         XFillRectangle(dpy, mask, shape_gc, CORNER_RADIUS, 0, w-dia, h); */
/*         XFillRectangle(dpy, mask, shape_gc, 0, CORNER_RADIUS, w, h-dia); */
/*         XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, mask, ShapeSet); */
/*         XFreePixmap(dpy, mask); */
/*         XFreeGC(dpy, shape_gc); */
/*     } */
/* } */

void resizemouse(const Arg *arg) {
    int ocx, ocy, nw, nh;
    int ocx2, ocy2, nx, ny;
    Client *c;
    Monitor *m;
    XEvent ev;
    int horizcorner, vertcorner;
    int di;
    unsigned int dui;
    Window dummy;
    Time lasttime = 0;

    if (!(c = selmon->sel)) return;
    if (c->isfullscreen) return;
    restack(selmon);
    ocx = c->x;
    ocy = c->y;

    ocx2 = c->x + c->w;
    ocy2 = c->y + c->h;
    if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
        None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
        return;
    if (!XQueryPointer (dpy, c->win, &dummy, &dummy, &di, &di, &nx, &ny, &dui)) return;
    horizcorner = nx < c->w / 2;
    vertcorner = ny < c->h / 2;

    XWarpPointer (dpy, None, c->win, 0, 0, 0, 0, \
        horizcorner ? (-c->bw) : (c->w + c->bw - 1), \
        vertcorner ? (-c->bw) : (c->h + c->bw - 1));
    do {
        XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
        switch(ev.type) {
            case ConfigureRequest:
            case MapRequest: handler[ev.type](&ev); break;
            case MotionNotify:
                if ((ev.xmotion.time - lasttime) <= (1000 / 60)) continue;
                lasttime = ev.xmotion.time;

                nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
                nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
                nx = horizcorner ? ev.xmotion.x : c->x;
                ny = vertcorner ? ev.xmotion.y : c->y;
                nw = MAX(horizcorner ? (ocx2 - nx) : (ev.xmotion.x - ocx - 2 * c->bw + 1), 1);
                nh = MAX(vertcorner ? (ocy2 - ny) : (ev.xmotion.y - ocy - 2 * c->bw + 1), 1);
                if (c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
                && c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh) {
                    if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
                    && (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
                        togglefloating(NULL);
                }


                if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
                    resize(c, nx, ny, nw, nh, 1);

                /* drawroundedcorners(c); */
                break;
        }
    } while (ev.type != ButtonRelease);
    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0,
              horizcorner ? (-c->bw) : (c->w + c->bw - 1),
              vertcorner ? (-c->bw) : (c->h + c->bw - 1));
    XUngrabPointer(dpy, CurrentTime);
    while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
    if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
        sendmon(c, m);
        selmon = m;
        focus(NULL);
    }
    /* drawroundedcorners(c); */
}

void restack(Monitor *m) {
    Client *c;
    XEvent ev;
    XWindowChanges wc;
    if (!m->sel) return;
    if (m->sel->isfloating || !m->lt[m->sellt]->arrange)
        XRaiseWindow(dpy, m->sel->win);
    if (m->lt[m->sellt]->arrange) {
        wc.stack_mode = Below;
        for (c = m->stack; c; c = c->snext)
            if (!c->isfloating && ISVISIBLE(c)) {
                XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
                wc.sibling = c->win;
            }
    }
    XSync(dpy, False);
    update_ws_bools(m);
    while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
    if( warp_mouse == 1)
        if (m == selmon && (m->tagset[m->seltags] & m->sel->tags) && selmon->lt[selmon->sellt] != &layouts[2])
            warp(m->sel);
}

void warp(const Client *c) {
	int x, y;

	if (!c) {
		XWarpPointer(dpy, None, root, 0, 0, 0, 0, selmon->wx + selmon->ww/2, selmon->wy + selmon->wh/2);
		return;
	}

	if (!getrootptr(&x, &y) ||
	    (x > c->x - c->bw &&
	     y > c->y - c->bw &&
	     x < c->x + c->w + c->bw*2 &&
	     y < c->y + c->h + c->bw*2) ||
	    (y > c->mon->by && y < c->mon->by + BAR_HEIGHT) ||
	    (topbar && !y))
		return;

	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w / 2, c->h / 2);
}

void run(void) {
    XEvent ev;
    /* main event loop */
    XSync(dpy, False);
    while (running && !XNextEvent(dpy, &ev))
        if (handler[ev.type])
            handler[ev.type](&ev); /* call handler */
}

void scan(void) {
    unsigned int i, num;
    Window d1, d2, *wins = NULL;
    XWindowAttributes wa;
    if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
        for (i = 0; i < num; i++) {
            if (!XGetWindowAttributes(dpy, wins[i], &wa)
            || wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1)) continue;
            if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState) manage(wins[i], &wa);
        }
        for (i = 0; i < num; i++) { /* now the transients */
            if (!XGetWindowAttributes(dpy, wins[i], &wa)) continue;
            if (XGetTransientForHint(dpy, wins[i], &d1)
            && (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
                manage(wins[i], &wa);
        }
        if (wins) XFree(wins);
    }
}

void sendmon(Client *c, Monitor *m) {
    if (c->mon == m) return;
    unfocus(c, 1);
    detach(c);
    detachstack(c);
    c->mon = m;
    c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
    attachabove(c);
    attachstack(c);
    focus(NULL);
    arrange(NULL);
}

void setclientstate(Client *c, long state) {
    long data[] = { state, None };
    XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32, PropModeReplace, (unsigned char *)data, 2);
}

int sendevent(Client *c, Atom proto) {
    int n;
    Atom *protocols;
    int exists = 0;
    XEvent ev;

    if (XGetWMProtocols(dpy, c->win, &protocols, &n)) {
        while (!exists && n--) exists = protocols[n] == proto;
        XFree(protocols);
    }
    if (exists) {
        ev.type = ClientMessage;
        ev.xclient.window = c->win;
        ev.xclient.message_type = wmatom[WMProtocols];
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = proto;
        ev.xclient.data.l[1] = CurrentTime;
        XSendEvent(dpy, c->win, False, NoEventMask, &ev);
    }
    return exists;
}

void setfocus(Client *c) {
    if (!c->neverfocus) {
        XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
        XChangeProperty(dpy, root, netatom[NetActiveWindow],
            XA_WINDOW, 32, PropModeReplace,
            (unsigned char *) &(c->win), 1);
    }
    sendevent(c, wmatom[WMTakeFocus]);
}

void setfullscreen(Client *c, int fullscreen) {
    if (fullscreen && !c->isfullscreen) {
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
            PropModeReplace, (unsigned char*)&netatom[NetWMFullscreen], 1);
        c->isfullscreen = 1;
        c->oldstate = c->isfloating;
        c->oldbw = c->bw;
        c->bw = 0;
        c->isfloating = 1;
        resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
        XRaiseWindow(dpy, c->win);
    } else if (!fullscreen && c->isfullscreen){
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
            PropModeReplace, (unsigned char*)0, 0);
        c->isfullscreen = 0;
        c->isfloating = c->oldstate;
        c->bw = c->oldbw;
        c->x = c->oldx;
        c->y = c->oldy;
        c->w = c->oldw;
        c->h = c->oldh;
        resizeclient(c, c->x, c->y, c->w, c->h);
        arrange(c->mon);
    }
}

void setlayout(const Arg *arg) {
    for(int i = 0 ; i < NUM_LAYOUTS ; i++ )
        if (&layouts[i] == arg->v) {
            set_dwm_info_current_layout(i);
            break;
        }

    if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
        selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag] ^= 1;
    if (arg && arg->v)
        selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt] = (Layout *)arg->v;
    if (selmon->sel) arrange(selmon);
}

/* arg > 1.0 will set mfact absolutely */
void setmfact(const Arg *arg) {
    float f;

    if (!arg || !selmon->lt[selmon->sellt]->arrange) return;
    f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
    if (f < 0.1 || f > 0.9) return;
    selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag] = f;
    arrange(selmon);
}

void setsmfact(const Arg *arg){
   float sf;
   if(!arg || !selmon->lt[selmon->sellt]->arrange) return;
   sf = arg->sf < 1.0 ? arg->sf + selmon->smfact : arg->sf - 1.0;
   if(sf < 0 || sf > 0.9) return;
   selmon->smfact = sf;
   arrange(selmon);
}

void setup(void) {
    int i;
    XSetWindowAttributes wa;
    /* clean up any zombies immediately */
    sigchld(0);
    /* init screen */
    screen = DefaultScreen(dpy);
    sw = DisplayWidth(dpy, screen);
    sh = DisplayHeight(dpy, screen);
    root = RootWindow(dpy, screen);
    drw = drw_create(dpy, screen, root, sw, sh);
    updategeom();
    /* init atoms */
    wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
    wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
    wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
    netatom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
    netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
    netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
    netatom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
    netatom[NetWMWindowTypeDock] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
    netatom[NetWMCheck] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
    netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    netatom[NetWMWindowTypeDialog] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
    /* init cursors */
    cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
    cursor[CurResize] = drw_cur_create(drw, XC_sizing);
    cursor[CurMove] = drw_cur_create(drw, XC_fleur);
    /* init appearance */
    scheme = ecalloc(LENGTH(colors), sizeof(Scm));
    for (i = 0; i < LENGTH(colors); i++)
        scheme[i] = drw_scm_create(drw, colors[i], 3);
    /* supporting window for NetWMCheck */
    wmcheckwin = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
    XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &wmcheckwin, 1);
    XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &wmcheckwin, 1);
    XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], XInternAtom(dpy, "UTF8_STRING", False), 8, PropModeReplace, (unsigned char *) "dwm", 3);
    /* EWMH support per view */
    XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32, PropModeReplace, (unsigned char *) netatom, NetLast);
    XDeleteProperty(dpy, root, netatom[NetClientList]);
    /* select events */
    wa.cursor = cursor[CurNormal]->cursor;
    wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask
        |ButtonPressMask|PointerMotionMask|EnterWindowMask
        |LeaveWindowMask|StructureNotifyMask|PropertyChangeMask;
    XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
    XSelectInput(dpy, root, wa.event_mask);
    grabkeys();
    focus(NULL);
}

void seturgent(Client *c, int urg) {
    XWMHints *wmh;
    c->isurgent = urg;
    if (!(wmh = XGetWMHints(dpy, c->win))) return;
    wmh->flags = urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
    XSetWMHints(dpy, c->win, wmh);
    XFree(wmh);
}

void showhide(Client *c) {
    if (!c) return;
    if (ISVISIBLE(c)) {
        /* show clients top down */
        XMoveWindow(dpy, c->win, c->x, c->y);
        if (c->needresize) {
            c->needresize = 0;
            XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
        } else 
            XMoveWindow(dpy, c->win, c->x, c->y);
        if ((!c->mon->lt[c->mon->sellt]->arrange || c->isfloating) && !c->isfullscreen)
            resize(c, c->x, c->y, c->w, c->h, 0);
        showhide(c->snext);
    } else {
        /* hide clients bottom up */
        showhide(c->snext);
        XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
    }
}

void sigchld(int unused) {
    if (signal(SIGCHLD, sigchld) == SIG_ERR)
        die("can't install SIGCHLD handler:");
    while (0 < waitpid(-1, NULL, WNOHANG));
}

#define SPAWN_CWD_DELIM " []{}()<>\"':"
void spawn(const Arg *arg) {
    if (arg->v == dmenucmd) dmenumon[0] = '0' + selmon->num;
    if (fork() == 0) {
        if (dpy) close(ConnectionNumber(dpy));

        /* --------- spawn cwd -------------------------------------- */ 
		/* if(selmon->sel) { */
		/* 	const char* const home = getenv("HOME"); */
		/* 	assert(home && strchr(home, '/')); */
		/* 	const size_t homelen = strlen(home); */
		/* 	char *cwd, *pathbuf = NULL; */
		/* 	struct stat statbuf; */

		/* 	cwd = strtok(selmon->sel->name, SPAWN_CWD_DELIM); */
		/* 	/1* NOTE: strtok() alters selmon->sel->name in-place, */
		/* 	 * but that does not matter because we are going to */
		/* 	 * exec() below anyway; nothing else will use it *1/ */
		/* 	while(cwd) { */
		/* 		if(*cwd == '~') { /1* replace ~ with $HOME *1/ */
		/* 			if(!(pathbuf = malloc(homelen + strlen(cwd)))) /1* ~ counts for NULL term *1/ */
		/* 				die("fatal: could not malloc() %u bytes\n", homelen + strlen(cwd)); */
		/* 			strcpy(strcpy(pathbuf, home) + homelen, cwd + 1); */
		/* 			cwd = pathbuf; */
		/* 		} */

		/* 		if(strchr(cwd, '/') && !stat(cwd, &statbuf)) { */
		/* 			if(!S_ISDIR(statbuf.st_mode)) */
		/* 				cwd = dirname(cwd); */

		/* 			if(!chdir(cwd)) */
		/* 				break; */
		/* 		} */

		/* 		cwd = strtok(NULL, SPAWN_CWD_DELIM); */
		/* 	} */

		/* 	free(pathbuf); */
		/* } */
        /* ------------------------------------------------------- */ 

        setsid();
        execvp(((char **)arg->v)[0], (char **)arg->v);
        fprintf(stderr, "dwm: execvp %s", ((char **)arg->v)[0]);
        perror(" failed");
        exit(EXIT_SUCCESS);
    }
}

void tag(const Arg *arg) {
    if (selmon->sel && arg->ui & TAGMASK) {
        selmon->sel->tags = arg->ui & TAGMASK;
        focus(NULL);
        arrange(selmon);
    }
}

void togglefloating(const Arg *arg) {
    if (!selmon->sel) return;
    if (selmon->sel->isfullscreen) /* no support for fullscreen windows */ return;
    selmon->sel->isfloating = !selmon->sel->isfloating || selmon->sel->isfixed;

    if(selmon->sel->isfloating) { 
        /*restore last known float dimensions*/
        resize(selmon->sel, selmon->sel->sfx, selmon->sel->sfy,
               selmon->sel->sfw, selmon->sel->sfh, 0);
    } else {
        /*save last known float dimensions*/
        selmon->sel->sfx = selmon->sel->x;
        selmon->sel->sfy = selmon->sel->y;
        selmon->sel->sfw = selmon->sel->w;
        selmon->sel->sfh = selmon->sel->h;
    }
    arrange(selmon);
}

void togglesticky(const Arg *arg) {
    if (!selmon->sel) return;
    selmon->sel->issticky = !selmon->sel->issticky;
    arrange(selmon);
}

void toggleview(const Arg *arg) {
    unsigned int newtagset = selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);
    int i;

    if (newtagset) {
        selmon->tagset[selmon->seltags] = newtagset;

        if (newtagset == ~0) {
            selmon->pertag->prevtag = selmon->pertag->curtag;
            selmon->pertag->curtag = 0;
        }

        /* test if the user did not select the same tag */
        if (!(newtagset & 1 << (selmon->pertag->curtag - 1))) {
            selmon->pertag->prevtag = selmon->pertag->curtag;
            for (i = 0; !(newtagset & 1 << i); i++) ;
            selmon->pertag->curtag = i + 1;
        }

        /* apply settings for this view */
        selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
        selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
        selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
        selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
        selmon->lt[selmon->sellt^1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt^1];

        focus(NULL);
        arrange(selmon);
    }
}

void unfocus(Client *c, int setfocus) {
    if (!c) return;
    grabbuttons(c, 0);
    XSetWindowBorder(dpy, c->win, scheme[SchemeNorm][ColBorder].pixel);
    if (setfocus) {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }
}

void unmanage(Client *c, int destroyed) {
    Monitor *m = c->mon;
    XWindowChanges wc;
    detach(c);
    detachstack(c);
    if (!destroyed) {
        wc.border_width = c->oldbw;
        XGrabServer(dpy); /* avoid race conditions */
        XSetErrorHandler(xerrordummy);
        XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        setclientstate(c, WithdrawnState);
        XSync(dpy, False);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
    }
    free(c);
    focus(NULL);
    updateclientlist();
    arrange(m);
}

void unmapnotify(XEvent *e) {
    Client *c;
    XUnmapEvent *ev = &e->xunmap;
	if (c = wintoclient(ev->window)) {
        if (ev->send_event) setclientstate(c, WithdrawnState);
        else unmanage(c, 0);
    }
}

void updatebarpos(Monitor *m) {
    Client *c;
    int nvis = 0;
    m->wy = m->my;
    m->wh = m->mh;
    m->wh -= BAR_HEIGHT;
    m->by = topbar ? m->wy : m->wy + m->wh;
    if (topbar) m->wy += BAR_HEIGHT;
    for(c = m->clients; c; c = c->next){
      if(ISVISIBLE(c)) ++nvis;
    }
}

void updateclientlist() {
    Client *c;
    Monitor *m;
    XDeleteProperty(dpy, root, netatom[NetClientList]);
    for (m = mons; m; m = m->next)
        for (c = m->clients; c; c = c->next)
            XChangeProperty(dpy, root, netatom[NetClientList],
                XA_WINDOW, 32, PropModeAppend,
                (unsigned char *) &(c->win), 1);
}

int updategeom(void) {
    int dirty = 0;

#ifdef XINERAMA
    if (XineramaIsActive(dpy)) {
        int i, j, n, nn;
        Client *c;
        Monitor *m;
        XineramaScreenInfo *info = XineramaQueryScreens(dpy, &nn);
        XineramaScreenInfo *unique = NULL;

        for (n = 0, m = mons; m; m = m->next, n++);
        unique = ecalloc(nn, sizeof(XineramaScreenInfo));
        for (i = 0, j = 0; i < nn; i++)
            if (isuniquegeom(unique, j, &info[i]))
                memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
        XFree(info);
        nn = j;
        if (n <= nn) { /* new monitors available */
            for (i = 0; i < (nn - n); i++) {
                for (m = mons; m && m->next; m = m->next);
                if (m) m->next = createmon();
                else mons = createmon();
            }
            for (i = 0, m = mons; i < nn && m; m = m->next, i++)
                if (i >= n
                || unique[i].x_org != m->mx || unique[i].y_org != m->my
                || unique[i].width != m->mw || unique[i].height != m->mh)
                {
                    dirty = 1;
                    m->num = i;
                    m->mx = m->wx = unique[i].x_org;
                    m->my = m->wy = unique[i].y_org;
                    m->mw = m->ww = unique[i].width;
                    m->mh = m->wh = unique[i].height;
                    updatebarpos(m);
                }
        } else { /* less monitors available nn < n */
            for (i = nn; i < n; i++) {
                for (m = mons; m && m->next; m = m->next);
                while ((c = m->clients)) {
                    dirty = 1;
                    m->clients = c->next;
                    detachstack(c);
                    c->mon = mons;
                    attachabove(c);
                    attachstack(c);
                }
                if (m == selmon) selmon = mons;
                cleanupmon(m);
            }
        }
        free(unique);
    } else
#endif /* XINERAMA */
    { /* default monitor setup */
        if (!mons) mons = createmon();
        if (mons->mw != sw || mons->mh != sh) {
            dirty = 1;
            mons->mw = mons->ww = sw;
            mons->mh = mons->wh = sh;
            updatebarpos(mons);
        }
    }
    if (dirty) {
        selmon = mons;
        selmon = wintomon(root);
    }
    return dirty;
}

void updatenumlockmask(void) {
    unsigned int i, j;
    XModifierKeymap *modmap;
    numlockmask = 0;
    modmap = XGetModifierMapping(dpy);
    for (i = 0; i < 8; i++)
        for (j = 0; j < modmap->max_keypermod; j++)
            if (modmap->modifiermap[i * modmap->max_keypermod + j]
                == XKeysymToKeycode(dpy, XK_Num_Lock))
                numlockmask = (1 << i);
    XFreeModifiermap(modmap);
}

void update_ws_bools(Monitor *m) {
    unsigned int i, occ = 0, urg = 0;
    Client *c;
    for (c = m->clients; c; c = c->next) {
        occ |= c->tags;
        // TODO: incorporate this urgent into lemonbar
        if (c->isurgent) urg |= c->tags;
    }
    for (i = 0; i < LENGTH(tags); i++) {
        // true if tag has clients, false if not
        if (occ & 1 << i) {
            toggle_dwm_info_ws(i+1, 1);
        } else {
            toggle_dwm_info_ws(i+1, 0);
        }
    }
}

void updatetitle(Client *c) {
    if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
        gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
}

void updatewindowtype(Client *c) {
    Atom state = getatomprop(c, netatom[NetWMState]);
    Atom wtype = getatomprop(c, netatom[NetWMWindowType]);
    if (state == netatom[NetWMFullscreen]) setfullscreen(c, 1);
    if (wtype == netatom[NetWMWindowTypeDialog]) c->isfloating = 1;
}

void updatewmhints(Client *c) {
    XWMHints *wmh;
    if ((wmh = XGetWMHints(dpy, c->win))) {
        if (c == selmon->sel && wmh->flags & XUrgencyHint) {
            wmh->flags &= ~XUrgencyHint;
            XSetWMHints(dpy, c->win, wmh);
        } else c->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;
        if (wmh->flags & InputHint) c->neverfocus = !wmh->input;
        else c->neverfocus = 0;
        XFree(wmh);
    }
}

void view(const Arg *arg) {
    int i, tmptag;

    if ((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags]) return;
    selmon->seltags ^= 1; /* toggle sel tagset */
    if (arg->ui & TAGMASK) {
        selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
        selmon->pertag->prevtag = selmon->pertag->curtag;
        if (arg->ui == ~0) {
            selmon->pertag->curtag = 0;
            set_dwm_info_current_workspace(0);
        } else {
            for (i = 0; !(arg->ui & 1 << i); i++) ;
            selmon->pertag->curtag = i + 1;
            set_dwm_info_current_workspace(i + 1);
        }
    } else {
        tmptag = selmon->pertag->prevtag;
        selmon->pertag->prevtag = selmon->pertag->curtag;
        selmon->pertag->curtag = tmptag;
    }
    selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
    selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
    selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
    selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
    selmon->lt[selmon->sellt^1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt^1];

    focus(NULL);
    arrange(selmon);
}

Client *wintoclient(Window w) {
    Client *c;
    Monitor *m;
    for (m = mons; m; m = m->next)
        for (c = m->clients; c; c = c->next)
            if (c->win == w)
                return c;
    return NULL;
}

Monitor *wintomon(Window w) {
    int x, y;
    Client *c;
    if (w == root && getrootptr(&x, &y)) 
        return recttomon(x, y, 1, 1);
	if (c = wintoclient(w))
        return c->mon;
    return selmon;
}

int xerror(Display *dpy, XErrorEvent *ee) {
    if (ee->error_code == BadWindow
    || (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
    || (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
    || (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
    || (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
    || (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
    || (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
    || (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
    || (ee->request_code == X_CopyArea && ee->error_code == BadDrawable)) return 0;
    fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
        ee->request_code, ee->error_code);
    return xerrorxlib(dpy, ee); /* may call exit */
}

int xerrordummy(Display *dpy, XErrorEvent *ee) { return 0; }

int xerrorstart(Display *dpy, XErrorEvent *ee) {
    die("dwm: another window manager is already running");
    return -1;
}

void togglegaps() {
    // todo: make this PerTag

    if(gappx == 0) {
        gappx = GAP_PX;
        borderpx = BORDER_PX;
        system("${HOME}/bin/composit &");
    } else {
        gappx = 0;
        borderpx = BORDER_PX - BORDER_PX / 2;
        if( borderpx % 2 != 0 ) borderpx++;
        system("${HOME}/bin/composit &");
    }

    // write our new gap config so our bar can read it
    FILE *fgappx = fopen("/tmp/dwm_info/gappx", "w"); fprintf(fgappx, "%d", gappx); fclose(fgappx);

    // restart bar with our new gaps so it resizes
    system("pkill -9 lemonbar ; pkill -9 bar ; bash ${HOME}/bin/bar &");

}

void on_start(void) {
    if(start_with_gaps == 1)
        gappx = GAP_PX;
    else
        gappx = 0;
    borderpx = BORDER_PX;
    init_dwm_info(gappx, BAR_HEIGHT, topbar, NUM_WORKSPACES);

    // Note: this is now in ~/.xinitrc
    /* system("/bin/sh ${HOME}/bin/autostart &"); */
}

int main(int argc, char *argv[]) {
    on_start();
    /* if (argc == 2 && !strcmp("-v", argv[1])) die("dwm-"VERSION); */
    if (!(dpy = XOpenDisplay(NULL))) die("dwm: cannot open display");
    checkotherwm();
    setup();
    scan();
    run();
    cleanup();
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
}
