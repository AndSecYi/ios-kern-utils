/*
 * mach.h - Mach API stuff.
 *
 * Copyright (c) 2017 Siguza
 */

#ifndef KUTIL_MACH_H
#define KUTIL_MACH_H

#include <mach/mach.h>

// TODO: document
mach_vm_size_t kutil_task_read(task_t task, mach_vm_address_t addr, mach_vm_size_t size, void *buf, bool wire);

mach_vm_size_t kutil_task_write(task_t task, mach_vm_address_t addr, mach_vm_size_t size, void *buf, bool wire);

#endif
