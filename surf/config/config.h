#define HOMEPAGE "file:///home/mitch/src/startpage/index.html"
const static SearchEngine searchengines[] = {
    { "",       "https://duckduckgo.com/?q=%s" },
    { "g",      "https://google.com/search?q=%s"   },
    { "image",  "https://duckduckgo.com/?q=!image %s" },
    { "git",    "https://github.com/search?utf8=&q=%s&type=" },
    { "ugit",   "https://github.com/%s" },
    { "gist",   "https://gist.github.com/search?q=%s" },
    { "wiki",   "https://en.wikipedia.org/wiki/%s" },
    { "w",      "https://en.wikipedia.org/wiki/%s" },
    { "metal",  "https://metal-archives.com/search?searchString=%s&type=band_name" },
    { "ebay",   "https://www.ebay.com/sch/i.html?_from=R40&_nkw=%s&_sacat=0&_sop=15&rt=nc&LH_BIN=1" },
    { "discogs","https://discogs.com/search?q=%s&btn=&type=all" },
    { "bc",     "https://bandcamp.com/search?q=%s" },
    { "arch",   "https://wiki.archlinux.org/index.php?search=%s" },
    { "gentoo", "https://wiki.gentoo.org/index.php?search=%s" },
    { "r",      "https://old.reddit.com/r/%s" },
    { "u",      "https://old.reddit.com/u/%s" },
    { "4",      "https://boards.4chan.org/%s" },
    { "yt",     "https://youtube.com/results?search_query=%s" },
    { "reddit", "https://old.reddit.com/search?q=%s" },
    { "pirate", "https://thepiratebay.org/search/%s" },
    { "think",  "https://thinkwiki.org/w/index.php?search=%s" },
    { "dict",   "https://thefreedictionary.com/%s" },
    { "thes",   "https://thesaurus.com/browse/%s" },
    { "urban",  "https://urbandictionary.com/define.php?term=%s" },
};

//static char *fulluseragent  = "Mozilla/5.0 (X11; Linux x86_64; rv:10.0) Firefox/10.0";
static char *fulluseragent  = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.157 Safari/537.36";

// --------------------------------------------------------------------------------

static int surfuseragent    = 0;  /* Append Surf version to default WebKit user agent */
static char *styledir       = "~/.surf/styles/";
static char *certdir        = "~/.surf/certificates/";
static char *cachedir       = "~/.surf/cache/";
static char *cookiefile     = "~/.surf/cookies.txt";
static char *historyfile    = "~/.surf/history.txt";
static char *scriptfiles[]  = {
    "~/src/suckless/surf/scripts_official/link_hints.js",
//    "~/src/suckless/surf/scripts_mine/.js",
//	"~/.surf/script.js",
};

/* Webkit default features */
/* Highest priority value will be used.
 * Default parameters are priority 0
 * Per-uri parameters are priority 1
 * Command parameters are priority 2 */
static Parameter defconfig[ParameterLast] = {
	/* parameter                    Arg value       priority */
	[AcceleratedCanvas]   =       { { .i = 1 },     },
	[StrictTLS]           =       { { .i = 0 },     },
	[AccessMicrophone]    =       { { .i = 0 },     },
	[AccessWebcam]        =       { { .i = 0 },     },
	[JavaScript]          =       { { .i = 1 },     },
	[LoadImages]          =       { { .i = 1 },     },
	[DNSPrefetch]         =       { { .i = 0 },     },
	[MediaManualPlay]     =       { { .i = 0 },     },
	[Certificate]         =       { { .i = 0 },     },
	[CaretBrowsing]       =       { { .i = 0 },     },
	[CookiePolicies]      =       { { .v = "@Aa" }, },
	[DefaultCharset]      =       { { .v = "UTF-8" }, },
	[DiskCache]           =       { { .i = 1 },     },
	[FileURLsCrossAccess] =       { { .i = 0 },     },
	[FontSize]            =       { { .i = 12 },    },
	[FrameFlattening]     =       { { .i = 0 },     },
	[Geolocation]         =       { { .i = 0 },     },
	[HideBackground]      =       { { .i = 0 },     },
	[Inspector]           =       { { .i = 0 },     },
	[Java]                =       { { .i = 0 },     },
	[KioskMode]           =       { { .i = 0 },     },
	[Plugins]             =       { { .i = 0 },     },
	[PreferredLanguages]  =       { { .v = (char *[]){ NULL } }, },
	[RunInFullscreen]     =       { { .i = 0 },     },
	[ScrollBars]          =       { { .i = 0 },     },
	[ShowIndicators]      =       { { .i = 0 },     },
	[SiteQuirks]          =       { { .i = 1 },     },
	[SmoothScrolling]     =       { { .i = 1 },     },
	[SpellChecking]       =       { { .i = 0 },     },
	[SpellLanguages]      =       { { .v = ((char *[]){ "en_US", NULL }) }, },
	[Style]               =       { { .i = 1 },     },
	[ZoomLevel]           =       { { .f = 1.0 },   },
};

