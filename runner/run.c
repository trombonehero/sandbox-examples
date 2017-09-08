/*-
 * Copyright (c) 2017 Jonathan Anderson
 * All rights reserved.
 *
 * This software was developed at Memorial University under the
 * NSERC Discovery (RGPIN-2015-06048) and RDC Ignite (#5404.1822.101) programs.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/capsicum.h>

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;

static char* open_library_dirs(void);

int
main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <program> [args]\n", argv[0]);
		return 1;
	}

	const char *name = argv[1];

	int rtld = open("/libexec/ld-elf.so.1", O_RDONLY);
	if (rtld < 0)
		err(-1, "unable to open RTLD");

	int binary = open(name, O_RDONLY);
	if (binary < 0)
		err(-1, "unable to open binary '%s'", name);

	char *args[argc + 4];
	args[0] = strdup(name);
	args[1] = "-f";
	if (asprintf(args + 2, "%d", binary) < 0)
		err(-1, "failed to asprintf(%d)", binary);

	args[3] = "--";
	args[argc + 3] = NULL;

	for (int i = 0; i < argc - 1; i++) {
		args[i + 4] = argv[i + 1];
	}

	setenv("LD_LIBRARY_PATH_FDS", open_library_dirs(), 1);
	cap_enter();

	fexecve(rtld, args, environ);
	err(-1, "failed to exec '%s'", name);

	// unreachable
}


static char*
open_library_dirs()
{
	static const char* DEFAULT_LIBDIRS[] = {
		"/lib", "/usr/lib", "/usr/local/lib"
	};

	static size_t LIB_COUNT =
		sizeof(DEFAULT_LIBDIRS) / sizeof(*DEFAULT_LIBDIRS);

	char *buffer = malloc(MAXPATHLEN);
	char *str = buffer;
	size_t remaining = MAXPATHLEN;

	for (size_t i = 0; i < LIB_COUNT; i++) {
		int fd = open(DEFAULT_LIBDIRS[i], O_RDONLY);
		if (fd < 0)
			err(-1, "failed to open '%s'", DEFAULT_LIBDIRS[i]);

		int result = snprintf(str, remaining, "%d:", fd);
		if (result < 0)
			err(-1, "failed to print %d into libdir buffer", fd);

		// result is non-negative
		size_t len = (unsigned int) result;
		if (len >= remaining)
			err(-1, "failed to print %d into libdir buffer", fd);

		str += len;
		remaining -= len;
	}

	return buffer;
}
