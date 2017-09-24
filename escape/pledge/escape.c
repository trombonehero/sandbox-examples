/*-
 * Copyright (c) 2017 Jonathan Anderson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

char **environ;


int main(int argc, char *argv[])
{
	printf("hello, world!\n");

	if (argc > 1)
	{
		const char *filename = argv[1];

		int fd = open(filename, O_RDONLY);
		if (fd < 0)
		{
			err(-1, "error opening '%s'", filename);
		}
		else
		{
			printf("successfully opened '%s'\n", filename);
			return 0;
		}
	}

	printf("entering pledge(2) restricted execution mode...");
	if (pledge("proc exec stdio", NULL) < 0)
	{
		err(-1, "error in pledge()");
	}
	printf(" done.\n");

	// We are in the parent process: try exec(2)'ing ourselves
	char *args[] = {
		argv[0],
		"/etc/passwd",
		NULL,
	};

	printf("exec()'ing ourselves... ");
	fflush(stdout);

	execve(argv[0], args, environ);
	err(-1, "error in execve()");

	return 0;
}
