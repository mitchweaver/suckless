#include <sys/wait.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>

#include <glib/gstdio.h>
#include <gtk/gtkx.h>
#include <webkit2/webkit2.h>
#include <X11/Xatom.h>

#include "arg.h"

#define LENGTH(x)       (sizeof(x) / sizeof(x[0]))
#define CLEANMASK(mask) (mask & (MODKEY|GDK_SHIFT_MASK))

enum { AtomFind, AtomGo, AtomUri, AtomLast };

enum {  OnDoc   = WEBKIT_HIT_TEST_RESULT_CONTEXT_DOCUMENT,
        OnLink  = WEBKIT_HIT_TEST_RESULT_CONTEXT_LINK,
        OnImg   = WEBKIT_HIT_TEST_RESULT_CONTEXT_IMAGE,
        OnMedia = WEBKIT_HIT_TEST_RESULT_CONTEXT_MEDIA,
        OnEdit  = WEBKIT_HIT_TEST_RESULT_CONTEXT_EDITABLE,
        OnBar   = WEBKIT_HIT_TEST_RESULT_CONTEXT_SCROLLBAR,
        OnSel   = WEBKIT_HIT_TEST_RESULT_CONTEXT_SELECTION,
        OnAny   = OnDoc | OnLink | OnImg | OnMedia | OnEdit | OnBar | OnSel, };

typedef enum {
    DNSPrefetch, FontSize,
    JavaScript, StrictTLS,
    Images, Style, WebGL,
    ZoomLevel, ParameterLast
} ParamName;

typedef union { int i; float f; const void *v; } Arg;
typedef struct { Arg val; int prio; } Parameter;

typedef struct Client {
    GtkWidget *win;
    WebKitWebView *view;
    WebKitFindController *finder;
    WebKitHitTestResult *mousepos;
    GTlsCertificate *cert, *failedcert;
    GTlsCertificateFlags tlserr;
    Window xid;
    int progress, https, insecure, errorpage;
    const char *title, *overtitle, *targeturi;
    const char *needle;
    struct Client *next;
} Client;

typedef struct {
    guint mod;
    guint keyval;
    void (*func)(Client *c, const Arg *a);
    const Arg arg;
} Key;

typedef struct {
    unsigned int target;
    unsigned int mask;
    guint button;
    void (*func)(Client *c, const Arg *a, WebKitHitTestResult *h);
    const Arg arg;
    unsigned int stopevent;
} Button;

typedef struct { char *token; char *uri; } SearchEngine;

typedef struct {
    const char *uri;
    Parameter config[ParameterLast];
    regex_t re;
} UriParameters;

typedef struct {
    char *regex;
    char *file;
    regex_t re;
} SiteSpecific;

static void die(const char *errstr, ...);
static void setup(void);
static void sigchld(int unused);
static void sighup(int unused);
static char *buildfile(const char *path);
static char *buildpath(const char *path);
static Client *newclient(Client *c);
static void loaduri(Client *c, const Arg *a);
static const char *geturi(Client *c);
static void setatom(Client *c, int a, const char *v);
static const char *getatom(Client *c, int a);
static void updatetitle(Client *c);
static void getpagestats(Client *c);
static char **parse_address(const char *url);
static char **parse_url(char *str);
static void seturiparameters(Client *c, const char *uri, ParamName *params);
static void setparameter(Client *c, int refresh, ParamName p, const Arg *a);
static const char *getstyle(const char *uri);
static void setstyle(Client *c, const char *file);
static void runscript(Client *c);
static void evalscript(Client *c, const char *jsstr, ...);
static void updatewinid(Client *c);
static void newwindow(Client *c, const Arg *a, int noembed);
static void spawn(Client *c, const Arg *a);
static void destroyclient(Client *c);
static void cleanup(void);

/* GTK/WebKit */
static WebKitWebView *newview(Client *c, WebKitWebView *rv);
static void initwebextensions(WebKitWebContext *wc, Client *c);
static GtkWidget *createview(WebKitWebView *v, WebKitNavigationAction *a, Client *c);
static gboolean buttonreleased(GtkWidget *w, GdkEvent *e, Client *c);
static GdkFilterReturn processx(GdkXEvent *xevent, GdkEvent *event, gpointer d);
static gboolean winevent(GtkWidget *w, GdkEvent *e, Client *c);
static void showview(WebKitWebView *v, Client *c);
static GtkWidget *createwindow(Client *c);
static gboolean loadfailedtls(WebKitWebView *v, gchar *uri, GTlsCertificate *cert, GTlsCertificateFlags err, Client *c);
static void loadchanged(WebKitWebView *v, WebKitLoadEvent e, Client *c);
static void progresschanged(WebKitWebView *v, GParamSpec *ps, Client *c);
static void titlechanged(WebKitWebView *view, GParamSpec *ps, Client *c);
static void mousetargetchanged(WebKitWebView *v, WebKitHitTestResult *h, guint modifiers, Client *c);
static gboolean permissionrequested(WebKitWebView *v, WebKitPermissionRequest *r, Client *c);
static gboolean decidepolicy(WebKitWebView *v, WebKitPolicyDecision *d, WebKitPolicyDecisionType dt, Client *c);
static void decidenavigation(WebKitPolicyDecision *d, Client *c);
static void decidenewwindow(WebKitPolicyDecision *d, Client *c);
static void decideresource(WebKitPolicyDecision *d, Client *c);
static void insecurecontent(WebKitWebView *v, WebKitInsecureContentEvent e, Client *c);
static void downloadstarted(WebKitWebContext *wc, WebKitDownload *d, Client *c);
static void responsereceived(WebKitDownload *d, GParamSpec *ps, Client *c);
static void download(Client *c, WebKitURIResponse *r);
static void closeview(WebKitWebView *v, Client *c);
static void destroywin(GtkWidget* w, Client *c);
static gchar *parseuri(const gchar *uri);

