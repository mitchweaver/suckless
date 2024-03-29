static const char *fonts[] = {
    /* "spleen:pixelsize=22:antialias=true:autohint=true", */
    "ShureTechMono Nerd Font:pixelsize=22:antialias=true:autohint=true",
    "Terminus:pixelsize=18:antialias=false:autohint=false",
    "Hack:pixelsize=20:antialias=true:autohint=true",
    "Noto Color Emoji:pixelsize=22:antialias=true:autohint=true",
    "Unifont:pixelsize=22"
};
    /* "cozette:size=10:antialias=false:autohint=false", */
    /* "Terminus:pixelsize=18:antialias=false:autohint=false" */
    /* "Hack:pixelsize=20:antialias=true:autohint=true", */
    /* "Noto Sans CJK JP:pixelsize=22:antialias=true:autohint=true", */
    /* "Noto Sans CJK HK:pixelsize=22:antialias=true:autohint=true", */
    /* "Noto Sans CJK KR:pixelsize=22:antialias=true:autohint=true", */
    /* "Noto Sans CJK SC:pixelsize=22:antialias=true:autohint=true", */
    /* "Noto Sans CJK TC:pixelsize=22:antialias=true:autohint=true", */

/* static double opacity = 0.8; // opacity patch */

/* static const unsigned int baralpha = 0xd0; */
/* static const unsigned int borderalpha = OPAQUE; */
/* static const unsigned int alphas[][3]      = { */
/* 	/1*               fg      bg        border     *1/ */
/* 	[SchemeNorm] = { OPAQUE, baralpha, borderalpha }, */
/* 	[SchemeSel]  = { OPAQUE, baralpha, borderalpha }, */
/* }; */

static const unsigned int border_width = 2;
static int fuzzy = 1;

// theme, included from ${HOME}/.cache/themes in config.mk
#include <dmenu.h>

static const char *prompt = NULL;  /* -p  option; prompt to the left of input field */
static const char worddelimiters[] = " /?\"&[].:,";
static int topbar = 1;
static unsigned int lines = 0;