static UriParameters uriparams[] = {
	{ "(://|\\.)suckless\\.org(/|$)", {
	  [JavaScript] = { { .i = 0 }, 1 },
	  [Plugins]    = { { .i = 0 }, 1 },
	}, },
};

/* default window size: width, height */
static int winsize[] = { 1000, 700 };

static WebKitFindOptions findopts = WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE |
                                    WEBKIT_FIND_OPTIONS_WRAP_AROUND;

#define PROMPT_GO   "Go:"
#define PROMPT_FIND "Find:"

#define SETPROP(r, s, p) { \
    .v = (const char *[]){ "/bin/sh", "-c", \
        "[ \"$(pgrep dmenu)\" ] && pkill -9 dmenu ; " \
        ". ${HOME}/.cache/wal/colors.sh ;" \
        "prop=\"$(printf '%b' \"$(xprop -id $1 $2 " \
        "| sed \"s/^$2(STRING) = //;s/^\\\"\\(.*\\)\\\"$/\\1/\" && cat ${HOME}/var/files/bkm/bookmarks.txt)\" " \
        "| dmenu -i -nb $color0 -nf $color15 -sb $color2 -sf $color15 -l 10 -p \"$4\" -w $1)\" && " \
        "xprop -id $1 -f $3 8s -set $3 \"$prop\"", \
        "surf-setprop", winid, r, s, p, NULL \
    } \
}

// same as above but don't cat my bookmarks
#define SETFIND(r, s, p) { \
    .v = (const char *[]){ "/bin/sh", "-c", \
        "[ \"$(pgrep dmenu)\" ] && pkill -9 dmenu ; " \
        ". ${HOME}/.cache/wal/colors.sh ;" \
        "prop=\"$(printf '%b' \"$(xprop -id $1 $2 " \
        "| sed \"s/^$2(STRING) = //;s/^\\\"\\(.*\\)\\\"$/\\1/\")\" " \
        "| dmenu -i -nb $color0 -nf $color15 -sb $color2 -sf $color15 -l 10 -p \"$4\" -w $1)\" && " \
        "xprop -id $1 -f $3 8s -set $3 \"$prop\"", \
        "surf-setprop", winid, r, s, p, NULL \
    } \
}

/* DOWNLOAD(URI, referer) */
#define DOWNLOAD(u, r) { \
        .v = (const char *[]){ "st", "-T", "surf-download", "-n", "surf-download", "-e", "/bin/sh", "-c",\
             "cd ${HOME}/var/downloads && curl -g -L -J -O -A \"$1\" -b \"$2\" -c \"$2\"" \
             " -e \"$3\" \"$4\"; read", \
             "surf-download", useragent, cookiefile, r, u, NULL \
        } \
}

/* PLUMB(URI) */
/* This called when some URI which does not begin with "about:",
 * "http://" or "https://" should be opened.
 */