/* Hotkeys */
static void reload(Client *c, const Arg *a);
static void print(Client *c, const Arg *a);
static void zoom(Client *c, const Arg *a);
static void scroll(Client *c, const Arg *a);
static void navigate(Client *c, const Arg *a);
static void stop(Client *c, const Arg *a);
static void toggle(Client *c, const Arg *a);
static void find(Client *c, const Arg *a);

/* Buttons */
static void clicknavigate(Client *c, const Arg *a, WebKitHitTestResult *h);
static void clicknewwindow(Client *c, const Arg *a, WebKitHitTestResult *h);
static void clickexternplayer(Client *c, const Arg *a, WebKitHitTestResult *h);

static char winid[64];
static char pagestats[2];
static Atom atoms[AtomLast];
static Window embed;
static int showxid;
static Display *dpy;
static Client *clients;
static GdkDevice *gdkkb;
static char *stylefile;
static const char *useragent;
static Parameter *curconfig;
static int modparams[ParameterLast];
char *argv0;

static ParamName loadtransient[] = {
    DNSPrefetch,
    JavaScript,
    StrictTLS,
    ParameterLast
};

static ParamName loadcommitted[] = {
    FontSize,
    Style,
    ZoomLevel,
    ParameterLast
};

static ParamName loadfinished[] = { ParameterLast };

#include "config.h"

void die(const char *errstr, ...) {
    va_list ap;
    va_start(ap, errstr);
    vfprintf(stderr, errstr, ap);
    va_end(ap);
    exit(1);
}

void setup(void) {
    GdkDisplay *gdpy;
    int i, j;

    sigchld(0);
    if (signal(SIGHUP, sighup) == SIG_ERR) die("Can't install SIGHUP handler");
    if (!(dpy = XOpenDisplay(NULL))) die("Can't open default display");

    atoms[AtomFind] = XInternAtom(dpy, "_SURF_FIND", False);
    atoms[AtomGo] = XInternAtom(dpy, "_SURF_GO", False);
    atoms[AtomUri] = XInternAtom(dpy, "_SURF_URI", False);

    gtk_init(NULL, NULL);

    gdpy = gdk_display_get_default();
    curconfig = defconfig;
    cookiefile = buildfile(cookiefile);
    scriptfile = buildfile(scriptfile);
    cachedir   = buildpath(cachedir);

    gdkkb = gdk_seat_get_keyboard(gdk_display_get_default_seat(gdpy));

    if (!stylefile) {
        styledir = buildpath(styledir);
        for (i = 0; i < LENGTH(styles); ++i) {
            if (!regcomp(&(styles[i].re), styles[i].regex, REG_EXTENDED))
                styles[i].file = g_strconcat(styledir, "/", styles[i].file, NULL);
            else {
                fprintf(stderr, "Could not compile regex: %s\n", styles[i].regex);
                styles[i].regex = NULL;
            }
        }
        g_free(styledir);
    } else stylefile = buildfile(stylefile);

    for (i = 0; i < LENGTH(uriparams); ++i) {
        if (regcomp(&(uriparams[i].re), uriparams[i].uri,
            REG_EXTENDED)) {
            fprintf(stderr, "Could not compile regex: %s\n", uriparams[i].uri);
            uriparams[i].uri = NULL;
            continue;
        }
        for (j = 0; j < ParameterLast; ++j) {
            if (defconfig[j].prio >= uriparams[i].config[j].prio)
                uriparams[i].config[j] = defconfig[j];
        }
    }
}

void sigchld(int unused) {
    if (signal(SIGCHLD, sigchld) == SIG_ERR) die("Can't install SIGCHLD handler");
    while (waitpid(-1, NULL, WNOHANG) > 0) ;
}

void sighup(int unused) {
    Arg a = { .i = 0 };
    Client *c;
    for (c = clients; c; c = c->next) reload(c, &a);
}

char * buildfile(const char *path) {
    char *dname, *bname, *bpath, *fpath;
    FILE *f;

    dname = g_path_get_dirname(path);
    bname = g_path_get_basename(path);

    bpath = buildpath(dname);
    g_free(dname);

    fpath = g_build_filename(bpath, bname, NULL);
    g_free(bpath);
    g_free(bname);

    if (!(f = fopen(fpath, "a"))) die("Could not open file: %s\n", fpath);

    g_chmod(fpath, 0600); /* always */
    fclose(f);

    return fpath;
}

char * buildpath(const char *path) {
    char *apath, *name, *p, *fpath;
    if (path[0] == '~') {
        if (path[1] == '/' || path[1] == '\0') {
            p = (char *)&path[1];
        } else {
            if ((p = strchr(path, '/'))) name = g_strndup(&path[1], --p - path);
            else name = g_strdup(&path[1]);
            g_free(name);
        }
        apath = g_build_filename(getenv("HOME"), p, NULL);
    } else apath = g_strdup(path);

    if (g_mkdir_with_parents(apath, 0700) < 0)
        die("Could not access directory: %s\n", apath);

    fpath = realpath(apath, NULL);
    g_free(apath);

    return fpath;
}

