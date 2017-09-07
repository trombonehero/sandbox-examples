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

#include <sys/mman.h>
#include <sys/stat.h>

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libconfparse.h>


/**
 * Create a copy-on-write mapping of a file.
 *
 * @returns  a pointer to the memory or NULL on failure
 */
static char *map_file(int fd, size_t *len);
static bool parse_file(int dir, const char *filename,
	struct statement**, size_t *len, size_t *cap);


const struct config*
parse_config(int dir)
{
	struct statement *statements;
	size_t capacity, count;

	capacity = 16;
	count = 0;
	statements = calloc(capacity, sizeof(struct statement));

	if (!parse_file(dir, "root.conf", &statements, &count, &capacity))
	{
		goto fail;
	}

	struct config *c = malloc(sizeof(*c));
	c->length = count;
	c->statements = statements;
	return (c);

fail:
	free(statements);
	return (NULL);
}


static char *map_file(int fd, size_t *len)
{
	struct stat s;
	if (fstat(fd, &s) < 0)
	{
		return (NULL);
	}

	*len = s.st_size;

	void *data = mmap(NULL, *len, PROT_READ | PROT_WRITE, MAP_PRIVATE,
		fd, 0);
	if (data == MAP_FAILED)
	{
		return (NULL);
	}

	return data;
}


static bool parse_file(int dir, const char *filename,
	struct statement** spp, size_t *len, size_t *cap)
{
	static const char INCLUDE[] = "include";
	static const char MAKE_LOCKFILE[] = "lock";
	static const char PRINT[] = "print";

	int fd = openat(dir, filename, O_RDONLY | O_SHLOCK);
	if (fd < 0)
	{
		fprintf(stderr, "failed to open '%s': ", filename);
		perror("");
		return (NULL);
	}

	size_t file_len;
	char *data = map_file(fd, &file_len);
	close(fd);

	if (data == NULL)
	{
		perror("failed to mmap config file");
		close(fd);
		return (false);
	}

	bool success = true;
	for (char *s = data; s < data + file_len;)
	{
		// Do we need more space for statements?
		if (*len == *cap)
		{
			size_t new_size = 2 * *cap * sizeof(struct statement);
			*spp = realloc(*spp, new_size);
			if (*spp == NULL)
			{
				fprintf(stderr,
					"failed to realloc statements array\n");
				success = false;
				break;
			}

			*cap *= 2;
		}

		struct statement *statements = *spp;

		// Find the instruction.
		char *next = s;
		strsep(&next, " \t");
		if (next == NULL || *next == '\0')
		{
			// EOF
			if (s != data + file_len)
			{
				fprintf(stderr, "unexpected EOF: %s\n", s);
				success = false;
			}
			break;
		}

		const char *instruction_string = s;
		s = next;

		// Grab the argument.
		strsep(&next, "\n");
		if (next == NULL || *next == '\0')
		{
			// EOF
			if (s == next)
			{
				fprintf(stderr, "unexpected EOF: %s\n", s);
				success = false;
			}
			break;
		}
		const char *arg = s;
		s = next;

		// Parse the statement.
		// The `include` directive is pre-processed like C
		if (strcmp(INCLUDE, instruction_string) == 0)
		{
			if (!parse_file(dir, arg, spp, len, cap))
			{
				return false;
			}
			continue;
		}

		strlcpy(statements[*len].arg, arg,
			sizeof(statements[*len].arg));
		if (strcmp(MAKE_LOCKFILE, instruction_string) == 0)
		{
			statements[*len].instruction = make_lockfile;
		}
		else if (strcmp(PRINT, instruction_string) == 0)
		{
			statements[*len].instruction = print;
		}
		else
		{
			fprintf(stderr, "syntax error: %s\n",
				instruction_string);
			success = false;
			break;
		}
		(*len)++;
	}

	munmap(data, file_len);
	return (success);
}


bool
interpret_config(const struct config *c, int scratch_dir)
{
	for (size_t i = 0; i < c->length; i++)
	{
		const struct statement *s = c->statements + i;

		switch (s->instruction)
		{
		case make_lockfile:
			if (openat(scratch_dir, s->arg,
				O_CREAT | O_EXCL | O_EXLOCK) < 0)
			{
				fprintf(stderr, "failed to open '%s'", s->arg);
				perror(": ");
				return false;
			}
			break;
		case print:
			printf("%s\n", s->arg);
			break;
		}
	}

	return true;
}
