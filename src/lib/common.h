/*
 * common.h - General-purpose stuff.
 *
 * Copyright (c) 2014 Samuel Groß
 * Copyright (c) 2016-2017 Siguza
 */

#ifndef KUTIL_COMMON_H
#define KUTIL_COMMON_H

#include <stdbool.h>            // bool
#include <stdio.h>              // fprintf, stderr
#include <unistd.h>             // usleep

#define BUGTRACKER_URL "https://github.com/Siguza/ios-kern-utils/issues/new"

#define DEBUG(str, args...) \
do \
{ \
    if(kutil_verbose) \
    { \
        fprintf(stderr, "[DEBUG] " str " [" __FILE__ ":%u]\n", ##args, __LINE__); \
    } \
    if(kutil_slow) \
    { \
        usleep(100); \
    } \
} while(0)

extern bool kutil_verbose;
extern bool kutil_slow;

/*
 * Iterate over all load commands in a Mach-O header
 */
#define CMD_ITERATE(hdr, cmd) \
for(struct load_command *cmd = (struct load_command *) ((hdr) + 1), \
                        *end = (struct load_command *) ((char *) cmd + (hdr)->sizeofcmds); \
    cmd < end; \
    cmd = (struct load_command *) ((char *) cmd + cmd->cmdsize))

// TODO: document
int kutil_strtoull(const char *str, unsigned long long *result);

void kutil_hexdump(FILE * restrict stream, const unsigned char *data, size_t size);

#endif
