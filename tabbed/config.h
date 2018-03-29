/* static const char font[]        = "Share Tech Mono:pixelsize=18:antialias=true:autohint=true"; */
static const char font[]        = "Share Tech Mono:pixelsize=14:antialias=true:autohint=true";
/* static const char font[]        = "Share Tech Mono:pixelsize=12:antialias=true:autohint=true"; */
/* static const char font[]        = "Terminus:pixelsize=14:antialias=false:autohint=false"; */
/* static const char font[]        = "Terminus:pixelsize=12:antialias=false:autohint=false"; */
/* static const char font[]        = "MonteCarlo:pixelsize=12:antialias=false:autohint=false"; */
/* static const char font[]        = "Roboto Mono:pixelsize=14:antialias=true:autohint=true"; */
/* static const char font[]        = "Roboto Mono:pixelsize=16:antialias=true:autohint=true"; */

// whether to append 1., 2., 3. etc to the start of clients
static const Bool numberclients = False;

static const int WIDTH=500;
static const int HEIGHT=400;

/* set to -1 if you want to use the text's height */
/* static const int barheight = 52; */ 
/* static const int barheight = 36; */ 
/* static const int barheight = 24; */ 
static const int barheight = -1; 

/* #include "normal-theme.h" */
#include "/home/mitch/.cache/wal/colors-wal-tabbed.h" // wal

static const char before[]      = "<";
static const char after[]       = ">";
// what to show in titles after truncation
static const char titletrim[]   = "...";
// the minimum size a tab can be
static const int  tabwidth      = 80;
// whether clients that are spawned automatically appear
// in their new tab focused, or backgrounded
// (keeping the previously opened tab still open)
static const Bool foreground    = False;
// whether to automatically focus urgent windows
static const Bool urgentswitch  = False;

/* Where to place a new tab when it is opened. When npisrelative is True,
 * then the current position is changed + newposition. If npisrelative
 * is False, then newposition is an absolute position.  */
static int newposition   = 1;
static Bool npisrelative  = True;

#define SETPROP(p) { \
        .v = (char *[]){ "/bin/sh", "-c", \
                "prop=\"`xwininfo -children -id $1 | grep '^     0x' |" \
                "sed -e's@^ *\\(0x[0-9a-f]*\\) \"\\([^\"]*\\)\".*@\\1 \\2@' |" \
                "xargs -0 printf %b | dmenu -l 10`\" &&" \
                "xprop -id $1 -f $0 8s -set $0 \"$prop\"", \
                p, winid, NULL \
        } \
}

static const Key keys[] = {
    /* modifier             key        function     argument */
    { ControlMask,     XK_n, focusonce,   { 0 } },
    { ControlMask,     XK_n, spawn,       { 0 } },

     /* ------ shifts to next tab ------ */
    { ControlMask|ShiftMask,     XK_k,      rotate,      { .i = +1 } },
    { ControlMask|ShiftMask,     XK_j,      rotate,      { .i = -1 } },

    { ControlMask,               XK_Tab,    rotate,      { .i = +1 } },
    { ControlMask|ShiftMask,     XK_Tab,    rotate,      { .i = -1 } },
    { ShiftMask,                 XK_Tab,    rotate,      { .i = 0 } }, // back-&-forth

    /* ------- moves a tab left or right in stack ---- */
    { ControlMask|ShiftMask,     XK_h,      movetab,     { .i = -1 } },
    { ControlMask|ShiftMask,     XK_l,      movetab,     { .i = +1 } },
    { ControlMask,               XK_w,      killclient,  { 0 } },

    /* this opens dmenu to rifle through tabs */
    /* { ControlMask,               XK_t,      spawn,       SETPROP("_TABBED_SELECT_TAB") }, */

    { ControlMask,               XK_1,      move,        { .i = 0 } },
    { ControlMask,               XK_2,      move,        { .i = 1 } },
    { ControlMask,               XK_3,      move,        { .i = 2 } },
    { ControlMask,               XK_4,      move,        { .i = 3 } },
    { ControlMask,               XK_5,      move,        { .i = 4 } },
    { ControlMask,               XK_6,      move,        { .i = 5 } },
    { ControlMask,               XK_7,      move,        { .i = 6 } },
    { ControlMask,               XK_8,      move,        { .i = 7 } },
    { ControlMask,               XK_9,      move,        { .i = 8 } },
    { ControlMask,               XK_0,      move,        { .i = 9 } },
    /* { ControlMask,               XK_u,      focusurgent, { 0 } }, */
    /* { ControlMask|ShiftMask,     XK_u,      toggle,      { .v = (void*) &urgentswitch } }, */
    { 0,                         XK_F11,    fullscreen,  { 0 } },
};
