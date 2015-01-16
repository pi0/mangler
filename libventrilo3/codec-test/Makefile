
all:
	gcc -o pacat -g pacat.c  `pkg-config --libs libpulse-simple`
	gcc -o parec -g parec.c  `pkg-config --libs libpulse-simple`
	gcc -o speexdec -g speexdec.c -lspeex
	gcc -o speexenc -g speexenc.c -lspeex
	gcc -o gsmenc -g gsmenc.c -lgsm
	gcc -o gsmdec -g gsmdec.c -lgsm
	gcc -o gsmdec -g gsmdec.c -lgsm

clean:
	rm -f pacat
	rm -f parec
	rm -f speexdec
	rm -f speexenc
	rm -f gsmenc
	rm -f gsmdec
	rm -f gsmdec
