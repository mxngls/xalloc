#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct {
        union _block *next; // pointer to the next block
        size_t        size; // block size
        bool          free; // indicate if the block is currently in use
} _block_data;

typedef union _block {
        _block_data data;
        void       *_align; // primitive alignment (NOTE: fails for `long double`)
} block;

static block *headp = NULL; // point to start of linked list;

void *xalloc(size_t size) {
        block *curr_p;
        block *newp;
        block *prevp = NULL;
        size_t aligned_size;
        size_t block_size;

        // round size up to a multiple of sizeof(void *)
        aligned_size = (size + sizeof(void *) - 1) & ~(sizeof(void *) - 1);

        // look for free memory in already allocated blocks
        curr_p = headp;
        while (curr_p) {
                if (curr_p->data.size >= aligned_size && curr_p->data.free) {
                        // split if found block is significantly larger
                        // then the requested size
                        if (curr_p->data.size >= aligned_size + sizeof(block) + sizeof(void *)) {
                                // define a new block exactly `aligned_size` single bytes from the
                                // start of the current block
                                block *split = (block *)((char *)(curr_p + 1) + aligned_size);

                                split->data.size = curr_p->data.size - aligned_size - sizeof(block);
                                split->data.next = curr_p->data.next;
                                split->data.free = true;

                                // update original block
                                curr_p->data.size = aligned_size;
                                curr_p->data.next = split;
                        }

                        // mark original block as used and return it
                        curr_p->data.free = false;
                        return (void *)(curr_p + 1);
                }
                prevp  = curr_p;
                curr_p = curr_p->data.next;
        }

        // request more memory
        block_size = sizeof(block) + aligned_size;
        newp       = sbrk((int)block_size);
        if (newp == (void *)-1) {
                return NULL;
        }

        newp->data.size = aligned_size;
        newp->data.free = false;
        newp->data.next = NULL;

        if (!headp)
                headp = newp;
        else
                prevp->data.next = newp;

        return (void *)(newp + 1);
}

block *merge_free(block *b) {
        if (!b)
                return b;

        block *curr_p = b;

        // merge adjacent free blocks
        while (curr_p && curr_p->data.free) {
                block *next_p = curr_p->data.next;
                if (!next_p || !next_p->data.free)
                        break;
                curr_p->data.size += next_p->data.size + sizeof(block);
                curr_p->data.next = next_p->data.next;
        }
        return b;
}

int xfree(void *p) {
        if (!p)
                return -1;

        block *b = (block *)p - 1;

        bool   found  = false;
        block *prev_p = NULL;
        block *curr_p = headp;
        while (curr_p) {
                if (curr_p == b) {
                        found             = true;
                        curr_p->data.free = true;
                        if (prev_p && prev_p->data.free)
                                curr_p = merge_free(prev_p);
                        else
                                curr_p = merge_free(curr_p);
                        break;
                }
                prev_p = curr_p;
                curr_p = curr_p->data.next;
        }

        curr_p = headp;
        while (curr_p) {
                curr_p = curr_p->data.next;
        }

        if (!found)
                return -1;

        return 0;
}
