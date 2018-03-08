INPUT_VIDEO = <please select a video file as INPUT_VIDEO>

SHELL = bash
# We need Bash's process substitution operator >()
# because we want to convert the files back to a
# cooked format, because raw takes a serious amount
# of space. It is of course also possible to use
# FIFO:s (if you know what you are doing).

DRAFT = -d
# Useful for better performance when not working
# with colours or not caring about colours.

FFMPEG_ARGS = -c:v libx264 -preset veryslow -crf 0 -pix_fmt yuv444p
#             ↑~~~~~~~~~~~ ↑~~~~~~~~~~~~~~~ ↑~~~~~~~~~~~~~~~~~~~~~~
#             │            │                │
#             │            │                └──── Lossless
#             │            │
#             │            └──── High compression
#             │
#             └──── H.264, a lossless-capable codec

FRAME_1 = 10
FRAME_2 = 20
FRAME_3 = 30
FRAME_4 = 40
FRAME_5 = end

1.mkv 2.mkv 3.mkv 4.mkv 5.mkv: $(INPUT_VIDEO)
	framerate=$$(ffprobe -v quiet -show_streams -select_streams v - < "$(INPUT_VIDEO)" | \
	             grep '^r_frame_rate=' | cut -d = -f 2) && \
	../../blind-from-video -L $(DRAFT) "$(INPUT_VIDEO)" - | \
	../../blind-split -L >(../../blind-to-video $(DRAFT) $${framerate} $(FFMPEG_ARGS) 1.mkv) $(FRAME_1) \
	                     >(../../blind-to-video $(DRAFT) $${framerate} $(FFMPEG_ARGS) 2.mkv) $(FRAME_2) \
	                     >(../../blind-to-video $(DRAFT) $${framerate} $(FFMPEG_ARGS) 3.mkv) $(FRAME_3) \
	                     >(../../blind-to-video $(DRAFT) $${framerate} $(FFMPEG_ARGS) 4.mkv) $(FRAME_4) \
	                     >(../../blind-to-video $(DRAFT) $${framerate} $(FFMPEG_ARGS) 5.mkv) $(FRAME_5)

clean:
	-rm 1.mkv 2.mkv 3.mkv 4.mkv 5.mkv

.PHONY: clean
