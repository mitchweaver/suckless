#define HOMEPAGE "https://start.duckduckgo.com/html"
#define BKMS "/home/mitch/files/bkm"
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
    { "dict",   "https://thefreedictionary.com/%s" },
    { "thes",   "https://thesaurus.com/browse/%s" },
    { "urban",  "https://urbandictionary.com/define.php?term=%s" },
};

static const char *fulluseragent  = "Mozilla/5.0";
static const char *scriptfile     = "~/.surf/script.js";
static const char *styledir       = "~/.surf/styles/";
static const char *cachedir       = "~/.surf/cache/";
static const char *cookiefile     = "~/.surf/cookies.txt";
static const int surfuseragent    = 0;

static Parameter defconfig[ParameterLast] = {
    SETB(StrictSSL,          1), // HTTPS Everywhere essentially
    SETB(JavaScript,         1), // NoScript -- whitelist or blacklist below
    SETB(Geolocation,        0), // NOPE!
    SETB(SiteQuirks,         1),
    SETV(CookiePolicies,     "@Aa"),
    SETB(Plugins,            1),
    SETB(LoadImages,         1),
    SETB(ScrollBars,         1),
    SETB(AcceleratedCanvas,  1),
    SETB(DiskCache,          1),
    SETI(FontSize,           12),
    SETB(DNSPrefetch,        0),
    SETB(CaretBrowsing,      0),
    SETB(FrameFlattening,    0),
    SETB(HideBackground,     0),
    SETB(Inspector,          0),
    SETB(KioskMode,          0),
    SETB(MediaManualPlay,    0),
    SETV(PreferredLanguages, ((char *[]){ NULL })),
    SETB(RunInFullscreen,    0),
    SETB(ShowIndicators,     1),
    SETB(SpellChecking,      1),
    SETV(SpellLanguages,     ((char *[]){ "en_US,de_DE", NULL })),
    SETB(Style,              1),
    SETF(ZoomLevel,          1.0),
};

static UriParameters uriparams[] = {
    { "(://|\\.)suckless\\.org(/|$)", {
      FSETB(JavaScript, 0),
      FSETB(Plugins,    0),
    }, },
};

static SiteStyle styles[] = {
    /* regexp               file in $styledir */
    { ".*",                 "default.css" },
};

static WebKitFindOptions findopts = WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE |
                                    WEBKIT_FIND_OPTIONS_WRAP_AROUND;

// bring up my bookmarks file in the url bar,
// while preserving the current page link at the top
#define SETPROP(p, q) { \
    .v = (const char *[]){ "/bin/sh", "-c", \
        "prop=\"`printf '%s\n' $(xprop -id $2 $0 " \
        "| sed -e 's|_SURF.*.= \"||' -e 's|\"$||' " \
        "&& cat $3) | menu -w $2 -p 'Go:'`\" &&" \
        "xprop -id $2 -f $1 8s -set $1 \"$prop\"", \
        p, q, winid, BKMS, NULL \
    } \
}

#define DOWNLOAD(d, r) { \
    .v = (const char *[]){ "/bin/sh", "-c", \
        "st -e /bin/sh -c \"cd ${HOME}/Downloads " \
        "&& curl -g -L -J -O --user-agent '$1'" \
        " --referer '$2' -b $3 -c $3 '$0';" \
        " sleep 5;\"", \
        d, useragent, r, cookiefile, NULL \
    } \
}

#define PLUMB(u) {\
    .v = (const char *[]){ "/bin/sh", "-c", \
            "${HOME}/bin/util/opn \"$0\"", u, NULL } \
}

// play current page w/ mpv+ytdl
#define MPV_URL {\
    .v = (const char *[]){ "/bin/sh", "-c", \
        "mpv --really-quiet \$(xprop -id $0 _SURF_URI | cut -d '\"' -f 2)", \
        winid, NULL } \
}

// go to homepage with a hotkey, ex: mod+spacebar
#define GO_HOME {\
    .v = (const char *[]){ "/bin/sh", "-c", \
        "xprop -id $0 -f _SURF_GO 8s -set _SURF_GO $1", \
        winid, HOMEPAGE, NULL } \
}

