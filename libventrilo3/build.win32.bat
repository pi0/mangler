gcc -DNO_AUTOMAKE -c -o libventrilo3.o libventrilo3.c
gcc -c -o ventrilo3_handshake.o ventrilo3_handshake.c
gcc -c -o libventrilo3_message.o libventrilo3_message.c
gcc -shared -o libventrilo3.dll libventrilo3.o ventrilo3_handshake.o libventrilo3_message.o -Wl,--out-implib,libmessage.a -lws2_32 -lpthread
gcc -o lv3_test.exe lv3_test.c libventrilo3.dll -lpthread