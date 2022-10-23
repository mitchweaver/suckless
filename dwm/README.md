# dwm

## misc notes

### "live" relaunch

to relaunch dwm without losing open windows:

```
while : ; do
    dwm && break || continue
done
```

If dwm is manually closed, it will exit 0 thus
breaking and exiting your X session as normal.

If you kill dwm forcefully, like with `pkill`, it will exit 1
causing the loop to continue and relaunching dwm.
This won't close your X session so all your opened apps will remain.

note: may want to add a sleep in there in case your dwm
      breaks for some reason (ex: segfaults)

### picom


to prevent picom, (or compton), drawing shadows on
your status bar add these lines to `picom.conf`:

```
shadow-exclude = [
    "class_g *= 'dwm'",
    "name *= 'dwm'"
]
```

### conky

make sure you have this line else dwm will try to manage it as a window
```
own_window_type = ‘override’
```
