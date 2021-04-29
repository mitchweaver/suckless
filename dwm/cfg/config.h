/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
#define GAPS_START 26
#define BORDERPX_START 0
/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

static const int topbar  = 1;

/* -*-*-*-*-*-*-*-*- FLOATING BAR -*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* static const int vertpad     = GAPS_START - GAPS_START / 3; // vertical padding of bar */
/* static const int sidepad     = GAPS_START - GAPS_START / 3; // horizontal padding of bar */
/* static const int horizpadbar = 6;   // horizontal padding for statusbar */
/* static const int vertpadbar  = 10;  // vertical padding for statusbar */
/* -*-*-*-*-*-*-*-* NON-FLOATING BAR -*-*-*-*-*-*-*-*-*-*-*-*- */
static const int vertpad     = 0; // vertical padding of bar
static const int sidepad     = 0; // horizontal padding of bar
static const int horizpadbar = 2; // horizontal padding for statusbar
static const int vertpadbar  = 4; // vertical padding for statusbar
/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */

// for use with the rounded corners patch (0 disables)
static const int CORNER_RADIUS = 0;

// defined as 0-255 in hex (unsigned), with 255 being opaque
//
// common hex values:
// 100%  =  1.0 * 255 = 255, in hex: 0xffu 
// 90%   =  0.9 * 255 = 228, in hex: 0xe4u
// 80%   =  0.8 * 255 = 182, in hex: 0xb6u
//
// OPAQUE = 0xffu, defined in the patch
static const unsigned int baralpha = 0xe4u;
static const unsigned int borderalpha = OPAQUE;
static const unsigned int alphas[][3] = {
	/*               fg      bg        border     */
	[SchemeNorm] = { OPAQUE, baralpha, borderalpha },
	[SchemeSel]  = { OPAQUE, baralpha, borderalpha },
};

// theme, included from ${HOME}/.cache/themes in config.mk
#include <dwm.h>

static const char*fonts[] = {
    "Terminus:pixelsize=16",
    "Terminess (TTF) Nerd Font:pixelsize=16",
    "Shure Tech Mono Nerd Font:pixelsize=16",
};

static const Rule rules[] = {
    /* class         instance  title       tags mask  iscentered   isfloating  monitor */
    { "brws",        NULL,     NULL,       1,         0,           0,          -1 },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
    { "pop",         NULL,     "pop",      0,         0,           1,          -1 },
    { "term",        NULL,     "term",     0,         1,           0,          -1 },
    { "x9term",      NULL,     "x9term",   0,         0,           1,          -1 },
    { "floating-st", NULL,     "floating-st", 0,      1,           1,          -1 },
    { "Simple Terminal", NULL, NULL,       0,         1,           0,          -1 },
    { NULL,          NULL, "Simple Terminal",       0,         1,           0,          -1 },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
    { "htop",        NULL,     NULL,       0,         1,           1,          -1 },
    { "ranger",      NULL,     NULL,       0,         1,           1,          -1 },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
    { "feh",         NULL,     NULL,       0,         1,           1,          -1 },
    { "mpv",         NULL,     NULL,       0,         1,           1,          -1 },
    { "sxiv",        NULL,     NULL,       0,         1,           1,          -1 },
    { "Sxiv",        NULL,     NULL,       0,         1,           1,          -1 },
    { "pcmanfm",     NULL,     NULL,       0,         1,           1,          -1 },
    { "Pcmanfm",     NULL,     NULL,       0,         1,           1,          -1 },
    { "mupdf",       NULL,     NULL,       0,         1,           0,          -1 },
    { "MuPDF",       NULL,     NULL,       0,         1,           0,          -1 },
    { "virt-manager", NULL,    NULL,       0,         1,           1,          -1 },
    { "Virt-manager", NULL,    NULL,       0,         1,           1,          -1 },
};

/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/*
 * cool emojis to use for tags:
 *
 * "", "爵", "", "", "ﭮ", "", "", "", "","", "﬐", "ﳨ"
 * "ﯙ", "", "", "", "", "", "", "", "", "", "", "" ""
 * "", "", "", "", "", "", "", "", "", "", "", "", "ﭮ"
 */
