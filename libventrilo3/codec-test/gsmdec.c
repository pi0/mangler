#include <gsm.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// DECODER

int main() {
	gsm handle;
	char buf[65];
	gsm_signal sample[320];
	int cc, soundfd;
	int one = 1;

	if (!(handle = gsm_create())) {
		fprintf(stderr, "fail handle\n");
		exit(0);
	}
	gsm_option(handle, GSM_OPT_WAV49, &one);
	while (cc = fread((char *)buf, sizeof buf, 1, stdin)) {
		if (cc != 1) {
			fprintf(stderr, "fail read %d: %s\n", cc, strerror(errno));
			exit(0);
		} else {
			fprintf(stderr, "read %lu bytes\n", sizeof buf);
		}
		if (gsm_decode(handle, buf, sample) || gsm_decode(handle, buf+33, ((short *)sample)+160)) {
			fprintf(stderr, "fail decode 1\n");
			exit(0);
		}
		if (fwrite(sample, sizeof sample, 1, stdout) != 1) {
			fprintf(stderr, "fail write\n");
			exit(0);
		}
	}
	gsm_destroy(handle);
}
