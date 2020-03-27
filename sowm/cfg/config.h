#ifndef CONFIG_H
#define CONFIG_H

#define TH  24
#define TC  255 + (255<<8) + (255<<16)

const char* term[]    = {"st",   0};
const char* menu[]    = {"menu", "run", "-p", "Run:", 0};
const char* brws[]    = {"brws", 0};
const char* scrap[]   = {"scrap", 0};

#define MOD Mod1Mask
static struct key keys[] = {
    {MOD,      XK_q,   win_kill,   {0}},
    {MOD,      XK_c,   win_center, {0}},
    {MOD,      XK_f,   win_fs,     {0}},

    {MOD,           XK_Tab, win_next,   {0}},
    {MOD|ShiftMask, XK_Tab, win_prev,   {0}},

    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
    {MOD, XK_p,      run, {.com = menu}},
    {MOD, XK_Return, run, {.com = term}},
    {MOD, XK_w,      run, {.com = brws}},
    {0,   XK_Print,  run, {.com = scrap}},
    /* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

    {MOD,           XK_1, ws_go,     {.i = 1}},
    {MOD,           XK_2, ws_go,     {.i = 2}},
    {MOD,           XK_3, ws_go,     {.i = 3}},
    {MOD,           XK_4, ws_go,     {.i = 4}},
    /* {MOD,           XK_5, ws_go,     {.i = 5}}, */
    /* {MOD,           XK_6, ws_go,     {.i = 6}}, */
    {MOD|ShiftMask, XK_1, win_to_ws, {.i = 1}},
    {MOD|ShiftMask, XK_2, win_to_ws, {.i = 2}},
    {MOD|ShiftMask, XK_3, win_to_ws, {.i = 3}},
    {MOD|ShiftMask, XK_4, win_to_ws, {.i = 4}},
    /* {MOD|ShiftMask, XK_5, win_to_ws, {.i = 5}}, */
    /* {MOD|ShiftMask, XK_6, win_to_ws, {.i = 6}}, */
};

#endif
