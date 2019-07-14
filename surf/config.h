#include "surf-configh-ignore.h"

/* --------- function to launch arbitrary commands ---------------- */
#define SH(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }
/* ------------------------------------------------------------------ */

// http://git.gnome.org/browse/gtk+/plain/gdk/gdkkeysyms.h
#define MODKEY GDK_CONTROL_MASK
#define SHIFT GDK_SHIFT_MASK
static const Key keys[] = {
    { MODKEY,               GDK_KEY_g,       spawn,      SETPROP("_SURF_URI", "_SURF_GO", PROMPT_GO) },
    { MODKEY,               GDK_KEY_slash,   spawn,      SETSEARCH("_SURF_FIND", "_SURF_FIND", PROMPT_FIND) },
    { MODKEY,               GDK_KEY_period,  find,       { .i = +1 } },
    { MODKEY,               GDK_KEY_comma,   find,       { .i = -1 } },
    { MODKEY,               GDK_KEY_r,       reload,     { .i = 0 } },
    { 0,                    GDK_KEY_F5,      reload,     { .i = 0 } },
    { 0,                    GDK_KEY_Escape,  stop,       { 0 } },
    { MODKEY,               GDK_KEY_c,       stop,       { 0 } },

    /* ----------------- Custom Functions ---------------------------- */
    { MODKEY,               GDK_KEY_y,      spawn,      YOUTUBEDL },
    { MODKEY|SHIFT,         GDK_KEY_b,      spawn,      BM_ADD },
    { MODKEY,               GDK_KEY_t,      spawn,      SH("python3.6 ${HOME}/etc/suckless-tools/surf/scripts-surf/surf-translate.py \"$(xsel -o)\"") },
    { MODKEY,               GDK_KEY_space,  spawn,      GO_HOME  },
    /* ----------------- End Custom Functions ------------------------ */

    /* ---------------------- History -------------------------------- */
    { MODKEY,               GDK_KEY_o,   navigate,   { .i = +1 } },
    { MODKEY,               GDK_KEY_i,   navigate,   { .i = -1 } },
    /* --------------------------------------------------------------- */

    /* -------------------- Zooming -------------------------------------- */
    { MODKEY,                GDK_KEY_minus,       zoom,       { .i = -1 } },
    { MODKEY,                GDK_KEY_plus,        zoom,       { .i = +1 } },
    { MODKEY,                GDK_KEY_equal,       zoom,       { .i = +1 } },
    { MODKEY,                GDK_KEY_BackSpace,   zoom,       { .i = +0 } },
    { MODKEY|SHIFT,          GDK_KEY_j,           zoom,       { .i = -1 } },
    { MODKEY|SHIFT,          GDK_KEY_k,           zoom,       { .i = +1 } },
    /* ------------------------------------------------------------------- */

    /* ---------------------- vim mode ----------------------------------- */
    { MODKEY,               GDK_KEY_j,      scroll,     { .i = 'd' } },
    { MODKEY,               GDK_KEY_k,      scroll,     { .i = 'u' } },
    { MODKEY,               GDK_KEY_l,      scroll,     { .i = 'r' } },
    { MODKEY,               GDK_KEY_h,      scroll,     { .i = 'l' } },
    { MODKEY,               GDK_KEY_f,      scroll,     { .i = 'U' } },
    { MODKEY,               GDK_KEY_b,      scroll,     { .i = 'D' } },
    /* -------------------------------------------------------------------------- */

    /* --------------------- Toggles -------------------------------------- */
    { MODKEY|SHIFT,          GDK_KEY_s,      toggle,     { .i = JavaScript } },
    { MODKEY|SHIFT,          GDK_KEY_m,      toggle,     { .i = Style } },
    { MODKEY|SHIFT,          GDK_KEY_t,      toggle,     { .i = StrictTLS } },
    { MODKEY|SHIFT,          GDK_KEY_i,      toggle,     { .i = Images } },
    /* --------------------------------------------------------------------------- */
    { MODKEY,                GDK_KEY_p,      print,      { 0 } },
};

/* target can be OnDoc, OnLink, OnImg, OnMedia, OnEdit, OnBar, OnSel, OnAny */
static Button buttons[] = {
    /* target       event mask      button  function        argument        stop event */
    /* { OnLink,       MODKEY,         1,      clicknewwindow, { .i = 1 },     1 }, */
    /* { OnMedia,      MODKEY,         1,      clickexternplayer, { 0 },       1 }, */
    /* { OnAny,        0,              4,      clicknavigate,  { .i = -1 },    1 }, */
    /* { OnAny,        0,              5,      clicknavigate,  { .i = +1 },    1 }, */
    { OnAny,        0,              4,      scroll,  { .i = 'U' },    0 },
    { OnAny,        0,              5,      scroll,  { .i = 'D' },    0 },

};

static Parameter defconfig[ParameterLast] = {
    [FontSize]            =       { { .i = 14 },    },
    [ZoomLevel]           =       { { .f = 1.1 },   },
    [DNSPrefetch]         =       { { .i = 1 },     },
    [JavaScript]          =       { { .i = 1 },     },
    [StrictTLS]           =       { { .i = 1 },     },
    [Style]               =       { { .i = 1 },     },
    [Images]              =       { { .i = 1 },     },
    [WebGL]               =       { { .i = 1 },     },
};