Client * newclient(Client *rc) {
    Client *c;

    if (!(c = calloc(1, sizeof(Client)))) die("Cannot malloc!\n");

    c->next = clients;
    clients = c;

    c->progress = 100;
    c->view = newview(c, rc ? rc->view : NULL);

    return c;
}

void loaduri(Client *c, const Arg *a) {
    struct stat st;
    char *url, *path;
    const char *uri = a->v;

    if (g_strcmp0(uri, "") == 0) return;

    if (g_str_has_prefix(uri, "http://")  ||
        g_str_has_prefix(uri, "https://") ||
        g_str_has_prefix(uri, "file://")) {
        url = g_strdup(uri);
    } else if (*uri == ' ') 
        url = g_strdup_printf("%s%s", searchengine, uri + 1);
    else if (!stat(uri, &st) && (path = realpath(uri, NULL))) {
        url = g_strdup_printf("file://%s", path);
        free(path);
    } else url = parseuri(uri);
    setatom(c, AtomUri, url);
    webkit_web_view_load_uri(c->view, url);
    updatetitle(c);
    g_free(url);
}

const char * geturi(Client *c) {
    const char *uri;
    if (!(uri = webkit_web_view_get_uri(c->view))) uri = "about:blank";
    return uri;
}

void setatom(Client *c, int a, const char *v) {
    XChangeProperty(dpy, c->xid, atoms[a], XA_STRING, 8, PropModeReplace,
                    (unsigned char *)v, strlen(v) + 1);
    XSync(dpy, False);
}

const char * getatom(Client *c, int a) {
    static char buf[BUFSIZ];
    Atom adummy;
    int idummy;
    unsigned long ldummy;
    unsigned char *p = NULL;
    XSync(dpy, False);
    XGetWindowProperty(dpy, c->xid, 
            atoms[a], 0L, BUFSIZ, False, AnyPropertyType,
            &adummy, &idummy, &ldummy, &ldummy, &p);
    if (p) strncpy(buf, (char *)p, LENGTH(buf) - 1);
    else buf[0] = '\0';
    XFree(p);
    return buf;
}

void updatetitle(Client *c) {
    char *title;
    const char *name = c->overtitle ? c->overtitle : c->title ? c->title : "";
    gtk_window_set_title(GTK_WINDOW(c->win), name);
}

void getpagestats(Client *c) {
    if (c->https) pagestats[0] = (c->tlserr || c->insecure) ?  'U' : 'T';
    else pagestats[0] = '-';
    pagestats[1] = '\0';
}

void seturiparameters(Client *c, const char *uri, ParamName *params) {
    Parameter *config, *uriconfig = NULL;
    int i, p;

    for (i = 0; i < LENGTH(uriparams); ++i) {
        if (uriparams[i].uri &&
            !regexec(&(uriparams[i].re), uri, 0, NULL, 0)) {
            uriconfig = uriparams[i].config;
            break;
        }
    }

    curconfig = uriconfig ? uriconfig : defconfig;

    for (i = 0; (p = params[i]) != ParameterLast; ++i) {
        switch(p) {
        default: /* FALLTHROUGH */
            if (!(defconfig[p].prio < curconfig[p].prio ||
                defconfig[p].prio < modparams[p]))
                continue;
        case Style: setparameter(c, 0, p, &curconfig[p].val);
        }
    }
}

void setparameter(Client *c, int refresh, ParamName p, const Arg *a) {
    GdkRGBA bgcolor = { 0 };
    WebKitSettings *s = webkit_web_view_get_settings(c->view);

    modparams[p] = curconfig[p].prio;

    switch (p) {
        case DNSPrefetch: webkit_settings_set_enable_dns_prefetching(s, a->i); return; /* do not update */
        case FontSize: webkit_settings_set_default_font_size(s, a->i); return; /* do not update */
        case JavaScript: webkit_settings_set_enable_javascript(s, a->i); break;
        case StrictTLS:
            webkit_web_context_set_tls_errors_policy(
                webkit_web_view_get_context(c->view), a->i ?
                WEBKIT_TLS_ERRORS_POLICY_FAIL :
                WEBKIT_TLS_ERRORS_POLICY_IGNORE);
            break;
        case Style:
            webkit_user_content_manager_remove_all_style_sheets(
                webkit_web_view_get_user_content_manager(c->view));
            if (a->i) setstyle(c, getstyle(geturi(c)));
            refresh = 0;
            break;
        case WebGL:
            webkit_settings_set_enable_webgl(s, a->i);
            break;
        case ZoomLevel: 
            webkit_web_view_set_zoom_level(c->view, a->f); 
            return; /* do not update */
    }

    updatetitle(c);
    if (refresh) reload(c, a);
}

const char * getstyle(const char *uri) {
    int i;
    if (stylefile) return stylefile;
    for (i = 0; i < LENGTH(styles); ++i) {
        if (styles[i].regex && !regexec(&(styles[i].re), uri, 0, NULL, 0))
            return styles[i].file;
    }
    return "";
}

