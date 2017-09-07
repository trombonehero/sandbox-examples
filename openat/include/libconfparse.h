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

#include <stdbool.h>
#include <unistd.h>

/*
 * DISCLAIMER: the code in this library is intended to be representative of
 *             typical (terrible!!) patterns in extant software... it is
 *             intentionally vulnerable to buffer-overflow attacks so that
 *             we can demonstrate what a good idea sandboxing is
 */

__BEGIN_DECLS

struct statement;

/**
 * The configuration format for this program is, like so many config systems
 * unfortunately, a programming language.
 *
 * This structure captures a set of statements to be executed.
 */
struct config
{
	size_t			length;
	struct statement	*statements;
};

/**
 * Instructions that represent the verb in a statement.
 */
typedef enum instruction
{
	/** Create a lockfile in the scratch directory (arg: filename) */
	make_lockfile,

	/** Print something out (arg: string to print) */
	print,
} instruction_t;

/**
 * An action to execute: an instruction and an argument for that instruction.
 */
typedef struct statement
{
	instruction_t	instruction;
	const char	arg[16];
} statement_t;


/**
 * Parse a "configuration" file.
 *
 * @param    config_dir     read-only directory containing (at least) root.conf
 * @param    scratch_dir    writable scratch directory for this program
 *
 * @returns  a config object (caller responsible for freeing) or NULL on error
 */
const struct config* parse_config(int config_dir, int scratch_dir);

/**
 * Execute a "configuration".
 *
 * @param    c              configuration "data"
 * @param    scratch_dir    descriptor for scratch directory
 *
 * @returns  whether or not the interpretation was successful
 */
bool interpret_config(const struct config *c, int scratch_dir);

__END_DECLS
