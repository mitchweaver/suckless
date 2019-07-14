#include "st-configh-ignores.h"

/* --------------- Fonts -------------------------------- */
char font[] = "Terminus:pixelsize=14:antialias=false:autohint=false";
/* char font[] = "Share Tech Mono:pixelsize=16:antialias=true:autohint=true"; */
/* char font[] = "GohuFont:pixelsize=14:antialias=false:autohint=false"; */
/* char font[] = "cherry:pixelsize=12:antialias=false:autohint=false"; */
/* char font[] = "Roboto Mono:pixelsize=16:antialias=true:autohint=true"; */
/* char font[] = "MonteCarlo:pixelsize=10:antialias=false:autohint=false"; */
/* ------------------------------------------------------- */

int borderpx = 22; /* Internal border */

/* --------------- Themes -------------------------------- */
#include "/home/mitch/.cache/wal/colors-wal-st.h" // pywal
/* #include "themes/light-orange-theme.h" */
/* #include "themes/terminaldotsexy/grayscale-theme.h" */
/* #include "themes/mine/bubblegum-theme.h" */
/* #include "themes/other/sourcerer-theme.h" */
/* #include "themes/terminaldotsexy/praiso-dark-theme.h" */
/* #include "themes/terminaldotsexy/estuary-light-theme.h" */
/* #include "themes/terminaldotsexy/gruvbox-light-hard-theme.h" */
/* #include "themes/terminaldotsexy/plateau-light-theme.h" */
/* #include "themes/terminaldotsexy/ashes-dark-theme.h" */
/* #include "themes/terminaldotsexy/mocha-light-theme.h" */
/* #include "themes/terminaldotsexy/ashes-light-theme.h" */
/* #include "themes/terminaldotsexy/unsifted-wheat-theme.h" */
/* #include "themes/terminaldotsexy/insignificato-theme.h" */
/* #include "themes/terminaldotsexy/mostly-bright-theme.h" */
/* #include "themes/terminaldotsexy/dawn-theme.h" */
/* #include "themes/terminaldotsexy/nature-suede-theme.h" */
/* #include "themes/terminaldotsexy/mocha-dark-theme.h" */
/* #include "themes/terminaldotsexy/navy-and-ivory-theme.h" */
/* #include "themes/terminaldotsexy/visibone-theme.h" */
/* #include "themes/terminaldotsexy/ocean-dark-theme.h" */
/* #include "themes/terminaldotsexy/tomorrow-dark-theme.h" */
/* #include "themes/terminaldotsexy/default-color-scheme.h" */
/*  -------------------------------------------------------  */

unsigned int cols = 80;
unsigned int rows = 30;
static char shell[] = "/bin/sh";

static const unsigned int tabspaces = 4;

float cwscale = 1.0; // kerning
float chscale = 1.0; // bounding box

unsigned int xfps = 120;
unsigned int actionfps = 30;

/* void plumb() { system("/bin/sh ${HOME}/usr/bin/plumb 2>> /tmp/plumb_errors.txt &"); } */
MouseKey mkeys[] = {
	/* button               mask            function        argument */
	{ Button4,              XK_NO_MOD,      kscrollup,      {.i =  1} },
	{ Button5,              XK_NO_MOD,      kscrolldown,    {.i =  1} },
    { Button4,              ControlMask,    zoom,           {.f =  +2} },
	{ Button5,              ControlMask,    zoom,           {.f =  -2} },
    /* ------------------------------------------------------- */ 
    /* { Button3,              XK_NO_MOD,      plumb,          { 0 } }, */
    /* { Button2,              XK_NO_MOD,      execute,        { 0 } }, */
    /* ------------------------------------------------------- */ 
};

Shortcut shortcuts[] = {
	/* mask                 keysym          function        argument */
	{ ControlMask,          XK_equal,       zoom,           {.f = +2} },
	{ ControlMask,          XK_minus,       zoom,           {.f = -2} },
	{ ControlMask,          XK_BackSpace,   zoomreset,      {.f =  0} },

    { ShiftMask,            XK_Insert,      clippaste,      {.i =  0} },
    { ControlMask,          XK_v,           clippaste,      {.i =  0} },

	{ ControlMask,          XK_Page_Up,     kscrollup,      {.i = 3} },
	{ ControlMask,          XK_Page_Down,   kscrolldown,    {.i = 3} },

    // note copyurl has been edited to also open with xdg-open, look in st.c
	{ ControlMask,          XK_l,           copyurl,        {.i =  0} },
};