void setstyle(Client *c, const char *file) {
    gchar *style;
    if (!g_file_get_contents(file, &style, NULL, NULL)) {
        fprintf(stderr, "Could not read style file: %s\n", file);
        return;
    }
    webkit_user_content_manager_add_style_sheet(
        webkit_web_view_get_user_content_manager(c->view),
        webkit_user_style_sheet_new(style,
        WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
        WEBKIT_USER_STYLE_LEVEL_USER,
        NULL, NULL));
    g_free(style);
}

void runscript(Client *c) {
    gchar *script;
    gsize l;
    if (g_file_get_contents(scriptfile, &script, &l, NULL) && l)
        evalscript(c, "%s", script);
    g_free(script);
}

void evalscript(Client *c, const char *jsstr, ...) {
    va_list ap;
    gchar *script;
    va_start(ap, jsstr);
    script = g_strdup_vprintf(jsstr, ap);
    va_end(ap);
    webkit_web_view_run_javascript(c->view, script, NULL, NULL, NULL);
    g_free(script);
}

void updatewinid(Client *c) { snprintf(winid, LENGTH(winid), "%lu", c->xid); }

void newwindow(Client *c, const Arg *a, int noembed) {
    int i = 0;
    char tmp[64];
    const char *cmd[29], *uri;
    const Arg arg = { .v = cmd };
    cmd[i++] = argv0;
    cmd[i++] = "-a";
    cmd[i++] = "@";
    if (cookiefile && g_strcmp0(cookiefile, "")) {
        cmd[i++] = "-c";
        cmd[i++] = cookiefile;
    }
    if (stylefile && g_strcmp0(stylefile, "")) {
        cmd[i++] = "-C";
        cmd[i++] = stylefile;
    }
    if (embed && !noembed) {
        cmd[i++] = "-e";
        snprintf(tmp, LENGTH(tmp), "%lu", embed);
        cmd[i++] = tmp;
    }
    cmd[i++] = curconfig[Style].val.i ?           "-M" : "-m" ;
    if (scriptfile && g_strcmp0(scriptfile, "")) {
        cmd[i++] = "-r";
        cmd[i++] = scriptfile;
    }
    cmd[i++] = curconfig[JavaScript].val.i ? "-S" : "-s";
    cmd[i++] = curconfig[StrictTLS].val.i ? "-T" : "-t";
    if (fulluseragent && g_strcmp0(fulluseragent, "")) {
        cmd[i++] = "-u";
        cmd[i++] = fulluseragent;
    }
    if (showxid) cmd[i++] = "-w";
    cmd[i++] = "--";
    if ((uri = a->v)) cmd[i++] = uri;
    cmd[i] = NULL;
    spawn(c, &arg);
}

void spawn(Client *c, const Arg *a) {
    if (fork() == 0) {
        if (dpy) close(ConnectionNumber(dpy));
        setsid();
        execvp(((char **)a->v)[0], (char **)a->v);
        fprintf(stderr, "%s: execvp %s", argv0, ((char **)a->v)[0]);
        perror(" failed");
        exit(1);
    }
}

void destroyclient(Client *c) {
    Client *p;
    webkit_web_view_stop_loading(c->view);
    for (p = clients; p && p->next != c; p = p->next) ;
    if (p) p->next = c->next;
    else clients = c->next;
    free(c);
}

void cleanup(void) {
    while (clients) destroyclient(clients);
    g_free(cookiefile);
    g_free(scriptfile);
    g_free(stylefile);
    g_free(cachedir);
    XCloseDisplay(dpy);
}

