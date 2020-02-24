/* -*--*-*-*-*-*-*-*-*- GAPS -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
static unsigned int gappx = 12;
static unsigned int borderpx  = 2;
/* -*-*-*-*-*-*-*-*-*- NO GAPS *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
/* static unsigned int gappx = 0; */
/* static const unsigned int borderpx  = 1; */
/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

static const char *fonts[] = { "Terminus:size=8" };
static const int showbar = 1;

static const int NUM_WORKSPACES=6;
static const char *tags[] = { "1", "2", "3", "4", "5", "6" };

static const Rule rules[] = {
    /* class             instance    title    tags mask  iscentered   isfloating   monitor */
    { "st-256color",     NULL,       0,       0,         1,           0,           -1 },
    { "feh",             NULL,       0,       0,         1,           1,           -1 },
    { "mpv",             NULL,       0,       0,         1,           1,           -1 },
    { "mupdf",           NULL,       0,       0,         1,           1,           -1 },
};

#include "/home/mitch/.cache/wal/colors-wal-dwm.h"
/* static const char *colors[][3]      = { */
/*     /1*               fg         bg         border   *1/ */
/*     [SchemeNorm] = { "#bbbbbb", "#222222", "#555555" }, */
/*     [SchemeSel]  = { "#eeeeee", "#eeeeee", "#333333" }, */
/*     [SchemeUrg]  = { "#ff0000", "#ff0000", "#ff0000" }, */
/* }; */

static const float mfact      = 0.5;
static const int nmaster      = 1;
static const int resizehints  = 1;
static const int focusonwheel = 1;
static const int topbar  = 1;

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
static const char *scratchpadcmd[] = { "st", "-t", scratchpadname, "-g", "80x22", NULL };
 
static const char *term[] = { "st", NULL };
static const char *termtabbed[] = { "tabbed", "-d", "-c", "-r", "2", "st", "-w", "''", NULL };
static const char *menu[]   = { "menu", "run", "-p", "Run:", NULL };
static const char *ranger[] = { "st", "-e", "ranger", NULL };
static const char *brws[]   = { "brws",        NULL };
static const char *lck[]    = { "lck",         NULL };
static const char *scrap[]  = { "scrap",       NULL };
static const char *scrapn[] = { "scrap", "-n", NULL };
static const char *dedit[]  = { "dedit",       NULL };
static const char *tasks[]  = { "tasks",       NULL };
static const char *clip[]   = { "clip",        NULL };
static const char *keylayout[] = { "keys",     NULL };

static const char *volinc[]       = { "vol", "-inc", "8",      NULL };
static const char *voldec[]       = { "vol", "-dec", "8",      NULL };
static const char *mpvctoggle[]   = { "mpvc", "toggle",        NULL };
static const char *mpvcnext[]     = { "mpvc", "--track", "+1", NULL };
static const char *mpvcprev[]     = { "mpvc", "--track", "-1", NULL };
static const char *mpvcforward[]  = { "mpvc", "--seek", "+30", NULL };
static const char *mpvcbackward[] = { "mpvc", "--seek", "-30", NULL };

static Key keys[] = {
    /* modifier            key        function        argument */
    /* -*-*-*-*-*-*-*- programs -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
	{ Mod1Mask|ShiftMask,   XK_Return, spawn,          {.v = termtabbed } },
	{ Mod1Mask,             XK_Return, spawn,          {.v = term   } },
	{ Mod1Mask,             XK_r,      spawn,          {.v = ranger } },
	{ Mod1Mask,             XK_w,      spawn,          {.v = brws   } },
	{ Mod1Mask,             XK_x,      spawn,          {.v = lck    } },
	{ Mod1Mask,             XK_p,      spawn,          {.v = menu   } },
	{ Mod1Mask,             XK_o,      spawn,          {.v = dedit  } },
	{ Mod1Mask,             XK_i,      spawn,          {.v = tasks  } },
	{ Mod1Mask,             XK_c,      spawn,          {.v = clip   } },
	{ ControlMask,          XK_Print,  spawn,          {.v = scrap  } },
	{ NULL,                 XK_Print,  spawn,          {.v = scrapn } },
	{ Mod1Mask|ControlMask, XK_k,      spawn,          {.v = keylayout } },
    /* -*-*-*-*-*-*-*- media control -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
	{ Mod1Mask,             XK_apostrophe,   spawn,    {.v = volinc       } },
	{ Mod1Mask,             XK_semicolon,    spawn,    {.v = voldec       } },
	{ Mod1Mask,             XK_slash,        spawn,    {.v = mpvctoggle   } },
	{ Mod1Mask,             XK_period,       spawn,    {.v = mpvcnext     } },
	{ Mod1Mask,             XK_comma,        spawn,    {.v = mpvcprev     } },
	{ Mod1Mask,             XK_bracketright, spawn,    {.v = mpvcforward  } },
	{ Mod1Mask,             XK_bracketleft,  spawn,    {.v = mpvcbackward } },
    /* -*-*-*-*-*-*-*- dwm commands -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    { Mod1Mask,            XK_q,      killclient,     {0} },
    { Mod1Mask,            XK_j,      focusstack,     {.i = +1 } },
    { Mod1Mask,            XK_k,      focusstack,     {.i = -1 } },
    { Mod1Mask|ShiftMask,  XK_h,      setmfact,       {.f = -0.05} },
    { Mod1Mask|ShiftMask,  XK_l,      setmfact,       {.f = +0.05} },
    { Mod1Mask,            XK_t,      setlayout,      {.v = &layouts[0]} },
    { Mod1Mask,            XK_f,      setlayout,      {.v = &layouts[1]} },
    { Mod1Mask,            XK_m,      setlayout,      {.v = &layouts[2]} },
    { Mod1Mask,            XK_space,  togglescratch,  {.v = scratchpadcmd } },
	{ Mod1Mask,            XK_b,      togglebar,      {0} },
    { Mod1Mask,            XK_s,      togglesticky,   {0} },
    { Mod1Mask|ShiftMask,  XK_space,  togglefloating, {0} },
    { Mod1Mask,            XK_h,      rotatestack,    {.i = -1 } },
    { Mod1Mask,            XK_l,      rotatestack,    {.i = +1 } },
    { Mod1Mask,            XK_Tab,    view,           {0} },
    { Mod1Mask|ShiftMask|ControlMask, XK_q,      quit,           {0} },
    TAGKEYS(XK_1,0) TAGKEYS(XK_2,1) TAGKEYS(XK_3,2) TAGKEYS(XK_4,3) 
    TAGKEYS(XK_5,4) TAGKEYS(XK_6,5) TAGKEYS(XK_BackSpace,5)
};

static Button buttons[] = {
    { ClkClientWin,         Mod1Mask,       Button1,        movemouse,      {0} },
    { ClkClientWin,         Mod1Mask,       Button3,        resizemouse,    {0} },
    { ClkRootWin,           0,              Button3,        spawn,          SHCMD("x9term") },
};
