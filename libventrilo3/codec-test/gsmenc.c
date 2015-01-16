#include <gsm.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// ENCODER

int main() {
        gsm handle;
        char buf[65];
        gsm_signal sample[320];
        int cc, soundfd;
        int one = 1;

	if (!(handle = gsm_create())) {
	}
        gsm_option(handle, GSM_OPT_WAV49, &one);
	while (cc = fread(sample, sizeof sample, 1, stdin)) {
		if (cc != 1) {
                        fprintf(stderr, "fail read %d: %s\n", cc, strerror(errno));
			exit(0);
		}
		gsm_encode(handle, sample, buf);
		gsm_encode(handle,((short*)sample)+160,buf+32);
		if (fwrite((char *)buf, sizeof buf, 1, stdout) != 1) {
			fprintf(stderr, "fail write\n");
			exit(0);
		} else {
			fprintf(stderr, "wrote %lu bytes\n", sizeof buf);
		}
	}
	gsm_destroy(handle);
}
