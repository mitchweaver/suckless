/* -*--*-*-*-*-*-*-*-*- GAPS -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
#define GAPS_START 20
#define BORDERPX_START 0
/* -*-*-*-*-*-*-*-*-*- SMALL GAPS *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
/* #define GAPS_START 8 */
/* #define BORDERPX_START 1 */
/* -*-*-*-*-*-*-*-*-*- NO GAPS *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
/* #define GAPS_START 0 */
/* #define BORDERPX_START 1 */
/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

/* -*-*-*-*-*-*-*-*- FLOATING BAR -*-*-*-*-*-*-*-*-*-*-*-*-*-* */
static const int vertpad     = GAPS_START - GAPS_START / 3; // vertical padding of bar
static const int sidepad     = GAPS_START - GAPS_START / 3; // horizontal padding of bar
static const int horizpadbar = 6;          // horizontal padding for statusbar
static const int vertpadbar  = 12;         // vertical padding for statusbar
/* -*-*-*-*-*-*-*-* NON-FLOATING BAR -*-*-*-*-*-*-*-*-*-*-*-*- */
/* static const int vertpad     = 0; // vertical padding of bar */
/* static const int sidepad     = 0; // horizontal padding of bar */
/* static const int horizpadbar = 2; // horizontal padding for statusbar */
/* static const int vertpadbar  = 4; // vertical padding for statusbar */
/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */

// for use with the rounded corners patch (0 disables)
static const int CORNER_RADIUS = 0;

static const char*fonts[] = {
    "Terminus:size=8",
    "ShureTechMono Nerd Font:size=10",
    "RobotoMono Nerd Font Mono:size=10"
};

static const Rule rules[] = {
    /* class         instance  title      tags mask  iscentered   isfloating  monitor */
    { "brws",        NULL,     0,         1,         0,           0,          -1 },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
    { "tabbed",      NULL,     0,         0,         1,           0,          -1 },
    { "x9term",      NULL,     0,         0,         0,           1,          -1 },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
    { "feh",         NULL,     0,         0,         1,           1,          -1 },
    { "mpv",         NULL,     0,         0,         1,           1,          -1 },
    { "MuPDF",       NULL,     0,         0,         1,           0,          -1 },
};

#include "/home/mitch/.cache/wal/colors-wal-dwm.h"
/* static const char norm_fg[] = "#d4ddda"; */
/* static const char norm_bg[] = "#062C39"; */
/* static const char norm_border[] = "#949a98"; */
/* static const char sel_fg[] = "#d4ddda"; */
/* static const char sel_bg[] = "#494262"; */
/* static const char sel_border[] = "#d4ddda"; */
/* static const char urg_fg[] = "#d4ddda"; */
/* static const char urg_bg[] = "#45405F"; */
/* static const char urg_border[] = "#45405F"; */
/* static const char *colors[][3]      = { */
/* /1*               fg           bg         border                         *1/ */
/* [SchemeNorm] = { norm_fg,     norm_bg,   norm_border }, // unfocused wins */
/* [SchemeSel]  = { sel_fg,      sel_bg,    sel_border },  // the focused win */
/* [SchemeUrg] =  { urg_fg,      urg_bg,    urg_border }, */
/* }; */

/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/*
 * cool emojis to use for tags:
 *
 * "", "爵", "", "", "ﭮ", "", "", "", "","", "﬐", "ﳨ"
 * "ﯙ", "", "", "", "", "", "", "", "", "", "", ""
 * "", "", "", "", "", "", "", "", "", "", "", "", "ﭮ"
 */
static const char *tags[] = { "", "", "", "", "", "" };
/* static const char *tags[] = { "1","2","3","4","5","6" }; */
/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

static const int showbar = 1;
static const int topbar  = 1;
static const float mfact = 0.5;
static const Layout layouts[] = {
    { "|  ", tile },
    { "| 缾 ", NULL }, // floating
    /* { "|  ", monocle }, */
};

#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) { MODKEY, KEY, view, {.ui = 1 << TAG} }, \
                         { MODKEY|ShiftMask, KEY, tag, {.ui = 1 << TAG} },
#define SH(cmd) { .v = (const char*[]) { "/bin/sh", "-r", "-c", cmd, NULL } }

static Key keys[] = {
    /* modifier            key        function       argument */
    /* -*-*-*-*-*-*-*- programs -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    { MODKEY,             XK_Return, spawn,        SH("term")             },
    { MODKEY,             XK_p,      spawn,        SH("menu run -p Run:") },
    { MODKEY,             XK_r,      spawn,        SH("st -e ranger --cmd='set viewmode multipane'") },

    { MODKEY,             XK_w,      spawn,        SH("brws")             },
    /* { MODKEY|ShiftMask,   XK_w,      spawn,        SH("tabbed -d -c surf -e") }, */
    { MODKEY,             XK_u,      spawn,        SH("ddg")              },

    { MODKEY,             XK_x,      spawn,        SH("lck")              },
    { MODKEY,             XK_o,      spawn,        SH("dedit")            },
    { MODKEY,             XK_i,      spawn,        SH("tasks")            },
    { MODKEY,             XK_c,      spawn,        SH("clip")             },
    { ControlMask,        XK_Print,  spawn,        SH("scrap")            },
    { 0,                  XK_Print,  spawn,        SH("scrap -n")         },
    { MODKEY|ControlMask, XK_k,      spawn,        SH("keys")             },
    /* -*-*-*-*-*-*-*- media control -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
    { MODKEY,             XK_apostrophe,   spawn,  SH("vol -i 6") },
    { MODKEY,             XK_semicolon,    spawn,  SH("vol -d 6") },
    { MODKEY,             XK_slash,        spawn,  SH("mmt -t")   },
    { MODKEY,             XK_period,       spawn,  SH("pkill skroll ; mmt -n")   },
    { MODKEY,             XK_comma,        spawn,  SH("pkill skroll ; mmt -p")   },
    { MODKEY,             XK_bracketright, spawn,  SH("mmt -f")   },
    { MODKEY,             XK_bracketleft,  spawn,  SH("mmt -b")   },
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
    { 0,                   XK_F11,    togglefullscr,  {0} },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    TAGKEYS(XK_1,0) TAGKEYS(XK_2,1) TAGKEYS(XK_3,2) TAGKEYS(XK_4,3) 
    TAGKEYS(XK_5,4) TAGKEYS(XK_6,5) 
    // -- if 4 WS:
    /* TAGKEYS(XK_BackSpace,3) */
    // -- if 6 WS:
    TAGKEYS(XK_BackSpace,5)
};

static Button buttons[] = {
    { ClkClientWin,  MODKEY,  Button1, movemouse,   {0} },
    { ClkClientWin,  MODKEY,  Button3, resizemouse, {0} },
    { ClkRootWin,    0,       Button3, spawn,       SH("9m") },
	{ ClkTagBar,     0,       Button1, view,        {0} },
};

static unsigned int gappx = GAPS_START;
static unsigned int borderpx = BORDERPX_START;

static const unsigned int minwsz = 20; /* min height of a client for smfact */

/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
static const float smfact     = 0.00; /* factor of tiled clients [0.00..0.95] */
static const int resizehints  = 1;
static const int focusonwheel = 1;
static const char scratchpadname[] = "scratchpad";
static const int nmaster      = 1;
