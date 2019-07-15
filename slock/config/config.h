/* user and group to drop privileges to */
static const char *user  = "nobody";
static const char *group = "nogroup";

static const char *colorname[NUMCOLS] = {
	[INIT] =   "#101315",     /* after initialization */
	[INPUT] =  "#02C164",   /* during input */
	[FAILED] = "#CC3333",   /* wrong password */
};

/* treat a cleared input like a wrong password (color) */
static const int failonclear = 0;

/* default message */
static const char * message = "Suckless: Software that sucks less.";

/* text color */
static const char * text_color = "#ffffff";

/* text size (must be a valid size) */
static const char * text_size = "7x13";

/* time in seconds before the monitor shuts down */
static const int monitortime = 300;

/* allow control key to trigger fail on clear */
static const int controlkeyclear = 0;
