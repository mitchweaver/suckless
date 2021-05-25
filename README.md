# suckless

## Building

`./build.sh $name`

For questions, feel free to [contact me](mailto:mitch@wvr.sh) or comment on the [git issues](https://github.com/mitchweaver/suckless/issues)

## Troubleshooting

#### It won't build on my machine!

Make sure you have `${LIBS}`, `${CFLAGS}`, and friends set correctly in `config.mk`.  
Notice: I use OpenBSD on most of my machines so that tends to be what is currently uncommented.  
If you're on Linux, NetBSD, etc you will need to comment/uncomment things appropriately.

Is everything set right but still not building? Let me know!

#### I can't find `#include <dwm.h>`, what/where is that?

These headers are how I include themes from my auto-rice script, which you can see [here](https://github.com/mitchweaver/bin/blob/master/rice/rice).

This script dumps a `.h` file to the `~/.cache` directory containing the theme template.  
This `.h` gets included in the build from `-I${HOME}/.cache/themes` in each `config.mk`.

You'll need to provide *your own* `.h` theme template, (or just dont source themes and paste in whatever)

## Screenshots
------

**[dwm](http://github.com/MitchWeaver/suckless/tree/master/dwm)**:

![obsd_sepia_rice](https://i.imgur.com/C1zSkJM.png)

![obsd_sepia_rice2](https://i.imgur.com/ecCsAkD.png)

**[dmenu](http://github.com/MitchWeaver/suckless/tree/master/dwm)**:

![dmenu](https://i.imgur.com/1E9EsSB.png)

**[st / tabbed](http://github.com/MitchWeaver/suckless/tree/master/st)**:

![st / tabbed](http://0x0.st/iAql.png)

### See also
-------

[bin](http://github.com/mitchweaver/bin) - helpful scripts  
[dots](http://github.com/mitchweaver/dots) - dotfiles  