#define PLUMB(u) {\
        .v = (const char *[]){ "/bin/sh", "-c", \
             "${HOME}/bin/utils/open \"$0\"", u, NULL \
        } \
}

/* VIDEOPLAY(URI) */
#define VIDEOPLAY(u) {\
        .v = (const char *[]){ "/bin/sh", "-c", \
             "pkill -9 mpv ; mpv --really-quiet $(xprop -id $0 _SURF_URI | cut -d \\\" -f 2)", u, NULL \
        } \
}

#define MPV_URL { .v = (char *[]){ "/bin/sh", "-c", \
    "mpv --really-quiet \$(xprop -id $0 _SURF_URI | cut -d '\"' -f 2)", \
    winid, NULL } }


#define BM_ADD { .v = (char *[]){ "/bin/sh", "-c", \
    "BMKS=\${HOME}/var/files/bkm/bookmarks.txt ; \
    (echo `xprop -id $0 _SURF_URI | cut -d '\"' -f 2` && \
    cat \${BMKS}) | sort -u > \${BMKS}_new && \
    mv \${BMKS}_new \${BMKS}", \
    winid, NULL } }

// go to homepage with a hotkey, ex: mod+spacebar
#define GO_HOME { .v = (char *[]){ "/bin/sh", "-c", \
    "xprop -id $0 -f _SURF_GO 8s -set _SURF_GO \
    \${HOME}/src/startpage/index.html || exit 0", \
    winid, NULL } }

// grab bookmark from my bookmark script
#define BKM { .v = (char *[]){ "/bin/sh", "-c", \
    "\${HOME}/src/suckless/surf/scripts_mine/bkm.sh $0", \
    winid, NULL } }

// store a bookmark to $bkm/unsorted
#define BKM_ADD { .v = (char *[]){ "/bin/sh", "-c", \
    "echo `xprop -id $0 _SURF_URI | cut -d '\"' -f 2` \
    > \${HOME}/var/files/bkm/unsorted/\$(date)", \
    winid, NULL } }

/* styles */
/* The iteration will stop at the first match, beginning at the beginning of
 * the list.  */
static SiteSpecific styles[] = {
	/* regexp               file in $styledir */
	{ ".*",                 "default.css" },
};

/* certificates */ /*
 * Provide custom certificate for urls */
static SiteSpecific certs[] = {
	/* regexp               file in $certdir */
	{ "://suckless\\.org/", "suckless.org.crt" },
};

