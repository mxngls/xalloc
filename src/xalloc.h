#ifndef XALLOC_H
#define XALLOC_H

#include <stddef.h>
/**
 * Allocates a block of memory of the specified size.
 *
 * @param size The number of bytes to allocate
 * @return A pointer to the allocated memory, or NULL if allocation fails
 */
void *xalloc(size_t size);

/**
 * Frees a previously allocated block of memory.
 *
 * @param p Pointer to the memory block to free
 * @return 0 on success, -1 on failure
 */
int xfree(void *p);

#endif // XALLOC_H
