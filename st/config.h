#include "st-configh-ignores.h"

/* --------------- Font ---------------------------------- */
/* char font[] = "Terminus:pixelsize=14:antialias=false:autohint=false"; */
char font[] = "Terminus:pixelsize=12:antialias=false:autohint=false";
/* char font[] = "scientifica:pixelsize=14:antialias=false:autohint=false"; */
/* char font[] = "MonteCarlo:pixelsize=14:antialias=false:autohint=false"; */
/* char font[] = "Roboto Mono:pixelsize=14:antialias=true:autohint=true"; */
/* char font[] = "Roboto Mono:pixelsize=12:antialias=true:autohint=true"; */
/* ------------------------------------------------------- */

/* ---------------- Border ------------------------------- */
/* int borderpx = 10; /1* Internal border *1/ */
/* int borderpx = 12; /1* Internal border *1/ */
/* int borderpx = 14; /1* Internal border *1/ */
int borderpx = 28; /* Internal border */
/* int borderpx = 35; /1* Internal border *1/ */
/* ------------------------------------------------------- */

/* --------------- Themes -------------------------------- */
#include "/home/mitch/.cache/wal/colors-wal-st.h" // pywal
/* #include "ashes-dark-theme.h" */
/* #include "unsifted-wheat-theme.h" */
/* #include "mocha-light-theme.h" */
/* #include "ashes-light-theme.h" */
/* #include "mocha-dark-theme.h" */
/* #include "navy-and-ivory-theme.h" */
/* #include "visibone-theme.h" */
/* #include "ocean-dark-theme.h" */
/* #include "tomorrow-dark-theme.h" */
/* #include "default-color-scheme.h" */
/*  -------------------------------------------------------  */

unsigned int cols = 80;
unsigned int rows = 30;
/* static char shell[] = "/bin/mksh"; */
static char shell[] = "/bin/ksh";

/* Kerning / character bounding-box multipliers */
float cwscale = 1.0;
float chscale = 1.0;
static const unsigned int tabspaces = 4;

/* frames per second st should at maximum draw to the screen */
unsigned int xfps = 300; // higher number here seems to help with ranger previews
unsigned int actionfps = 30;

/* Default colour and shape of the mouse cursor */
unsigned int mousefg = 7;
unsigned int mousebg = 0;
unsigned int defaultrcs = 257;

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