// http://git.gnome.org/browse/gtk+/plain/gdk/gdkkeysyms.h
#define MODKEY GDK_CONTROL_MASK
#define SHIFT GDK_SHIFT_MASK
static Key keys[] = {
	/* modifier              keyval          function    arg */
	{ MODKEY,                GDK_KEY_g,      spawn,      SETPROP("_SURF_URI", "_SURF_GO", PROMPT_GO) },
	{ MODKEY,                GDK_KEY_slash,  spawn,      SETFIND("_SURF_FIND", "_SURF_FIND", PROMPT_FIND) },

    /* ------------------------ FINDING ------------------------------ */
    { MODKEY,               GDK_KEY_period,  find,       { .i = +1 } },
    { MODKEY,               GDK_KEY_comma,   find,       { .i = -1 } },
    /* --------------------------------------------------------------- */

    /* ----------------- Custom Functions ---------------------------- */
	{ MODKEY|SHIFT,          GDK_KEY_b,      spawn,      BM_ADD   },
    { MODKEY,                GDK_KEY_e,      spawn,      BKM      },
    { MODKEY|SHIFT,          GDK_KEY_e,      spawn,      BKM_ADD  },
    { MODKEY,                GDK_KEY_space,  spawn,      GO_HOME  },
    { MODKEY,                GDK_KEY_y,      spawn,      MPV_URL },
    /* ----------------- End Custom Functions ------------------------ */

    /* ---------------------- History -------------------------------- */
    { MODKEY,               GDK_KEY_o,   navigate,   { .i = +1 } },
    { MODKEY,               GDK_KEY_i,   navigate,   { .i = -1 } },
    /* --------------------------------------------------------------- */

    /* -------------------- Zooming -------------------------------------- */
    { MODKEY,                GDK_KEY_minus,       zoom,       { .i = -1 } },
    { MODKEY,                GDK_KEY_plus,        zoom,       { .i = +1 } },
    { MODKEY,                GDK_KEY_equal,       zoom,       { .i = +1 } },
    { MODKEY,                GDK_KEY_BackSpace,   zoom,       { .i = +0 } },
    { MODKEY|SHIFT,          GDK_KEY_j,           zoom,       { .i = -1 } },
    { MODKEY|SHIFT,          GDK_KEY_k,           zoom,       { .i = +1 } },
    /* ------------------------------------------------------------------- */

	{ 0,                     GDK_KEY_Escape, stop,       { 0 } },
	{ MODKEY,                GDK_KEY_c,      stop,       { 0 } },

	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_r,      reload,     { .i = 1 } },
	{ MODKEY,                GDK_KEY_r,      reload,     { .i = 0 } },

	/* Currently we have to use scrolling steps that WebKit2GTK+ gives us
	 * d: step down, u: step up, r: step right, l:step left
	 * D: page down, U: page up */
    /* ---------------------- vim mode ----------------------------------- */
	/* vertical and horizontal scrolling, in viewport percentage */
	{ MODKEY,                GDK_KEY_j,      scrollv,    { .i = +10 } },
	{ MODKEY,                GDK_KEY_k,      scrollv,    { .i = -10 } },
	{ MODKEY,                GDK_KEY_space,  scrollv,    { .i = +50 } },
	{ MODKEY,                GDK_KEY_b,      scrollv,    { .i = -50 } },
	{ MODKEY,                GDK_KEY_i,      scrollh,    { .i = +10 } },
	{ MODKEY,                GDK_KEY_u,      scrollh,    { .i = -10 } },
    /* ------------------------------------------------------------------- */

    // -------------------- COPY PASTE ---------------------------- //
	//{ MODKEY,                GDK_KEY_v,      clipboard,  { .i = 1 } },
	{ MODKEY,                GDK_KEY_c,      clipboard,  { .i = 0 } },
    // ------------------------------------------------------------ //

	{ MODKEY,                GDK_KEY_p,      print,      { 0 } },
	{ 0,                     GDK_KEY_F11,    togglefullscreen, { 0 } },

//	{ MODKEY,                GDK_KEY_t,      showcert,   { 0 } },
//	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_a,      togglecookiepolicy, { 0 } },
//	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_o,      toggleinspector, { 0 } },
//	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_c,      toggle,     { .i = CaretBrowsing } },
//	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_f,      toggle,     { .i = FrameFlattening } },
//	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_g,      toggle,     { .i = Geolocation } },
//	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_s,      toggle,     { .i = JavaScript } },
//	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_i,      toggle,     { .i = LoadImages } },
//	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_v,      toggle,     { .i = Plugins } },
//	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_b,      toggle,     { .i = ScrollBars } },
//	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_t,      toggle,     { .i = StrictTLS } },
//	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_m,      toggle,     { .i = Style } },
};

/* button definitions */
/* target can be OnDoc, OnLink, OnImg, OnMedia, OnEdit, OnBar, OnSel, OnAny */
static Button buttons[] = {
	/* target       event mask      button  function        argument        stop event */
	{ OnLink,       0,              2,      clicknewwindow, { .i = 0 },     1 },
	{ OnLink,       MODKEY,         2,      clicknewwindow, { .i = 1 },     1 },
	{ OnLink,       MODKEY,         1,      clicknewwindow, { .i = 1 },     1 },
	{ OnAny,        0,              8,      clicknavigate,  { .i = -1 },    1 },
	{ OnAny,        0,              9,      clicknavigate,  { .i = +1 },    1 },
	{ OnMedia,      MODKEY,         1,      clickexternplayer, { 0 },       1 },
};
