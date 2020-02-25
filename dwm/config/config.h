/* -*--*-*-*-*-*-*-*-*- GAPS -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
static unsigned int gappx = 14;
static unsigned int borderpx  = 0;
/* -*-*-*-*-*-*-*-*-*- NO GAPS *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
/* static unsigned int gappx = 0; */
/* static const unsigned int borderpx  = 1; */
/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

// for use with the rounded corners patch:
static const int CORNER_RADIUS = 11;

static const Rule rules[] = {
    /* class             instance    title    tags mask  iscentered   isfloating   monitor */
    { "st-256color",     NULL,       0,       0,         1,           0,           -1 },
    { "feh",             NULL,       0,       0,         1,           1,           -1 },
    { "mpv",             NULL,       0,       0,         1,           1,           -1 },
    { "mupdf",           NULL,       0,       0,         1,           1,           -1 },
};

#include "/home/mitch/.cache/wal/colors-wal-dwm.h"
static const char *fonts[] = { "tewi:size=8" };
/* static const char *fonts[] = { "Terminus:size=8" }; */
static const char *tags[] = { "1", "2", "3", "4", "5", "6" };
static const int showbar = 1;
static const float mfact      = 0.5;
static const int nmaster      = 1;
static const int resizehints  = 1;
static const int focusonwheel = 1;
static const int topbar  = 1;
static const char scratchpadname[] = "scratchpad";
static const Layout layouts[] = {
    { "T", tile },
    { "F", NULL }, // floating
    { "M", monocle },
};
#define TAGKEYS(KEY,TAG) { Mod1Mask, KEY, view, {.ui = 1 << TAG} }, \
                         { Mod1Mask|ShiftMask, KEY, tag, {.ui = 1 << TAG} },
#define SH(cmd) { .v = (const char*[]) { "/bin/sh", "-r", "-c", cmd, NULL } }
static const char *term[] = { "st", NULL };

static Key keys[] = {
    /* modifier            key        function       argument */
    /* -*-*-*-*-*-*-*- programs -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    /* { Mod1Mask|ShiftMask,   XK_Return, spawn,        SH("tabbed -d -c -r 2 st -w ''") }, */
    { Mod1Mask,             XK_Return, spawn,        { .v = term, }         },
    { Mod1Mask,             XK_p,      spawn,        SH("menu run -p Run:") },
    { Mod1Mask,             XK_r,      spawn,        SH("st -e ranger")     },
    { Mod1Mask,             XK_w,      spawn,        SH("brws")             },
    { Mod1Mask,             XK_x,      spawn,        SH("lck")              },
    { Mod1Mask,             XK_o,      spawn,        SH("dedit")            },
    { Mod1Mask,             XK_i,      spawn,        SH("tasks")            },
    { Mod1Mask,             XK_c,      spawn,        SH("clip")             },
    { ControlMask,          XK_Print,  spawn,        SH("scrap")            },
    { NULL,                 XK_Print,  spawn,        SH("scrap -n")         },
    { Mod1Mask|ControlMask, XK_k,      spawn,        SH("keylayout")        },
    /* -*-*-*-*-*-*-*- media control -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
    { Mod1Mask,             XK_apostrophe,   spawn,  SH("vol -inc 8")       },
    { Mod1Mask,             XK_semicolon,    spawn,  SH("vol -dec 8")       },
    { Mod1Mask,             XK_slash,        spawn,  SH("mpvc toggle")      },
    { Mod1Mask,             XK_period,       spawn,  SH("mpvc --track +1")  },
    { Mod1Mask,             XK_comma,        spawn,  SH("mpvc --track -1")  },
    { Mod1Mask,             XK_bracketright, spawn,  SH("mpvc --seek +30")  },
    { Mod1Mask,             XK_bracketleft,  spawn,  SH("mpvc --seek -30")  },
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
    TAGKEYS(XK_1,0) TAGKEYS(XK_2,1) TAGKEYS(XK_3,2) TAGKEYS(XK_4,3) 
    TAGKEYS(XK_5,4) TAGKEYS(XK_6,5) TAGKEYS(XK_BackSpace,5)
};

static Button buttons[] = {
    { ClkClientWin,         Mod1Mask,       Button1,        movemouse,      {0} },
    { ClkClientWin,         Mod1Mask,       Button3,        resizemouse,    {0} },
    { ClkRootWin,           0,              Button3,        spawn,          SH("x9term") },
};
