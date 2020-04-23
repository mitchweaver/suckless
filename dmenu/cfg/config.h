static const char *fonts[] = { "ShureTechMono Nerd Font:pixelsize=22:antialias=true:autohint=true" };
/* static const char *fonts[] = { "cozette:size=10:antialias=false:autohint=false" }; */
/* static const char *fonts[] = { "Terminus:pixelsize=18:antialias=false:autohint=false" }; */

static const unsigned int border_width = 2;
static int fuzzy = 1;

// theme, included from ${HOME}/.cache/themes in config.mk
#include <dmenu.h>

static const char *prompt = NULL;  /* -p  option; prompt to the left of input field */
static const char worddelimiters[] = " /?\"&[].:,";
static int topbar = 1;
static unsigned int lines = 0;
