
all:
	cd ../libventrilo3 && make dev
	gcc -pthread -Wall -g -o mangler-jukebox -I../libventrilo3/ -I.. \
		`if pkg-config vorbisfile; then echo -n "-DHAVE_VORBIS"; fi` \
		`if pkg-config flac; then echo -n "-DHAVE_FLAC"; fi` \
		jukebox.c \
		../libventrilo3/libventrilo3.a \
		-lgsm \
		-lspeex \
		`if grep -q "HAVE_OPUS 1" ../config.h; then pkg-config --cflags --libs opus; fi` \
		-lmpg123 \
		-lspeexdsp \
		`if pkg-config vorbisfile; then echo -n "-lvorbisfile"; fi` \
		`if pkg-config flac; then echo -n "-lFLAC"; fi` \
		-lm