static const char *tags[] = { "","","", "","","" };
/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

static const int showbar = 1;
static const float mfact = 0.5;
static const Layout layouts[] = {
    { "|  ", tile },
    { "| 缾", NULL }, // floating
    /* { "|  ", NULL }, // floating */
    /* { "| C ", monocle }, */
};

#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) { MODKEY, KEY, view, {.ui = 1 << TAG} }, \
                         { MODKEY|ShiftMask, KEY, tag, {.ui = 1 << TAG} },
#define SH(cmd) { .v = (const char*[]) { "/bin/sh", "-c", cmd, NULL } }

// need this include for brightness/audio keys
#include <X11/XF86keysym.h>
static Key keys[] = {
    //
    // NOTE: regular program bindings now handled in sxhkdrc
    //

    /* modifier            key        function       argument */
    /* -*-*-*-*-*-*-*- dwm commands -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    { MODKEY,            XK_space,  togglescratch, SH("st -t scratchpad -g 68x18") },
    { MODKEY,            XK_q,      killclient,    {0} },
    { MODKEY,            XK_j,      focusstack,    {.i = +1 } },
    { MODKEY,            XK_k,      focusstack,    {.i = -1 } },
    { MODKEY|ShiftMask,  XK_h,      setmfact,      {.f = -0.05} },
    { MODKEY|ShiftMask,  XK_l,      setmfact,      {.f = +0.05} },
    { MODKEY,            XK_t,      setlayout,     {.v = &layouts[0]} },
    { MODKEY,            XK_f,      setlayout,     {.v = &layouts[1]} },
    /* { MODKEY,            XK_m,      setlayout,     {.v = &layouts[2]} }, */
    { MODKEY,            XK_b,      togglebar,     {0} },
    { MODKEY,            XK_s,      togglesticky,  {0} },
    { MODKEY|ShiftMask,  XK_space,  togglefloating,{0} },
    { MODKEY,            XK_h,      rotatestack,   {.i = -1 } },
    { MODKEY,            XK_l,      rotatestack,   {.i = +1 } },
    { MODKEY,            XK_Tab,    view,          {0} },
    { MODKEY|ShiftMask|ControlMask, XK_q,      quit,           {0} },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    { MODKEY|ShiftMask,  XK_k,      setsmfact,      {.f = +0.05} },
    { MODKEY|ShiftMask,  XK_j,      setsmfact,      {.f = -0.05} },
    { MODKEY,            XK_g,      setgaps,        {.i = +4}    },
    { MODKEY|ShiftMask,  XK_g,      setgaps,        {.i = -4}    },
    { 0,                 XK_F11,    togglefullscr,  {0} },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    TAGKEYS(XK_1,0) TAGKEYS(XK_2,1) TAGKEYS(XK_3,2) TAGKEYS(XK_4,3) 
    TAGKEYS(XK_5,4) TAGKEYS(XK_6,5) 
    TAGKEYS(XK_BackSpace,5)
};

static Button buttons[] = {
    { ClkClientWin,  MODKEY,      Button1, movemouse,   {0} },
    { ClkClientWin,  MODKEY,      Button3, resizemouse, {0} },
	{ ClkTagBar,     0,           Button1, view,        {0} },
    { ClkRootWin,    0,           Button3, spawn,       SH("x9term") },
    /* { ClkAny,        ControlMask, Button3, spawn,       SH("opn -c") }, */
};

// decor hints patch
static const int decorhints  = 1;

static unsigned int gappx = GAPS_START;
static unsigned int borderpx = BORDERPX_START;

static const unsigned int minwsz = 20; /* min height of a client for smfact */

/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
static const float smfact     = 0.00; /* factor of tiled clients [0.00..0.95] */
static const int resizehints  = 1;
static const int focusonwheel = 1;
static const char scratchpadname[] = "scratchpad";
static const int nmaster      = 1;
