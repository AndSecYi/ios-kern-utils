/*
 * kmem.c - Read kernel memory and dump it to the console
 *
 * Copyright (c) 2014 Samuel Groß
 * Copyright (c) 2016-2017 Siguza
 */

#include <errno.h>              // errno
#include <stdbool.h>            // bool, true, false
#include <stdio.h>              // printf, fprintf
#include <stdlib.h>             // free, malloc, strtoull
#include <string.h>             // memset, strlen, strerror
#include <unistd.h>             // getopt, write, STDOUT_FILENO

#include <mach/mach_types.h>    // task_t
#include <mach/vm_types.h>      // vm_address_t, vm_size_t

#include "arch.h"               // ADDR
#include "common.h"             // kutil_hexdump
#include "libkern.h"            // KERNEL_TASK_OR_GTFO
#include "mach.h"               // kutil_task_read

static void print_usage(const char *self)
{
    fprintf(stderr, "Usage:\n"
                    "    %s [options] addr len\n"
                    "    %s [options] -f addr file\n"
                    "    %s [options] -w/-q addr 0x...\n"
                    "    %s [options] -x addr ...\n"
                    "\n"
                    "The first form prints, the other forms patch.\n"
                    "\n"
                    "Options:\n"
                    "    -b      Print output binary (rather than hex)\n"
                    "    -d      Debug mode (sleep between function calls, gives\n"
                    "            sshd time to deliver output before kernel panic)\n"
                    "    -f      Read patch from file\n"
                    "    -h      Print this help\n"
                    "    -p pid  Operate on a process other than the kernel\n"
                    "    -q      Patch uint64 from immediate\n"
                    "            (Requires addr to be 8-byte aligned)\n"
                    "    -s      Safe memory access (wire)\n"
                    "    -v      Verbose (debug output)\n"
                    "    -w      Patch uint32 from immediate\n"
                    "            (Requires addr to be 4-byte aligned)\n"
                    "    -x      Patch from immediate hex string\n"
                    "            (little endian, must have even amount of chars)\n"
                    , self, self, self, self);
}

static void too_few_args(const char *self)
{
    fprintf(stderr, "[!] Too few arguments\n");
    print_usage(self);
}

int main(int argc, char **argv)
{
    bool raw  = false, // print raw bytes instead of a hexdump
         wire = false;
    vm_address_t addr;
    vm_size_t size;
    char c, *end;

    while((c = getopt(argc, argv, "bhs")) != -1)
    {
        switch (c)
        {
            case 'b':
                raw = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 's':
                wire = true;
                break;
        }
    }

    if(argc < optind + 2)
    {
        too_few_args(argv[0]);
        return -1;
    }

    // addr
    errno = 0;
    addr = strtoull(argv[optind], &end, 0);
    if(argv[optind][0] == '\0' || end[0] != '\0' || errno != 0)
    {
        fprintf(stderr, "[!] Failed to parse \"%s\": %s\n", argv[optind], argv[optind][0] == '\0' ? "zero characters given" : strerror(errno));
        return -1;
    }

    // size
    errno = 0;
    size = strtoull(argv[optind + 1], &end, 0);
    if(argv[optind + 1][0] == '\0' || end[0] != '\0' || errno != 0)
    {
        fprintf(stderr, "[!] Failed to parse \"%s\": %s\n", argv[optind + 1], argv[optind + 1][0] == '\0' ? "zero characters given" : strerror(errno));
        return -1;
    }
    if(size == 0)
    {
        fprintf(stderr, "[!] Size must be > 0\n");
        return -1;
    }

    int retval = 0;
    task_t kernel_task;
    KERNEL_TASK_OR_GTFO(kernel_task);

    if(!raw)
    {
        fprintf(stderr, "[*] Reading " SIZE " bytes from 0x" ADDR "\n", size, addr);
    }
    unsigned char* buf = malloc(size);
    if(!buf)
    {
        fprintf(stderr, "[!] Failed to allocate buffer: %s\n", strerror(errno));
        retval = -1;
    }
    else
    {
        if(kutil_task_read(kernel_task, addr, size, buf, wire) != size)
        {
            fprintf(stderr, "[!] Failed to read memory\n");
            retval = -1;
        }
        else if(raw)
        {
            write(STDOUT_FILENO, buf, size);
        }
        else
        {
            kutil_hexdump(stdout, buf, size);
        }

        free(buf);
    }

    return retval;
}