WebKitWebView * newview(Client *c, WebKitWebView *rv) {
    WebKitWebView *v;
    WebKitSettings *settings;
    WebKitUserContentManager *contentmanager;
    WebKitWebContext *context;

    if (rv) v = WEBKIT_WEB_VIEW( webkit_web_view_new_with_related_view(rv));
    else {
        settings = webkit_settings_new_with_settings(
           "auto-load-images", curconfig[Images].val.i,
           "default-charset", "UTF-8",
           "default-font-size", curconfig[FontSize].val.i,
           "enable-dns-prefetching", curconfig[DNSPrefetch].val.i,
           "enable-html5-database", 1,
           "enable-html5-local-storage", 1,
           "enable-javascript", curconfig[JavaScript].val.i,
           "enable-plugins", 1,
           "enable-accelerated-2d-canvas", 1,
           "enable-site-specific-quirks", 1,
           "enable-smooth-scrolling", 1,
		   "enable-webgl", curconfig[WebGL].val.i,
           "media-playback-requires-user-gesture", 0,
           NULL);

        if (strcmp(fulluseragent, ""))
            webkit_settings_set_user_agent(settings, fulluseragent);
        useragent = webkit_settings_get_user_agent(settings);

        contentmanager = webkit_user_content_manager_new();

        context = webkit_web_context_new_with_website_data_manager(
            webkit_website_data_manager_new( "base-cache-directory", cachedir,
            "base-data-directory", cachedir, NULL));

        webkit_web_context_set_process_model(context,
            WEBKIT_PROCESS_MODEL_MULTIPLE_SECONDARY_PROCESSES);
        webkit_web_context_set_tls_errors_policy(context,
            curconfig[StrictTLS].val.i ? WEBKIT_TLS_ERRORS_POLICY_FAIL :
            WEBKIT_TLS_ERRORS_POLICY_IGNORE);
        webkit_web_context_set_cache_model(context, WEBKIT_CACHE_MODEL_WEB_BROWSER);
        webkit_cookie_manager_set_persistent_storage(
            webkit_web_context_get_cookie_manager(context), cookiefile,
            WEBKIT_COOKIE_PERSISTENT_STORAGE_TEXT);
        webkit_cookie_manager_set_accept_policy(
            webkit_web_context_get_cookie_manager(context),
            WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY);

        g_signal_connect(G_OBJECT(context), "download-started", G_CALLBACK(downloadstarted), c);
        g_signal_connect(G_OBJECT(context), "initialize-web-extensions", G_CALLBACK(initwebextensions), c);

        v = g_object_new(WEBKIT_TYPE_WEB_VIEW,
            "settings", settings,
            "user-content-manager", contentmanager,
            "web-context", context,
            NULL);
    }
    g_signal_connect(G_OBJECT(v), "notify::estimated-load-progress", G_CALLBACK(progresschanged), c);
    g_signal_connect(G_OBJECT(v), "notify::title",                   G_CALLBACK(titlechanged), c);
    g_signal_connect(G_OBJECT(v), "button-release-event",            G_CALLBACK(buttonreleased), c);
    g_signal_connect(G_OBJECT(v), "close",                           G_CALLBACK(closeview), c);
    g_signal_connect(G_OBJECT(v), "create",                          G_CALLBACK(createview), c);
    g_signal_connect(G_OBJECT(v), "decide-policy",                   G_CALLBACK(decidepolicy), c);
    g_signal_connect(G_OBJECT(v), "insecure-content-detected",       G_CALLBACK(insecurecontent), c);
    g_signal_connect(G_OBJECT(v), "load-failed-with-tls-errors",     G_CALLBACK(loadfailedtls), c);
    g_signal_connect(G_OBJECT(v), "load-changed",                    G_CALLBACK(loadchanged), c);
    g_signal_connect(G_OBJECT(v), "mouse-target-changed",            G_CALLBACK(mousetargetchanged), c);
    g_signal_connect(G_OBJECT(v), "permission-request",              G_CALLBACK(permissionrequested), c);
    g_signal_connect(G_OBJECT(v), "ready-to-show",                   G_CALLBACK(showview), c);
    return v;
}

void initwebextensions(WebKitWebContext *wc, Client *c) {
    webkit_web_context_set_web_extensions_directory(wc, WEBEXTDIR);
}

GtkWidget * createview(WebKitWebView *v, WebKitNavigationAction *a, Client *c) {
    Client *n;

    switch (webkit_navigation_action_get_navigation_type(a)) {
        case WEBKIT_NAVIGATION_TYPE_OTHER: /* fallthrough */
            /* popup windows of type “other” are almost always triggered
            * by user gesture, so inverse the logic here */
            /* instead of this, compare destination uri to mouse-over uri for validating window */
            if (webkit_navigation_action_is_user_gesture(a)) return NULL;
        case WEBKIT_NAVIGATION_TYPE_LINK_CLICKED: /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_FORM_SUBMITTED: /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_BACK_FORWARD: /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_RELOAD: /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_FORM_RESUBMITTED:
            n = newclient(c);
            break;
        default: return NULL;
    }
    return GTK_WIDGET(n->view);
}

gboolean buttonreleased(GtkWidget *w, GdkEvent *e, Client *c) {
    WebKitHitTestResultContext element;
    int i;
    element = webkit_hit_test_result_get_context(c->mousepos);
    for (i = 0; i < LENGTH(buttons); ++i) {
        if (element & buttons[i].target &&
            e->button.button == buttons[i].button &&
            CLEANMASK(e->button.state) == CLEANMASK(buttons[i].mask) &&
            buttons[i].func) {
            buttons[i].func(c, &buttons[i].arg, c->mousepos);
            return buttons[i].stopevent;
        }
    }
    return FALSE;
}

GdkFilterReturn processx(GdkXEvent *e, GdkEvent *event, gpointer d) {
    Client *c = (Client *)d;
    XPropertyEvent *ev;
    Arg a;
    if (((XEvent *)e)->type == PropertyNotify) {
        ev = &((XEvent *)e)->xproperty;
        if (ev->state == PropertyNewValue) {
            if (ev->atom == atoms[AtomFind]) {
                find(c, NULL);
                return GDK_FILTER_REMOVE;
            } else if (ev->atom == atoms[AtomGo]) {
                a.v = getatom(c, AtomGo);
                loaduri(c, &a);
                return GDK_FILTER_REMOVE;
            }
        }
    }
    return GDK_FILTER_CONTINUE;
}

gboolean winevent(GtkWidget *w, GdkEvent *e, Client *c) {
    int i;

    switch (e->type) {
        case GDK_ENTER_NOTIFY:
            c->overtitle = c->targeturi;
            updatetitle(c);
            break;
        case GDK_KEY_PRESS:
            for (i = 0; i < LENGTH(keys); ++i) {
                if (gdk_keyval_to_lower(e->key.keyval) ==
                    keys[i].keyval &&
                    CLEANMASK(e->key.state) == keys[i].mod &&
                    keys[i].func) {
                    updatewinid(c);
                    keys[i].func(c, &(keys[i].arg));
                    return TRUE;
                }
            }
        case GDK_LEAVE_NOTIFY:
            c->overtitle = NULL;
            updatetitle(c);
            break;
    }

    return FALSE;
}

