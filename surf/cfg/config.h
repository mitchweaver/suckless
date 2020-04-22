#define HOMEPAGE "https://wvr.sh/start"
#define BKMS "/home/mitch/fil/bkm.txt"
#define DOWNLOADS "/home/mitch/dls"

const static SearchEngine searchengines[] = {
    { "",       "https://duckduckgo.com/?q=%s" },
    { "g",      "https://startpage.com/do/dsearch?q=%s"   },
    { "sp",     "https://startpage.com/do/dsearch?q=%s"   },
    { "image",  "https://duckduckgo.com/?q=!image %s" },
    { "images", "https://duckduckgo.com/?q=!image %s" },
    { "git",    "https://github.com/search?utf8=&q=%s&type=" },
    { "ugit",   "https://github.com/%s" },
    { "gist",   "https://gist.github.com/search?q=%s" },
    { "wiki",   "https://en.wikipedia.org/wiki/%s" },
    { "w",      "https://en.wikipedia.org/wiki/%s" },
    { "metal",  "https://metal-archives.com/search?searchString=%s&type=band_name" },
    { "ebay",   "https://ebay.com/sch/i.html?_from=R40&_nkw=%s&_sacat=0&_sop=15&rt=nc&LH_BIN=1" },
    { "az",     "https://smile.amazon.com/s?k=%s" },
    { "gr",     "https://goodreads.com/search?q=%s" },
    { "discogs","https://discogs.com/search?q=%s&btn=&type=all" },
    { "bc",     "https://bandcamp.com/search?q=%s" },
    { "arch",   "https://wiki.archlinux.org/index.php?search=%s" },
    { "gentoo", "https://wiki.gentoo.org/index.php?search=%s" },
    { "r",      "https://old.reddit.com/r/%s" },
    { "u",      "https://old.reddit.com/u/%s" },
    { "4",      "https://boards.4chan.org/%s" },
    { "yt",     "https://invidio.us/search?q=%s" },
    { "reddit", "https://old.reddit.com/search?q=%s" },
    { "dict",   "https://thefreedictionary.com/%s" },
    { "thes",   "https://thesaurus.com/browse/%s" },
    { "urban",  "https://urbandictionary.com/define.php?term=%s" },
};

static int winsize[] = { 1000, 600 }; // default window size

static const char *fulluseragent  = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.157 Safari/537.36";
static const char *scriptfile     = "~/.surf/scripts";
static const char *styledir       = "~/.surf/styles";
static const char *cookiefile     = "~/.cache/surf/cookies.txt";
static const char *cachedir       = "~/.cache/surf/cache";
static const char *certdir        = "~/.cache/surf/certificates/";

static Parameter defconfig[ParameterLast] = {
    /* parameter                    Arg value       priority */
    [StrictTLS]           =       { { .i = 1 },     }, // HTTPS Everywhere
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    [JavaScript]          =       { { .i = 1 },     }, // NoScript -- whitelist or blacklist below
                                                       // (or turn on as needed with ctrl+shift+s)
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */ 
    [CookiePolicies]      =       { { .v = "@" }, }, // @ = do not allow external, def:"@Aa"
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    [LoadImages]          =       { { .i = 1 },     }, // disable on a page with ctrl+shift+i
    [DiskCache]           =       { { .i = 1 },     }, // local file cache
    [Geolocation]         =       { { .i = 0 },     }, // NOPE!
    [ShowIndicators]      =       { { .i = 0 },     }, // annoying
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    [FontSize]            =       { { .i = 14 },    },
    [ZoomLevel]           =       { { .f = 1.0 },   }, // default zoom level
    [MediaManualPlay]     =       { { .i = 1 },     }, // don't autoplay videos
    [ClipboardNotPrimary] =       { { .i = 1 },     }, // clipbard vs primary selection
    [ScrollBars]          =       { { .i = 0 },     },
    [SmoothScrolling]     =       { { .i = 1 },     },
    [AcceleratedCanvas]   =       { { .i = 1 },     },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    [SpellChecking]       =       { { .i = 0 },     }, // spell checking is laggy for me
    [SpellLanguages]      =       { { .v = ((char *[]){ "en_US,de_DE", NULL }) }, },
    [Java]                =       { { .i = 0 },     },
    [AccessMicrophone]    =       { { .i = 0 },     },
    [AccessWebcam]        =       { { .i = 0 },     },
    [Certificate]         =       { { .i = 0 },     },
    [CaretBrowsing]       =       { { .i = 0 },     },
    [DefaultCharset]      =       { { .v = "UTF-8" }, },
    [DNSPrefetch]         =       { { .i = 0 },     },
    [FileURLsCrossAccess] =       { { .i = 0 },     },
    [FrameFlattening]     =       { { .i = 0 },     },
    [HideBackground]      =       { { .i = 0 },     },
    [Plugins]             =       { { .i = 1 },     },
    [PreferredLanguages]  =       { { .v = (char *[]){ NULL } }, },
    [SiteQuirks]          =       { { .i = 1 },     },
    [Style]               =       { { .i = 1 },     },
    [WebGL]               =       { { .i = 0 },     },
    [RunInFullscreen]     =       { { .i = 0 },     },
    [Inspector]           =       { { .i = 0 },     },
    [KioskMode]           =       { { .i = 0 },     },
};

