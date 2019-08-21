static const char norm_fg[] = "#423F3D";
static const char norm_bg[] = "#fcfaf8";
static const char norm_border[] = "#7e7d7c";

static const char sel_fg[] = "#423F3D";
static const char sel_bg[] = "#998d66";
static const char sel_border[] = "#423F3D";

static const char urg_fg[] = "#423F3D";
static const char urg_bg[] = "#998866";
static const char urg_border[] = "#998866";

static const char *colors[][3]      = {
    /*               fg           bg         border                         */
    [SchemeNorm] = { norm_fg,     norm_bg,   norm_border }, // unfocused wins
    [SchemeSel]  = { sel_fg,      sel_bg,    sel_border },  // the focused win
    [SchemeUrg] =  { urg_fg,      urg_bg,    urg_border },
};