void showview(WebKitWebView *v, Client *c) {
    GdkRGBA bgcolor = { 0 };
    GdkWindow *gwin;
    c->finder = webkit_web_view_get_find_controller(c->view);
    c->win = createwindow(c);
    gtk_container_add(GTK_CONTAINER(c->win), GTK_WIDGET(c->view));
    gtk_widget_show_all(c->win);
    gtk_widget_grab_focus(GTK_WIDGET(c->view));
    gwin = gtk_widget_get_window(GTK_WIDGET(c->win));
    c->xid = gdk_x11_window_get_xid(gwin);
    updatewinid(c);
    if (showxid) {
        gdk_display_sync(gtk_widget_get_display(c->win));
        puts(winid);
    }
    gdk_window_set_events(gwin, GDK_ALL_EVENTS_MASK);
    gdk_window_add_filter(gwin, processx, c);
    if (curconfig[ZoomLevel].val.f != 1.0)
        webkit_web_view_set_zoom_level(c->view, curconfig[ZoomLevel].val.f);
    setatom(c, AtomFind, "");
    setatom(c, AtomUri, "about:blank");
}

GtkWidget * createwindow(Client *c) {
    char *wmstr;
    GtkWidget *w;
    if (embed) w = gtk_plug_new(embed);
    else {
        w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        wmstr = g_path_get_basename(argv0);
        gtk_window_set_wmclass(GTK_WINDOW(w), wmstr, "Surf");
        g_free(wmstr);
        wmstr = g_strdup_printf("%s[%lu]", "Surf", webkit_web_view_get_page_id(c->view));
        gtk_window_set_role(GTK_WINDOW(w), wmstr);
        g_free(wmstr);
        gtk_window_set_default_size(GTK_WINDOW(w), winsize[0], winsize[1]);
    }
    g_signal_connect(G_OBJECT(w), "destroy",            G_CALLBACK(destroywin), c);
    g_signal_connect(G_OBJECT(w), "enter-notify-event", G_CALLBACK(winevent), c);
    g_signal_connect(G_OBJECT(w), "key-press-event",    G_CALLBACK(winevent), c);
    g_signal_connect(G_OBJECT(w), "leave-notify-event", G_CALLBACK(winevent), c);
    g_signal_connect(G_OBJECT(w), "window-state-event", G_CALLBACK(winevent), c);
    return w;
}

gboolean loadfailedtls(WebKitWebView *v, gchar *uri, GTlsCertificate *cert, GTlsCertificateFlags err, Client *c) {
    GString *errmsg = g_string_new(NULL);
    gchar *html, *pem;
    c->failedcert = g_object_ref(cert);
    c->tlserr = err;
    c->errorpage = 1;
    if (err & G_TLS_CERTIFICATE_UNKNOWN_CA)
        g_string_append(errmsg, "The signing certificate authority is not known.<br>");
    if (err & G_TLS_CERTIFICATE_BAD_IDENTITY)
        g_string_append(errmsg,
            "The certificate does not match the expected identity "
            "of the site that it was retrieved from.<br>");
    if (err & G_TLS_CERTIFICATE_NOT_ACTIVATED)
        g_string_append(errmsg,
            "The certificate's activation time "
            "is still in the future.<br>");
    if (err & G_TLS_CERTIFICATE_EXPIRED)
        g_string_append(errmsg, "The certificate has expired.<br>");
    if (err & G_TLS_CERTIFICATE_REVOKED)
        g_string_append(errmsg,
            "The certificate has been revoked according to "
            "the GTlsConnection's certificate revocation list.<br>");
    if (err & G_TLS_CERTIFICATE_INSECURE)
        g_string_append(errmsg,
            "The certificate's algorithm is considered insecure.<br>");
    if (err & G_TLS_CERTIFICATE_GENERIC_ERROR)
        g_string_append(errmsg,
            "Some error occurred validating the certificate.<br>");

    g_object_get(cert, "certificate-pem", &pem, NULL);


    char skull[2048];

    #include <stdio.h>
    #include <stdlib.h>

    FILE *cmd = popen(crossbones_ascii, "r");
    
    size_t n;

    while ((n = fread(skull, 1, sizeof(skull)-1, cmd)) > 0) {
        skull[n] = '\0';
        printf("%s", skull);
    }

    html = g_strdup_printf("<p>Could not validate TLS for “%s”<br>%s</p>"
                            "<style type=\"text/css\">"
                            "body {text-align: center; color: white; background: black;}"
                            "</style>"
                           "<p><pre>%s</pre></p>", uri, errmsg->str, skull);
    g_free(pem);
    g_string_free(errmsg, TRUE);

    webkit_web_view_load_alternate_html(c->view, html, uri, NULL);
    g_free(html);

    return TRUE;
}

void loadchanged(WebKitWebView *v, WebKitLoadEvent e, Client *c) {
    const char *uri = geturi(c);
    switch (e) {
        case WEBKIT_LOAD_STARTED:
            setatom(c, AtomUri, uri);
            c->title = uri;
            c->https = c->insecure = 0;
            seturiparameters(c, uri, loadtransient);
            if (c->errorpage) c->errorpage = 0;
            else g_clear_object(&c->failedcert);
            break;
        case WEBKIT_LOAD_REDIRECTED:
            setatom(c, AtomUri, uri);
            c->title = uri;
            seturiparameters(c, uri, loadtransient);
            break;
        case WEBKIT_LOAD_COMMITTED:
            seturiparameters(c, uri, loadcommitted);
            c->https = webkit_web_view_get_tls_info(c->view, &c->cert, &c->tlserr);
            break;
        case WEBKIT_LOAD_FINISHED:
            seturiparameters(c, uri, loadfinished);
            runscript(c);
            break;
    }
    updatetitle(c);
}

