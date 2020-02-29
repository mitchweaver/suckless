#define MAXLEN 2048
#define WIFI_INT "iwn0"
#define AUDIO_INT "/dev/audioctl0"
const unsigned int interval = 1000;
static const char unknown_str[] = "?";

/* -*-*-*-*-*-*-*- my functions -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */
const char* get_song(void) {
    if ( system("pgrep -x mpv >/dev/null") == 0 ) {
        FILE *fp = popen("sh /home/mitch/bin/mpv/song", "r");
        if (fp) {
            char buf[1024];
            fgets(buf, 1024, fp);
            buf[strlen(buf)-1]='\0';
            pclose(fp);
            return bprintf("%s %s", "🎵", buf);
        } else
            return NULL;
    } else {
        // if no song playing don't show anything
        return bprintf("");
    }
}

const char* get_vpn(void) {
    if ( system("pgrep -x openvpn >/dev/null") == 0 ) {
        FILE *fp = popen("ps -U root | grep openvpn | sed -e 's|.*.\/||' -e 's|\.ovpn||'", "r");
        if (fp) {
            char buf[128];
            fgets(buf, 128, fp);
            buf[strlen(buf)-1]='\0';
            pclose(fp);
            return bprintf("%s %s", "", buf);
        } else
            return NULL;
    } else {
        return bprintf("%s", " None");
    }
}

// NOTE: OpenBSD
const char* get_bat() {
    char *perc = battery_perc("NULL");
    int p =atoi(perc);
    char *emoji;

    // note the difference in spaces is due to the
    // battery icon being a "double width" character
    if (strcmp(battery_state("NULL"), "+") == 0) {
        emoji = " "; // ﮣ
    } else {
             if ( p > 80 ) emoji = "  ";
        else if ( p > 60 ) emoji = "  ";
        else if ( p > 40 ) emoji = "  ";
        else if ( p > 12 ) emoji = "  ";
                      else emoji = "  ";
    }

    char buf[64];
    sprintf(buf, "%s%s%%", emoji, perc);
    bprintf("%s", buf);
}
/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
static const struct arg args[] = {
    /* function format          argument */
    { get_song,     "%s", "NULL" },
    { cpu_perc,     " / 💻 %s%%",    "NULL" },
    { ram_perc,     " /  %s%%",     "NULL" },
    { get_vpn,      " / %s",         "NULL" },
    { wifi_essid,   " /   %s",       WIFI_INT },
    { wifi_perc,    " %s%%",         WIFI_INT },
    { get_bat,      " / %s",         "NULL" },
    { vol_perc,     " / 🔊 %s%%",    AUDIO_INT },
    { datetime,     " / %s",         "%a %b %d - %I:%M %p" },
};

/*
 * function            description                     argument (example)
 *
 * battery_perc        battery percentage              NULL on OpenBSD/FreeBSD
 * battery_state       battery charging state          NULL on OpenBSD/FreeBSD
 * battery_remaining   battery remaining HH:MM         NULL on OpenBSD/FreeBSD
 *
 * cpu_perc            cpu usage in percent            NULL
 * datetime            date and time                   format string (%F %T)
 *
 * disk_free           free disk space in GB           mountpoint path (/)
 * disk_perc           disk usage in percent           mountpoint path (/)
 * disk_total          total disk space in GB          mountpoint path (/")
 * disk_used           used disk space in GB           mountpoint path (/)
 *
 * hostname            hostname                        NULL
 * ipv4                IPv4 address                    interface name (eth0)
 * ipv6                IPv6 address                    interface name (eth0)
 * keymap              layout (variant) of keymap      NULL
 * netspeed_rx         receive network speed           interface name (wlan0)
 * netspeed_tx         transfer network speed          interface name (wlan0)
 * num_files           number of files in a directory  path
 *                                                     (/home/foo/Inbox/cur)
 * ram_free            free memory in GB               NULL
 * ram_perc            memory usage in percent         NULL
 * ram_total           total memory size in GB         NULL
 * ram_used            used memory in GB               NULL
 *
 * run_command         custom shell command            command (echo foo)
 *
 * temp                temperature in degree celsius   NULL on OpenBSD
 * uptime              system uptime                   NULL
 * vol_perc            OSS/ALSA volume in percent      mixer file (/dev/mixer)
 * wifi_perc           WiFi signal in percent          interface name (wlan0)
 * wifi_essid          WiFi ESSID                      interface name (wlan0)
 */
