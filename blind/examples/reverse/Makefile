INPUT_VIDEO = <please select a video file as INPUT_VIDEO>
TEMPFILE = tmp.uivf

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

START = 20
END = 40

reversed.mkv: $(TEMPFILE) $(INPUT_VIDEO)
	framerate=$$(ffprobe -v quiet -show_streams -select_streams v - < "$(INPUT_VIDEO)" | \
	             grep '^r_frame_rate=' | cut -d = -f 2) && \
	../../blind-reverse $(TEMPFILE) | ../../blind-to-video $(DRAFT) $${framerate} $(FFMPEG_ARGS) reversed.mkv

$(TEMPFILE): $(INPUT_VIDEO)
	../../blind-from-video -L $(DRAFT) "$(INPUT_VIDEO)" - | \
	../../blind-split -L /dev/null $(START) $(TEMPFILE) $(END)

clean:
	-rm reversed.mkv  $(TEMPFILE)

.PHONY: clean