void progresschanged(WebKitWebView *v, GParamSpec *ps, Client *c) {
    c->progress = webkit_web_view_get_estimated_load_progress(c->view) * 100;
    updatetitle(c);
}

void titlechanged(WebKitWebView *view, GParamSpec *ps, Client *c) {
    c->title = webkit_web_view_get_title(c->view);
    updatetitle(c);
}

void mousetargetchanged(WebKitWebView *v, WebKitHitTestResult *h, guint modifiers, Client *c) {
    WebKitHitTestResultContext hc = webkit_hit_test_result_get_context(h);
    /* Keep the hit test to know where is the pointer on the next click */
    c->mousepos = h;
    if (hc & OnLink)       c->targeturi = webkit_hit_test_result_get_link_uri(h);
    else if (hc & OnImg)   c->targeturi = webkit_hit_test_result_get_image_uri(h);
    else if (hc & OnMedia) c->targeturi = webkit_hit_test_result_get_media_uri(h);
    else c->targeturi = NULL;
    c->overtitle = c->targeturi;
    updatetitle(c);
}

gboolean permissionrequested(WebKitWebView *v, WebKitPermissionRequest *r, Client *c) {
    if (WEBKIT_IS_GEOLOCATION_PERMISSION_REQUEST(r)) webkit_permission_request_deny(r);
    else if (WEBKIT_IS_USER_MEDIA_PERMISSION_REQUEST(r) && 
            (webkit_user_media_permission_is_for_audio_device( WEBKIT_USER_MEDIA_PERMISSION_REQUEST(r)) ||
            webkit_user_media_permission_is_for_video_device( WEBKIT_USER_MEDIA_PERMISSION_REQUEST(r))))
            webkit_permission_request_deny(r);
    else return FALSE;
    return TRUE;
}

gboolean decidepolicy(WebKitWebView *v, WebKitPolicyDecision *d,
    WebKitPolicyDecisionType dt, Client *c) {
    switch (dt) {
        case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION: decidenavigation(d, c); break;
        case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION: decidenewwindow(d, c); break;
        case WEBKIT_POLICY_DECISION_TYPE_RESPONSE: decideresource(d, c); break;
        default: webkit_policy_decision_ignore(d); break;
    }
    return TRUE;
}

void decidenavigation(WebKitPolicyDecision *d, Client *c) {
    WebKitNavigationAction *a =
        webkit_navigation_policy_decision_get_navigation_action(
        WEBKIT_NAVIGATION_POLICY_DECISION(d));
    switch (webkit_navigation_action_get_navigation_type(a)) {
    default:
        /* Do not navigate to links with a "_blank" target (popup) */
        if (webkit_navigation_policy_decision_get_frame_name(
            WEBKIT_NAVIGATION_POLICY_DECISION(d))) {
            webkit_policy_decision_ignore(d);
        } else webkit_policy_decision_use(d);
        break;
    }
}

void decidenewwindow(WebKitPolicyDecision *d, Client *c) {
    Arg arg;
    WebKitNavigationAction *a =
        webkit_navigation_policy_decision_get_navigation_action(
        WEBKIT_NAVIGATION_POLICY_DECISION(d));
    switch (webkit_navigation_action_get_navigation_type(a)) {
        case WEBKIT_NAVIGATION_TYPE_FORM_RESUBMITTED:
            arg.v = webkit_uri_request_get_uri(webkit_navigation_action_get_request(a));
            newwindow(c, &arg, 0);
            break;
    }
    webkit_policy_decision_ignore(d);
}

void decideresource(WebKitPolicyDecision *d, Client *c) {
    int i, isascii = 1;
    WebKitResponsePolicyDecision *r = WEBKIT_RESPONSE_POLICY_DECISION(d);
    WebKitURIResponse *res = webkit_response_policy_decision_get_response(r);
    const gchar *uri = webkit_uri_response_get_uri(res);
    if (g_str_has_suffix(uri, "/favicon.ico")) {
        webkit_policy_decision_ignore(d);
        return;
    }
    if (!g_str_has_prefix(uri, "http://")
        && !g_str_has_prefix(uri, "https://")
        && !g_str_has_prefix(uri, "file://")
        && strlen(uri) > 0) {
        for (i = 0; i < strlen(uri); i++) {
            if (!g_ascii_isprint(uri[i])) {
                isascii = 0;
                break;
            }
        }
    }

    if (webkit_response_policy_decision_is_mime_type_supported(r))
        webkit_policy_decision_use(d);
    else {
        webkit_policy_decision_ignore(d);
        download(c, res);
    }
}

void insecurecontent(WebKitWebView *v, WebKitInsecureContentEvent e, Client *c) {
    c->insecure = 1;
}

void downloadstarted(WebKitWebContext *wc, WebKitDownload *d, Client *c) {
    g_signal_connect(G_OBJECT(d), "notify::response",
                     G_CALLBACK(responsereceived), c);
}