static UriParameters uriparams[] = {
    { ".*webmail\\.dsu\\.edu(/|$)",     { [JavaScript] = { { .i = 1 }, 1 }, }, },
    { ".*github\\.com(/|$)",     { [JavaScript] = { { .i = 1 }, 1 }, }, },
    { ".*reddit\\.com(/|$)",     { [JavaScript] = { { .i = 1 }, 1 }, }, },
    { ".*startpage\\.com(/|$)",  { [JavaScript] = { { .i = 1 }, 1 }, }, },
    { ".*ebay\\.com(/|$)",       { [JavaScript] = { { .i = 1 }, 1 }, }, },
    { ".*paypal\\.com(/|$)",     { [JavaScript] = { { .i = 1 }, 1 }, }, },
    { ".*discordapp\\.com(/|$)", { [JavaScript] = { { .i = 1 }, 1 }, }, },
};

// bring up my bookmarks file in the url bar,
// while preserving the current page link at the top
#define SETPROP(p, q, prompt) { \
    .v = (const char *[]){ "/bin/sh", "-c", \
        "prop=\"`printf '%s\n' $(xprop -id $3 $0 " \
        "| sed -e 's|_SURF.*.= \"||' -e 's|\"$||' " \
        "&& cat $4) | menu -w $3 -p $2`\" &&" \
        "xprop -id $3 -f $1 8s -set $1 \"$prop\"", \
        p, q, prompt, winid, BKMS, NULL \
    } \
}

// same as above but dont cat my bookmarks
#define SETFIND(p, q, prompt) { \
    .v = (const char *[]){ "/bin/sh", "-c", \
        "prop=\"$(echo | menu -w $3 -p $2)\" &&" \
        "xprop -id $3 -f $1 8s -set $1 \"$prop\"", \
        p, q, prompt, winid, NULL \
    } \
}

