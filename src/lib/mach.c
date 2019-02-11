/*
 * mach.c
 *
 * Copyright (c) 2014 Samuel Groß
 * Copyright (c) 2016-2017 Siguza
 */

#include <mach/mach.h>

#include "common.h"             // DEBUG
#include "mach.h"

extern kern_return_t mach_vm_read_overwrite(task_t task, mach_vm_address_t addr, mach_vm_size_t size, mach_vm_address_t data, mach_vm_size_t *outsize);
extern kern_return_t mach_vm_write(task_t task, mach_vm_address_t addr, mach_vm_address_t data, mach_msg_type_number_t dataCnt);

#if __LP64__
#   define ADDR "%016llx"
#else
#   define ADDR "%08llx"
#endif

// The vm_* APIs are part of the mach_vm subsystem, which is a MIG thing
// and therefore has a hard limit of 0x1000 bytes that it accepts. Due to
// this, we have to do both reading and writing in chunks smaller than that.
// Also, make sure we don't become unaligned.
#define MAX_CHUNK_SIZE 0xff0

mach_vm_size_t kutil_task_read(task_t task, mach_vm_address_t addr, mach_vm_size_t size, void *buf, bool wire)
{
    DEBUG("Reading task bytes " ADDR "-" ADDR, addr, addr + size);
    kern_return_t ret;
    mach_vm_size_t remainder = size,
                   bytes_read = 0;

    if(wire)
    {
        ret = mach_vm_wire(mach_host_self(), task, addr, size, VM_PROT_READ);
        if(ret != KERN_SUCCESS)
        {
            DEBUG("mach_vm_wire: %s", mach_error_string(ret));
            return 0;
        }
    }

    for(mach_vm_address_t end = addr + size; addr < end; remainder -= size)
    {
        size = remainder > MAX_CHUNK_SIZE ? MAX_CHUNK_SIZE : remainder;
        ret = mach_vm_read_overwrite(task, addr, size, (mach_vm_address_t)buf + bytes_read, &size);
        if(ret != KERN_SUCCESS || size == 0)
        {
            DEBUG("mach_vm_read: %s", mach_error_string(ret));
            break;
        }
        bytes_read += size;
        addr += size;
    }

    if(wire)
    {
        ret = mach_vm_wire(mach_host_self(), task, addr, size, VM_PROT_NONE);
        if(ret != KERN_SUCCESS)
        {
            DEBUG("mach_vm_unwire: %s", mach_error_string(ret));
            // Ignore unwiring errors
        }
    }

    return bytes_read;
}

mach_vm_size_t kutil_task_write(task_t task, mach_vm_address_t addr, mach_vm_size_t size, void *buf, bool wire)
{
    DEBUG("Writing to task at " ADDR "-" ADDR, addr, addr + size);
    kern_return_t ret;
    mach_vm_size_t remainder = size,
                   bytes_written = 0;

    if(wire)
    {
        ret = mach_vm_wire(mach_host_self(), task, addr, size, VM_PROT_WRITE);
        if(ret != KERN_SUCCESS)
        {
            DEBUG("mach_vm_wire: %s", mach_error_string(ret));
            return 0;
        }
    }

    for(mach_vm_address_t end = addr + size; addr < end; remainder -= size)
    {
        size = remainder > MAX_CHUNK_SIZE ? MAX_CHUNK_SIZE : remainder;
        ret = mach_vm_write(task, addr, (mach_vm_address_t)buf + bytes_written, size);
        if(ret != KERN_SUCCESS)
        {
            DEBUG("mach_vm_write: %s", mach_error_string(ret));
            break;
        }
        bytes_written += size;
        addr += size;
    }

    if(wire)
    {
        ret = mach_vm_wire(mach_host_self(), task, addr, size, VM_PROT_NONE);
        if(ret != KERN_SUCCESS)
        {
            DEBUG("mach_vm_unwire: %s", mach_error_string(ret));
            // Ignore unwiring errors
        }
    }

    return bytes_written;
}