void responsereceived(WebKitDownload *d, GParamSpec *ps, Client *c) {
    download(c, webkit_download_get_response(d));
    webkit_download_cancel(d);
}

void download(Client *c, WebKitURIResponse *r) {
    Arg a = (Arg)DOWNLOAD(webkit_uri_response_get_uri(r), geturi(c));
    spawn(c, &a);
}

void closeview(WebKitWebView *v, Client *c) {
    gtk_widget_destroy(c->win);
}

void destroywin(GtkWidget* w, Client *c) {
    destroyclient(c);
    if (!clients) gtk_main_quit();
}

gchar * parseuri(const gchar *uri) {
    guint i;
    for (i = 0; i < LENGTH(searchengines); i++) {
        if (searchengines[i].token == NULL || searchengines[i].uri == NULL ||
            *(uri + strlen(searchengines[i].token)) != ' ') continue;
        if (g_str_has_prefix(uri, searchengines[i].token))
            return g_strdup_printf(searchengines[i].uri, uri + strlen(searchengines[i].token) + 1);
    }
    return g_strdup_printf("http://%s", uri);
}

void reload(Client *c, const Arg *a) {
    if (a->i) webkit_web_view_reload_bypass_cache(c->view);
    else webkit_web_view_reload(c->view);
}

void print(Client *c, const Arg *a) {
    webkit_print_operation_run_dialog(webkit_print_operation_new(c->view), GTK_WINDOW(c->win));
}

void zoom(Client *c, const Arg *a) {
    if (a->i > 0) webkit_web_view_set_zoom_level(c->view, curconfig[ZoomLevel].val.f + 0.1);
    else if (a->i < 0) webkit_web_view_set_zoom_level(c->view, curconfig[ZoomLevel].val.f - 0.1);
    else webkit_web_view_set_zoom_level(c->view, 1.0);
    curconfig[ZoomLevel].val.f = webkit_web_view_get_zoom_level(c->view);
}

void scroll(Client *c, const Arg *a) {
    GdkEvent *ev = gdk_event_new(GDK_KEY_PRESS);
    gdk_event_set_device(ev, gdkkb);
    ev->key.window = gtk_widget_get_window(GTK_WIDGET(c->win));
    ev->key.state = GDK_CONTROL_MASK;
    ev->key.time = GDK_CURRENT_TIME;
    switch (a->i) {
        case 'd': ev->key.keyval = GDK_KEY_Down;      break;
        case 'D': ev->key.keyval = GDK_KEY_Page_Down; break;
        case 'l': ev->key.keyval = GDK_KEY_Left;      break;
        case 'r': ev->key.keyval = GDK_KEY_Right;     break;
        case 'U': ev->key.keyval = GDK_KEY_Page_Up;   break;
        case 'u': ev->key.keyval = GDK_KEY_Up;        break;
    }
    gdk_event_put(ev);
}

void navigate(Client *c, const Arg *a) {
    if (a->i < 0) webkit_web_view_go_back(c->view);
    else if (a->i > 0) webkit_web_view_go_forward(c->view);
}

void stop(Client *c, const Arg *a) {
    webkit_web_view_stop_loading(c->view);
}

void toggle(Client *c, const Arg *a) {
    curconfig[a->i].val.i ^= 1;
    setparameter(c, 1, (ParamName)a->i, &curconfig[a->i].val);
}

void find(Client *c, const Arg *a) {
    const char *s, *f;
    if (a && a->i) {
        if (a->i > 0) webkit_find_controller_search_next(c->finder);
        else webkit_find_controller_search_previous(c->finder);
    } else {
        s = getatom(c, AtomFind);
        f = webkit_find_controller_get_search_text(c->finder);
        if (g_strcmp0(f, s) == 0) /* reset search */
            webkit_find_controller_search(c->finder, "", findopts, G_MAXUINT);
        webkit_find_controller_search(c->finder, s, findopts, G_MAXUINT);
        if (strcmp(s, "") == 0)
            webkit_find_controller_search_finish(c->finder);
    }
}

void clicknavigate(Client *c, const Arg *a, WebKitHitTestResult *h) {
    navigate(c, a);
}

void clicknewwindow(Client *c, const Arg *a, WebKitHitTestResult *h) {
    Arg arg;
    arg.v = webkit_hit_test_result_get_link_uri(h);
    newwindow(c, &arg, a->i);
}

void clickexternplayer(Client *c, const Arg *a, WebKitHitTestResult *h) {
    Arg arg;
    arg = (Arg)VIDEOPLAY(webkit_hit_test_result_get_media_uri(h));
    spawn(c, &arg);
}

void on_start() { /* nop */ }

int main(int argc, char *argv[]) {
    Arg arg;
    Client *c;
    ARGBEGIN {
        case 'e': embed = strtol(EARGF(on_start()), NULL, 0); break;
        case 'w': showxid = 1; break;
        case 'z':
            defconfig[ZoomLevel].val.f = strtof(EARGF(on_start()), NULL);
            defconfig[ZoomLevel].prio = 2;
            break;
    } ARGEND;
    if (argc > 0) 
        arg.v = argv[0]; // if arg, launch that site
    else 
        arg.v = HOMEPAGE; // otherwise use homepage from config.h
    setup();
    c = newclient(NULL);
    showview(NULL, c);
    loaduri(c, &arg);
    updatetitle(c);
    gtk_main();
    cleanup();
    return 0;
}
