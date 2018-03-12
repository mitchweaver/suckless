#include "st-configh-ignores.h"

/* --------------- Font ---------------------------------- */
/* char font[] = "Terminus:pixelsize=14:antialias=false:autohint=false"; */
/* char font[] = "Terminus:pixelsize=12:antialias=false:autohint=false"; */
/* char font[] = "GohuFont:pixelsize=12:antialias=false:autohint=false"; */
/* char font[] = "scientifica:pixelsize=10:antialias=false:autohint=false"; */
/* char font[] = "MonteCarlo:pixelsize=10:antialias=false:autohint=false"; */
/* char font[] = "cherry:pixelsize=10:antialias=false:autohint=false"; */
/* char font[] = "orp:pixelsize=10:antialias=false:autohint=false"; */
char font[] = "Roboto Mono:pixelsize=12:antialias=true:autohint=true";
/* ------------------------------------------------------- */

/* ---------------- Border ------------------------------- */
/* int borderpx = 14; /1* Internal border *1/ */
int borderpx = 28; /* Internal border */
/* ------------------------------------------------------- */

/* --------------- Themes -------------------------------- */
#include "/home/mitch/.cache/wal/colors-wal-st.h" // pywal
/* #include "themes/estuary-light-theme.h" */
/* #include "themes/gruvbox-light-hard-theme.h" */
/* #include "themes/plateau-light-theme.h" */
/* #include "themes/ashes-dark-theme.h" */
/* #include "themes/mocha-light-theme.h" */
/* #include "themes/ashes-light-theme.h" */
/* #include "themes/unsifted-wheat-theme.h" */
/* #include "themes/insignificato-theme.h" */
/* #include "themes/mostly-bright-theme.h" */
/* #include "themes/dawn-theme.h" */
/* #include "themes/grayscale-theme.h" */
/* #include "themes/praiso-dark-theme.h" */
/* #include "themes/nature-suede-theme.h" */
/* #include "themes/mocha-dark-theme.h" */
/* #include "themes/navy-and-ivory-theme.h" */
/* #include "themes/visibone-theme.h" */
/* #include "themes/ocean-dark-theme.h" */
/* #include "themes/tomorrow-dark-theme.h" */
/* #include "themes/default-color-scheme.h" */
/*  -------------------------------------------------------  */

unsigned int cols = 80;
unsigned int rows = 30;
/* static char shell[] = "/bin/mksh"; */
static char shell[] = "/bin/ksh";

static const unsigned int tabspaces = 4;

float cwscale = 1.0; // kerning
float chscale = 1.0; // bounding box

/* frames per second st should at maximum draw to the screen */
unsigned int xfps = 300; // higher number here helps with ranger previews
unsigned int actionfps = 30;

// this is using my own patch as the official one doesn't work on BSD... and
// just isn't good.
void plumb()   { system("/bin/dash ${HOME}/usr/bin/plumb 2>> /tmp/plumb_errors.txt &"); }
/* void execute() { system("/bin/dash ${HOME}/bin/execute.sh 2>> /tmp/execute_errors.txt &"); } */

// this has since been moved to dwm's binds
/* void history() { system("${SHELL} ${HOME}/bin/dhistory.sh"); } */

MouseKey mkeys[] = {
	/* button               mask            function        argument */
	{ Button4,              XK_NO_MOD,      kscrollup,      {.i =  1} },
	{ Button5,              XK_NO_MOD,      kscrolldown,    {.i =  1} },
    { Button4,              ControlMask,    zoom,           {.f =  +2} },
	{ Button5,              ControlMask,    zoom,           {.f =  -2} },
    /* ------------------------------------------------------- */ 
    { Button3,              XK_NO_MOD,      plumb,          { 0 } },
    /* { Button2,              XK_NO_MOD,      execute,        { 0 } }, */
    /* ------------------------------------------------------- */ 
};

Shortcut shortcuts[] = {
	/* mask                 keysym          function        argument */
	{ ControlMask,          XK_equal,       zoom,           {.f = +2} },
	{ ControlMask,          XK_minus,       zoom,           {.f = -2} },
	{ ControlMask,          XK_BackSpace,   zoomreset,      {.f =  0} },
    { ShiftMask,            XK_Insert,      clippaste,      {.i =  0} },
    { Mod4Mask,             XK_v,           clippaste,      {.i =  0} },

	{ ControlMask,          XK_Page_Up,     kscrollup,      {.i = 3} },
	{ ControlMask,          XK_Page_Down,   kscrolldown,    {.i = 3} },

	{ Mod4Mask,             XK_k,           kscrollup,      {.i = 3} },
	{ Mod4Mask,             XK_j,           kscrolldown,    {.i = 3} },

    /* -------------- Custom Funcs ---------------------------------------------- */
    /* { Mod4Mask,              XK_h,           history,        {.i = 0} }, */
    // note copyurl has been edited to also open with xdg-open, look in st.c
	{ Mod4Mask,              XK_l,           copyurl,        {.i =  0} },
    /* -------------------------------------------------------------------------- */
	/* { ControlMask,              XK_I,           iso14755,       {.i =  0} }, */ 
    /* { XK_ANY_MOD,           XK_Break,       sendbreak,      {.i =  0} }, */
};