#define DOWNLOAD(d, r) { \
    .v = (const char *[]){ "/bin/sh", "-c", \
        "st -t floating-st -e /bin/sh -c \"mkdir -p $4 ; cd $4 " \
        "&& curl -g -L -J -O --user-agent '$1'" \
        " --referer '$2' -b $3 -c $3 '$0';" \
        " sleep 5;\"", \
        d, useragent, r, cookiefile, DOWNLOADS, NULL \
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

static SiteSpecific styles[] = {
    /* regexp               file in $styledir */
    { ".*",                 "default.css" },
};

// http://git.gnome.org/browse/gtk+/plain/gdk/gdkkeysyms.h
#define MODKEY GDK_CONTROL_MASK
#define SHIFT GDK_SHIFT_MASK
static Key keys[] = {
    /* modifier              keyval          function    arg */
    { MODKEY,                GDK_KEY_g,      spawn,      SETPROP("_SURF_URI", "_SURF_GO", "Go:") },

    /* -*-*-*-*-*-*-*-*-*-*- CUSTOM FUNCS *-*-*-*-*-*-*-*-*-*-*-*-* */
    { MODKEY|SHIFT,            GDK_KEY_b,      spawn,      BM_ADD   },
    { MODKEY,                  GDK_KEY_y,      spawn,      MPV_URL  },
    { MODKEY,                  GDK_KEY_space,  spawn,      GO_HOME  },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

    /* -*-*-*-*-*-*-*-*-*-*- GENERAL -*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    { 0,                     GDK_KEY_Escape, stop,            { 0 } },
    { MODKEY,                GDK_KEY_c,      stop,            { 0 } },
    { MODKEY,                GDK_KEY_c,      clipboard,  { .i = 0 } },
    { 0,                     GDK_KEY_F11,    togglefullscreen, { 0 } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_p,      print,      { 0 } },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

    /* -*-*-*-*-*-*-*-*-*-*-*-* FINDING *-*-*-*-*-*-*-*-*-*-*-*-*-* */
    { MODKEY,   GDK_KEY_slash,   spawn,  SETFIND("_SURF_FIND", "_SURF_FIND", "Find:") },
    { MODKEY,   GDK_KEY_period,  find,   { .i = +1 } },
    { MODKEY,   GDK_KEY_comma,   find,   { .i = -1 } },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

    /* -*-*-*-*-*-*-*-*-*-*-* HISTORY *-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    { MODKEY,               GDK_KEY_o,   navigate,   { .i = +1 } },
    { MODKEY,               GDK_KEY_i,   navigate,   { .i = -1 } },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

    /* -*-*-*-*-*-*-*-* RELOADING -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    { MODKEY,GDK_KEY_F5,       reload,  { .i = 0 } }, // reload page
    { MODKEY,GDK_KEY_r,        reload,  { .i = 0 } }, // reload page
    { MODKEY|SHIFT, GDK_KEY_r, reload,  { .i = 1 } }, // "hard" reload
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

    /* -*-*-*-*-*-*-*-*- VIM MODE SCROLLING -*-*-*-*-*-*-*-*-*-*-*-* */
    { MODKEY,                GDK_KEY_j,      scrollv,     { .i = +10 } },
    { MODKEY,                GDK_KEY_k,      scrollv,     { .i = -10 } },
    { MODKEY,                GDK_KEY_f,      scrollv,     { .i = -50 } },
    { MODKEY,                GDK_KEY_b,      scrollv,     { .i = +50 } },
    { MODKEY|SHIFT,          GDK_KEY_j,      scrollh,     { .i = +10 } },
    { MODKEY|SHIFT,          GDK_KEY_k,      scrollh,     { .i = -10 } },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

    /* -*-*-*-*-*-*-*-*- ZOOMING -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    { MODKEY, GDK_KEY_minus,      zoom,       { .i = -1 } },
    { MODKEY, GDK_KEY_equal,      zoom,       { .i = +1 } },
    { MODKEY, GDK_KEY_plus,       zoom,       { .i = +1 } },
    { MODKEY, GDK_KEY_BackSpace,  zoom,       { .i = +0 } },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    
    /* -*-*-*-*-*-*-*-* TOGGLES -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
    { MODKEY|GDK_SHIFT_MASK, GDK_KEY_a, togglecookiepolicy, { 0 } },
    { MODKEY|GDK_SHIFT_MASK, GDK_KEY_s, toggle,     { .i = JavaScript } },
    { MODKEY|GDK_SHIFT_MASK, GDK_KEY_i, toggle,     { .i = LoadImages } },
    { MODKEY|GDK_SHIFT_MASK, GDK_KEY_o, toggleinspector, { 0 } },
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
};

/* target can be OnDoc, OnLink, OnImg, OnMedia, OnEdit, OnBar, OnSel, OnAny */
static Button buttons[] = {
    /* target       event mask      button  function        argument        stop event */
    { OnLink,       0,              2,      clicknewwindow, { .i = 0 },     1 },
    { OnLink,       MODKEY,         2,      clicknewwindow, { .i = 1 },     1 },
    { OnLink,       MODKEY,         1,      clicknewwindow, { .i = 1 },     1 },
    { OnAny,        0,              8,      clicknavigate,  { .i = -1 },    1 },
    { OnAny,        0,              9,      clicknavigate,  { .i = +1 },    1 },
};

static SiteSpecific certs[] = {
    /* regexp               file in $certdir */
    { "://suckless\\.org/", "suckless.org.crt" },
};
static WebKitFindOptions findopts = WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE |
                                    WEBKIT_FIND_OPTIONS_WRAP_AROUND;
static const int surfuseragent    = 0;
