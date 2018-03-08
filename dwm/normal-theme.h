static const char norm_fg[] = "#bbbbbb";
static const char norm_bg[] = "#222222";
static const char norm_border[] = "#373B41";

static const char sel_fg[] = "#8DEEEE";
static const char sel_bg[] = "#332a2a";
static const char sel_border[] = "#8DEEEE";

static const char urg_fg[] = "#eeeeee";
static const char urg_bg[] = "#ee4444";
static const char urg_border[] = "#ee4444";

static const char *colors[][3]      = {
    /*               fg           bg         border                         */
    [SchemeNorm] = { norm_fg,     norm_bg,   norm_border }, // unfocused wins
    [SchemeSel]  = { sel_fg,      sel_bg,    sel_border }, // the focused win
    [SchemeUrg] =  { urg_fg,      urg_bg,    urg_border },
};
