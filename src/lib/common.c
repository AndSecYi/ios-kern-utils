/*
 * common.c
 *
 * Copyright (c) 2014 Samuel Groß
 * Copyright (c) 2016-2017 Siguza
 */

#include <errno.h>              // errno, EINVAL
#include <stdbool.h>            // bool, false
#include <stdio.h>              // fprintf
#include <stdlib.h>             // strtoull
#include <string.h>             // memset

#include "common.h"

bool kutil_verbose = false;
bool kutil_slow = false;

int kutil_strtoull(const char *str, unsigned long long *result)
{
    if(str[0] == '\0')
    {
        return EINVAL;
    }
    errno = 0;
    char *end = NULL;
    *result = strtoull(str, &end, 0);
    return end[0] == '\0' ? 0 : errno;
}

void kutil_hexdump(FILE * restrict stream, const unsigned char *data, size_t size)
{
    int i;
    char cs[17];
    memset(cs, 0, 17);

    for(i = 0; i < size; i++)
    {
        if(i != 0 && i % 0x10 == 0)
        {
            fprintf(stream, " |%s|\n", cs);
            memset(cs, 0, 17);
        }
        else if(i != 0 && i % 0x8 == 0)
        {
            fprintf(stream, " ");
        }
        fprintf(stream, "%02X ", data[i]);
        cs[(i % 0x10)] = (data[i] >= 0x20 && data[i] <= 0x7e) ? data[i] : '.';
    }

    i = i % 0x10;
    if(i != 0)
    {
        if(i <= 0x8)
        {
            fprintf(stream, " ");
        }
        while(i++ < 0x10)
        {
            fprintf(stream, "   ");
        }
    }
    fprintf(stream, " |%s|\n", cs);
}
