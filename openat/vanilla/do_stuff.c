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

#include <libconfparse.h>


int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage:  do_stuff <config dir> <scratch dir>\n");
		return (1);
	}

	// Open read-only config directory
	int config_dir = open(argv[1], O_DIRECTORY);
	if (config_dir < 0)
	{
		err(-1, "error opening config dir '%s'", argv[1]);
	}

	// Open writable scratch directory
	int scratch_dir = open(argv[2], O_DIRECTORY);
	if (scratch_dir < 0)
	{
		err(-1, "error opening scratch dir '%s'", argv[1]);
	}

	// Parse config file(s)
	const struct config *conf = parse_config(config_dir);
	if (conf == NULL)
	{
		errx(-1, "error parsing configuration file(s)");
	}

	// Interpret "configuration"
	if (!interpret_config(conf, scratch_dir))
	{
		errx(-1, "error interpreting configuration file");
	}

	return 0;
}