// add current page to my bookmarks
#define BM_ADD {\
    .v = (const char *[]){ "/bin/sh", "-c", \
        "xprop -id $0 _SURF_URI " \
        "| cut -d '\"' -f 2 " \
        "| sed -E -e 's|http(s)?://(www\.)?||' -e 's|/$||' >>$1" \
        "&& sort -u $1 -o $1", \
        winid, BKMS, NULL } \
}

// http://git.gnome.org/browse/gtk+/plain/gdk/gdkkeysyms.h
#define MODKEY GDK_CONTROL_MASK
#define SHIFT GDK_SHIFT_MASK
static Key keys[] = {
    /* modifier              keyval          function    arg */
    { MODKEY,                GDK_KEY_g,      spawn,      SETPROP("_SURF_URI", "_SURF_GO") },

    /* -*-*-*-*-*-*-*-*-*-*- CUSTOM FUNCS *-*-*-*-*-*-*-*-*-*-*-*-* */
	{ MODKEY|SHIFT,            GDK_KEY_b,      spawn,      BM_ADD   },
    { MODKEY,                  GDK_KEY_y,      spawn,      MPV_URL  },
    { MODKEY,                  GDK_KEY_space,  spawn,      GO_HOME  },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

    /* ------------------------ FINDING ------------------------------ */
    { MODKEY,   GDK_KEY_slash,   spawn,  SETPROP("_SURF_FIND", "_SURF_FIND") },
    { MODKEY,   GDK_KEY_period,  find,   { .i = +1 } },
    { MODKEY,   GDK_KEY_comma,   find,   { .i = -1 } },
    /* --------------------------------------------------------------- */

    /* ---------------------- HISTORY -------------------------------- */
    { MODKEY,               GDK_KEY_o,   navigate,   { .i = +1 } },
    { MODKEY,               GDK_KEY_i,   navigate,   { .i = -1 } },
    /* --------------------------------------------------------------- */

    /* -*-*-*-*-*-*-*-* RELOADING -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    { MODKEY,GDK_KEY_F5,       reload,     { .b = 0 } }, // reload page
    { MODKEY,GDK_KEY_r,        reload,     { .b = 0 } }, // reload page
    { MODKEY|SHIFT, GDK_KEY_r, reload,     { .b = 1 } }, // "hard" reload
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

    /* -*-*-*-*-*-*-*-*-*-*- GENERAL -*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    { 0,                     GDK_KEY_Escape, stop,       { 0 } },
    { MODKEY,                GDK_KEY_c,      stop,       { 0 } },
    { MODKEY,                GDK_KEY_c,      clipboard,  { .b = 0 } },
    { 0,                     GDK_KEY_F11,    togglefullscreen, { 0 } },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

    /* -*-*-*-*-*-*-*-*- VIM MODE SCROLLING -*-*-*-*-*-*-*-*-*-*-*-* */
    { MODKEY,                GDK_KEY_j,      scroll,     { .i = 'd' } },
    { MODKEY,                GDK_KEY_k,      scroll,     { .i = 'u' } },
    { MODKEY,                GDK_KEY_f,      scroll,     { .i = 'U' } },
    { MODKEY,                GDK_KEY_b,      scroll,     { .i = 'D' } },
    { MODKEY|SHIFT,          GDK_KEY_j,      scroll,     { .i = 'r' } },
    { MODKEY|SHIFT,          GDK_KEY_k,      scroll,     { .i = 'l' } },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

    /* -*-*-*-*-*-*-*-*- ZOOMING -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    { MODKEY, GDK_KEY_minus,      zoom,       { .i = -1 } },
    { MODKEY, GDK_KEY_equal,      zoom,       { .i = +1 } },
    { MODKEY, GDK_KEY_plus,       zoom,       { .i = +1 } },
    { MODKEY, GDK_KEY_BackSpace,  zoom,       { .i = +0 } },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
};

/* target can be OnDoc, OnLink, OnImg, OnMedia, OnEdit, OnBar, OnSel, OnAny */
static Button buttons[] = {
    /* target       event mask      button  function        argument        stop event */
    { OnLink,       0,              2,      clicknewwindow, { .b = 0 },     1 },
    { OnLink,       MODKEY,         2,      clicknewwindow, { .b = 1 },     1 },
    { OnLink,       MODKEY,         1,      clicknewwindow, { .b = 1 },     1 },
    { OnAny,        0,              8,      clicknavigate,  { .i = -1 },    1 },
    { OnAny,        0,              9,      clicknavigate,  { .i = +1 },    1 },
};
