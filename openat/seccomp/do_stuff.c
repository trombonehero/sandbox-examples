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

#define _POSIX_C_SOURCE	200809L

#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <linux/filter.h>
#include <linux/seccomp.h>
#include <linux/audit.h>

#include <err.h>
#include <fcntl.h>
#include <stddef.h>
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

	// Enter sandbox!
	// (the following was derived from https://eigenstate.org/notes/seccomp)
	#define ArchField offsetof(struct seccomp_data, arch)

	#define Allow(syscall) \
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, SYS_##syscall, 0, 1), \
		BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_ALLOW)

	static const size_t SYSCALL_NUM_OFFSET =
		offsetof(struct seccomp_data, nr);

	struct sock_filter filter[] = {
		// Check architecture: syscall numbers arch-dependent!
		BPF_STMT(BPF_LD+BPF_W+BPF_ABS, ArchField),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, AUDIT_ARCH_X86_64, 1, 0),
		BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_KILL),

		// Check syscall:
		BPF_STMT(BPF_LD+BPF_W+BPF_ABS, SYSCALL_NUM_OFFSET),
		Allow(brk),            // allow stack extension
		Allow(close),          // allow closing files!
		Allow(exit_group),     // called on exit(3)
		Allow(fstat),          // we need to check file sizes
		Allow(mmap),           // we map config files when reading
		Allow(munmap),         // we also unmap things
		Allow(openat),         // to permit openat(config_dir), etc.
		Allow(write),          // we write(2) to stdout
		BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_TRAP),      // or die!
	};
	struct sock_fprog filterprog = {
		.len = sizeof(filter)/sizeof(filter[0]),
		.filter = filter
	};

	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
		perror("Could not start seccomp:");
		return (1);
	}
	if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &filterprog) == -1) {		perror("Could not start seccomp:");
		return (1);
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
