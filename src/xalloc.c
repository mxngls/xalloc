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

static block  head;
static block *headp = &head; // point to start of linked list;

void *xalloc(size_t size) {
        block *p;
        block *newp;
        block *prevp = NULL;
        size_t aligned_size;
        size_t block_size;

        // round size up to a multiple of sizeof(void *)
        aligned_size = (size + sizeof(void *) - 1) & ~(sizeof(void *) - 1);

        // look for free memory in already allocated blocks
        p = headp;
        while (p != NULL) {
                if (p->data.size >= aligned_size && p->data.free == true) {
                        // split if found block is significantly larger
                        // then the requested size
                        if (p->data.size >= aligned_size + sizeof(block) + sizeof(void *)) {
                                // define a new block exactly `aligned_size` single bytes from the
                                // start of the current block
                                block *split = (block *)((char *)(p + 1) + aligned_size);

                                split->data.size = p->data.size - aligned_size - sizeof(block);
                                split->data.next = p->data.next;
                                split->data.free = true;

                                // update original block
                                p->data.size = aligned_size;
                                p->data.next = split;
                        }

                        // mark original block as used and return it
                        p->data.free = false;
                        return (void *)(p + 1);
                }
                prevp = p;
                p     = p->data.next;
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

        if (headp == NULL)
                headp = newp;
        else
                prevp->data.next = newp;

        return (void *)(newp + 1);
}

int xfree(void *p) {
        if (p == NULL)
                return -1;

        block *b = (block *)p - 1;

        bool   found = false;
        block *cp    = headp;
        while (cp != NULL) {
                if (cp == b) {
                        found = true;
                        break;
                }
                cp = cp->data.next;
        }

        if (!found)
                return -1;

        b->data.free = true;

        return 0;
}

int main(void) {
        char *src = "Hello, World!";
        char *dst = xalloc(strlen(src));

        strncpy(dst, src, strlen(src));

        printf("%s\n", dst);
}
