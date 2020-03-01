/* -*--*-*-*-*-*-*-*-*- GAPS -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
static unsigned int gappx = 16;
static unsigned int borderpx  = 0;
/* -*-*-*-*-*-*-*-*-*- NO GAPS *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
/* static unsigned int gappx = 0; */
/* static const unsigned int borderpx  = 1; */
/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

// for use with the rounded corners patch (0 disables)
static const int CORNER_RADIUS = 0;
/* static const int CORNER_RADIUS = 12; */

static const unsigned int minwsz = 20;   /* min height of a client for smfact */
static const int vertpad         = 10;   /* vertical padding of bar */
static const int sidepad         = 11;   /* horizontal padding of bar */
static const int horizpadbar = 3; // horizontal padding for statusbar
static const int vertpadbar  = 10; // vertical padding for statusbar

static const char *fonts[] = {
    "Terminus:size=8",
    "RobotoMono Nerd Font:size=10"
};

static const Rule rules[] = {
    /* class          instance    title    tags mask  iscentered   isfloating   monitor */
    { "st-256color",  NULL,      0,             0,         1,           0,           -1 },
    { 0,              NULL,      "x9term",      0,         0,           1,           -1 },
    { 0,              NULL,      "floating-st", 0,         1,           1,           -1 },
    { "feh",          NULL,      0,             0,         1,           1,           -1 },
    { "mpv",          NULL,      0,             0,         1,           1,           -1 },
    { "mupdf",        NULL,      0,             0,         1,           1,           -1 },
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
/*     /1*               fg           bg         border                         *1/ */
/*     [SchemeNorm] = { norm_fg,     norm_bg,   norm_border }, // unfocused wins */
/*     [SchemeSel]  = { sel_fg,      sel_bg,    sel_border },  // the focused win */
/*     [SchemeUrg] =  { urg_fg,      urg_bg,    urg_border }, */
/* }; */

static const char *tags[] = { "爵", "", "", "ﭮ" };
static const int showbar = 1;
static const int topbar  = 1;
static const float mfact = 0.5;
static const Layout layouts[] = {
    { "|  ", tile },
    { "| 缾 ", NULL }, // floating
    { "|  ", monocle },
};
#define TAGKEYS(KEY,TAG) { Mod1Mask, KEY, view, {.ui = 1 << TAG} }, \
                         { Mod1Mask|ShiftMask, KEY, tag, {.ui = 1 << TAG} },
#define SH(cmd) { .v = (const char*[]) { "/bin/sh", "-r", "-c", cmd, NULL } }
static const char *term[] = { "st", NULL };

static Key keys[] = {
    /* modifier            key        function       argument */
    /* -*-*-*-*-*-*-*- programs -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    { Mod1Mask|ShiftMask,   XK_Return, spawn,        SH("tabbed -d -c -r 2 st -w ''") },
    { Mod1Mask,             XK_Return, spawn,        { .v = term, }         },
    { Mod1Mask,             XK_p,      spawn,        SH("menu run -p Run:") },
    { Mod1Mask,             XK_r,      spawn,        SH("st -e ranger")     },
    { Mod1Mask,             XK_w,      spawn,        SH("tabbed -d -c surf -e") },
    { Mod1Mask|ShiftMask,   XK_w,      spawn,        SH("chrome --new-window")  },
    { Mod1Mask,             XK_x,      spawn,        SH("lck")              },
    { Mod1Mask,             XK_o,      spawn,        SH("dedit")            },
    { Mod1Mask,             XK_i,      spawn,        SH("tasks")            },
    { Mod1Mask,             XK_c,      spawn,        SH("clip")             },
    { ControlMask,          XK_Print,  spawn,        SH("scrap")            },
    { 0,                    XK_Print,  spawn,        SH("scrap -n")         },
    { Mod1Mask|ControlMask, XK_k,      spawn,        SH("keylayout")        },
    /* -*-*-*-*-*-*-*- media control -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
    { Mod1Mask,             XK_apostrophe,   spawn,  SH("vol -i 6") },
    { Mod1Mask,             XK_semicolon,    spawn,  SH("vol -d 6") },
    { Mod1Mask,             XK_slash,        spawn,  SH("mmt -t")   },
    { Mod1Mask,             XK_period,       spawn,  SH("mmt -n")   },
    { Mod1Mask,             XK_comma,        spawn,  SH("mmt -p")   },
    { Mod1Mask,             XK_bracketright, spawn,  SH("mmt -f")   },
    { Mod1Mask,             XK_bracketleft,  spawn,  SH("mmt -b")   },
    /* -*-*-*-*-*-*-*- dwm commands -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    { Mod1Mask,            XK_space,  togglescratch, SH("st -t scratchpad -g 80x22") },
    { Mod1Mask,            XK_q,      killclient,    {0} },
    { Mod1Mask,            XK_j,      focusstack,    {.i = +1 } },
    { Mod1Mask,            XK_k,      focusstack,    {.i = -1 } },
    { Mod1Mask|ShiftMask,  XK_h,      setmfact,      {.f = -0.05} },
    { Mod1Mask|ShiftMask,  XK_l,      setmfact,      {.f = +0.05} },
    { Mod1Mask,            XK_t,      setlayout,     {.v = &layouts[0]} },
    { Mod1Mask,            XK_f,      setlayout,     {.v = &layouts[1]} },
    { Mod1Mask,            XK_m,      setlayout,     {.v = &layouts[2]} },
    { Mod1Mask,            XK_b,      togglebar,     {0} },
    { Mod1Mask,            XK_s,      togglesticky,  {0} },
    { Mod1Mask|ShiftMask,  XK_space,  togglefloating,{0} },
    { Mod1Mask,            XK_h,      rotatestack,   {.i = -1 } },
    { Mod1Mask,            XK_l,      rotatestack,   {.i = +1 } },
    { Mod1Mask,            XK_Tab,    view,          {0} },
    { Mod1Mask|ShiftMask|ControlMask, XK_q,      quit,           {0} },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    { Mod1Mask|ShiftMask,  XK_k,      setsmfact,      {.f = +0.05} },
    { Mod1Mask|ShiftMask,  XK_j,      setsmfact,      {.f = -0.05} },
    { Mod1Mask,            XK_g,      setgaps,        {.i = +4}    },
    { Mod1Mask|ShiftMask,  XK_g,      setgaps,        {.i = -4}    },
    { 0,                   XK_F11,    togglefullscr,  {0} },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    TAGKEYS(XK_1,0) TAGKEYS(XK_2,1) TAGKEYS(XK_3,2) TAGKEYS(XK_4,3) 
    TAGKEYS(XK_5,4) TAGKEYS(XK_6,5) 
    // -- if 4 WS:
    TAGKEYS(XK_BackSpace,3)
    // -- if 6 WS:
    /* TAGKEYS(XK_BackSpace,5) */
};

static Button buttons[] = {
    { ClkClientWin,         Mod1Mask,       Button1,        movemouse,      {0} },
    { ClkClientWin,         Mod1Mask,       Button3,        resizemouse,    {0} },
    { ClkRootWin,           0,              Button3,        spawn,          SH("x9term") },
};

/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
static const float smfact     = 0.00; /* factor of tiled clients [0.00..0.95] */
static const int resizehints  = 1;
static const int focusonwheel = 1;
static const char scratchpadname[] = "scratchpad";
static const int nmaster      = 1;
