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

#include <assert.h>
#include <stdbool.h>

#include <libexcite.h>


char
excite(char c)
{
	if (c >= 'a' && c <= 'z')
	{
		return c - ('a' - 'A');
	}

	switch (c)
	{
	case '.':
		return '!';

	case '?':
	case ',':
		return '.';

	default:
		return c;
	}
}


ssize_t
excite_file(int input, int output)
{
	char buffer[1024] = { 0 };
	ssize_t bytes_written = 0;

	while (true)
	{
		ssize_t result = read(input, buffer, sizeof(buffer) - 1);
		if (result < 0)
		{
			return (-1);
		}
		else if (result == 0)
		{
			// EOF.
			return bytes_written;
		}

		// `result` is a non-negative number of bytes
		size_t bytes = (size_t) result;
		for (size_t i = 0; i < bytes; i++)
		{
			buffer[i] = excite(buffer[i]);
		}

		result = write(output, buffer, bytes);
		if (result < 0)
		{
			return result;
		}
		else
		{
			assert(result > 0);
			bytes_written += result;
		}
	}
}
