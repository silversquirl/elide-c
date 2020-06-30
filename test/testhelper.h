#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <stdio.h>
#include <unistd.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

static FILE *stropen(const char *s) {
	int fds[2];
	if (pipe(fds)) return NULL;

	FILE *rpipe = fdopen(fds[0], "r");
	FILE *wpipe = fdopen(fds[1], "w");
	if (!rpipe || !wpipe) {
		close(fds[0]);
		close(fds[1]);
		return NULL;
	}

	fputs(s, wpipe);
	fclose(wpipe);

	return rpipe;
}

#pragma GCC diagnostic pop

#endif
