static const char *fonts[] = { "cozette:size=10:antialias=false:autohint=false" };
/* static const char *fonts[] = { "Terminus:size=12:antialias=false:autohint=false" }; */
/* static const char *fonts[] = { "ShureTechMono Nerd Font:size=16:antialias=true:autohint=true" }; */

static const unsigned int border_width = 2;
static int fuzzy = 1;

#include "/home/mitch/.cache/wal/colors-wal-dmenu.h";
/* static const char *colors[SchemeLast][2] = { */
/* /1*     fg         bg       *1/ */
/* [SchemeNorm] = { "#bbbbbb", "#373B41" }, */
/* [SchemeSel] =  { "#eeeeee", "#5F819D" }, */
/* [SchemeOut] =  { "#000000", "#00ffff" }, */
/* }; */

static const char *prompt = NULL;  /* -p  option; prompt to the left of input field */
static const char worddelimiters[] = " /?\"&[].:,";
static int topbar = 1;
static unsigned int lines = 0;
