static int winsize[] = { 1280, 800 };
static WebKitFindOptions findopts = WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE | WEBKIT_FIND_OPTIONS_WRAP_AROUND;

#define SETPROP(r, s, p) { \
    .v = (const char *[]){ "/bin/sh", "-c", \
        "[ \"$(pgrep dmenu)\" ] && pkill -9 dmenu ; " \
        ". ${HOME}/.cache/wal/colors.sh ; prop=\"$(printf '%b' \"$(xprop -id $1 $2 " \
        "| sed \"s/^$2(STRING) = //;s/^\\\"\\(.*\\)\\\"$/\\1/\" && cat ${HOME}/var/files/bookmarks/bookmarks.txt)\" " \
        "| dmenu -nb $color0 -nf $color15 -sb $color2 -sf $color15 -l 10 -p \"$4\" -w $1)\" && " \
        "xprop -id $1 -f $3 8u -set $3 \"$prop\"", \
        "surf-setprop", winid, r, s, p, NULL \
    } \
}

#define SETSEARCH(r, s, p) { \
    .v = (const char *[]){ "/bin/sh", "-c", \
            ". ${HOME}/.cache/wal/colors.sh ; [ \"$(pgrep dmenu)\" ] && pkill -9 dmenu ; prop=\"$(printf '%b' \"$(xprop -id $1 $2 " \
            "| sed \"s/^$2(STRING) = //;s/^\\\"\\(.*\\)\\\"$/\\1/\")\" " \
            "| dmenu -nb $color0 -nf $color15 -sb $color2 -sf $color15 -p \"$4\" -w $1)\" && xprop -id $1 -f $3 8s -set $3 \"$prop\"", \
            "surf-setprop", winid, r, s, p, NULL \
    } \
}
