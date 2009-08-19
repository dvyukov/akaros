/* Copyright (c) 2009 The Regents of the University of California. 
 * See the COPYRIGHT files at the top of this source tree for full 
 * license information.
 * 
 * Kevin Klues <klueska@cs.berkeley.edu>    
 */

#ifndef KMALLOC_H
#define KMALLOC_H

#include <arch/types.h>

void  kmalloc_init();
void *(DALLOC(size) kmalloc)(size_t size, int flags);
void  DFREE(addr) kfree(void* addr);

#endif //KMALLOC_H