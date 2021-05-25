/* time in seconds to cancel lock with mouse movement */
static const int timetocancel = 3;

/* allow control key to trigger fail on clear */
static const int controlkeyclear = 0;

/* user and group to drop privileges to */
static const char *user  = "nobody";
static const char *group = "nogroup";

static const char *colorname[NUMCOLS] = {
	[BACKGROUND] =   "black",  /* after initialization */
	[INIT] =   "#2d2d2d",      /* after initialization */
	[INPUT] =  "#005577",      /* during input */
	[FAILED] = "#CC3333",      /* wrong password */
};

/* treat a cleared input like a wrong password (color) */
static const int failonclear = 0;


/* insert grid pattern with scale 1:1, the size can be changed with logosize */
static const int logosize = 75;
static const int logow = 12;	/* grid width and height for right center alignment*/
static const int logoh = 6;

static XRectangle rectangles[9] = {
	/* x	y	w	h */
	{ 0,	3,	1,	3 },
	{ 1,	3,	2,	1 },
	{ 0,	5,	8,	1 },
	{ 3,	0,	1,	5 },
	{ 5,	3,	1,	2 },
	{ 7,	3,	1,	2 },
	{ 8,	3,	4,	1 },
	{ 9,	4,	1,	2 },
	{ 11,	4,	1,	2 },

};
