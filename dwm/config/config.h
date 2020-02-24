/* -*--*-*-*-*-*-*-*-*-*-*- GAPS -*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
// x200
/* static unsigned int gappx = 12; */
/* static unsigned int borderpx  = 2; */
// surface
/* static unsigned int gappx = 18; */
/* static unsigned int borderpx  = 3; */

// no gaps
static unsigned int gappx = 0;
static const unsigned int borderpx  = 1;
/* -*-*-*-*-*-*-*-*-*-*- BAR SIZE -*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
// x200
static const char *fonts[] = { "Terminus:size=8" };
// surface
/* static const char *fonts[] = { "Terminus:size=10" }; */
/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

static const int showbar = 1;
static const int topbar  = 1;
static const int focusonwheel = 1;

#include "/home/mitch/.cache/wal/colors-wal-dwm.h"
/* static const char *colors[][3]      = { */
/*     /1*               fg         bg         border   *1/ */
/*     [SchemeNorm] = { "#bbbbbb", "#222222", "#555555" }, */
/*     [SchemeSel]  = { "#eeeeee", "#eeeeee", "#333333" }, */
/*     [SchemeUrg]  = { "#ff0000", "#ff0000", "#ff0000" }, */
/* }; */

static const int NUM_WORKSPACES=6;
static const char *tags[] = { "1", "2", "3", "4", "5", "6" };

static const Rule rules[] = {
    /* class           instance       title                 tags mask  iscentered   isfloating   monitor */
    { "Gimp",            NULL,       "Gimp",                 0,         0,           1,           -1 },
    { "Image Manipulation Program", NULL, "Image Manipulation Program",  0, 0, 1, -1 },
    { "GNU Image Manipulation Program", NULL, "GNU Image Manipulation Program", 0, 0, 1, -1 },
    { "st",              NULL,       "surf-download",     0,         1,           1,           -1 },
    { "surf-download",   NULL,       "surf-download",     0,         1,           1,           -1 },
    { "floating-st",     NULL,       "floating-st",       0,         1,           1,           -1 },
    { "tabbed",          NULL,       "floating-st",       0,         1,           1,           -1 },
    { "st",              NULL,       "floating-st",       0,         1,           1,           -1 },
    { "st",              NULL,       "x9term",            0,         1,           1,           -1 },
    { "0",               NULL,       "x9term",            0,         1,           1,           -1 },
    { "NULL",            NULL,       "x9term",            0,         1,           1,           -1 },
    { "feh",             NULL,       "feh",               0,         1,           1,           -1 },
    { "meh",             NULL,       "meh",               0,         1,           1,           -1 },
    { "mpv",             NULL,       "mpv",               0,         1,           1,           -1 },
    { "x9term",          NULL,       "x9term",            0,         0,           1,           -1 },
};

static const float mfact       = 0.5;
static const int   nmaster     = 1;
static const int   resizehints = 1;

static const int NUM_LAYOUTS = 3;
static const Layout layouts[] = {
    /* symbol     arrange function */
    { "[T]",      tile },    /* first entry is default */
    { "[F]",      NULL },    /* no layout function means floating behavior */
    { "[M]",      monocle },
};

#define TAGKEYS(KEY,TAG) { Mod1Mask, KEY, view, {.ui = 1 << TAG} }, \
                         { Mod1Mask|ShiftMask, KEY, tag, {.ui = 1 << TAG} },
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

static const char scratchpadname[] = "scratchpad";
static const char *scratchpadcmd[] = { "st", "-t", scratchpadname, "-g", "84x24", NULL };
 
// NOTE: most keybinds for programs I use sxhkd for
static Key keys[] = {
    /* modifier                     key        function        argument */
    { Mod1Mask,                       XK_q,      killclient,     {0} },
    { Mod1Mask,                       XK_j,      focusstack,     {.i = +1 } },
    { Mod1Mask,                       XK_k,      focusstack,     {.i = -1 } },
    { Mod1Mask|ShiftMask,             XK_h,      setmfact,       {.f = -0.05} },
    { Mod1Mask|ShiftMask,             XK_l,      setmfact,       {.f = +0.05} },
    { Mod1Mask,                       XK_t,      setlayout,      {.v = &layouts[0]} },
    { Mod1Mask,                       XK_f,      setlayout,      {.v = &layouts[1]} },
    { Mod1Mask,                       XK_m,      setlayout,      {.v = &layouts[2]} },
    { Mod1Mask,                       XK_space,  togglescratch,  {.v = scratchpadcmd } },
    { Mod1Mask,                       XK_s,      togglesticky,   {0} },
    { Mod1Mask|ShiftMask,             XK_space,  togglefloating, {0} },
    { Mod1Mask,                       XK_h,      rotatestack,    {.i = -1 } },
    { Mod1Mask,                       XK_l,      rotatestack,    {.i = +1 } },
    { Mod1Mask,                       XK_Tab,    view,           {0} },
    { Mod1Mask|ShiftMask|ControlMask, XK_q,      quit,           {0} },

    TAGKEYS(XK_1,0) TAGKEYS(XK_2,1) TAGKEYS(XK_3,2) TAGKEYS(XK_4,3) 
    TAGKEYS(XK_5,4) TAGKEYS(XK_6,5) TAGKEYS(XK_BackSpace,5)
};

static Button buttons[] = {
    { ClkClientWin,         Mod1Mask,       Button1,        movemouse,      {0} },
    { ClkClientWin,         Mod1Mask,       Button3,        resizemouse,    {0} },
    /* { ClkClientWin,         Mod4Mask,       Button1,        resizemouse,    {0} }, */
    { ClkRootWin,           0,              Button3,        spawn,          SHCMD("x9term") },
};
