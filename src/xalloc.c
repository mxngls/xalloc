#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// debug
#define VARNAME(variable) (#variable)

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
        // Keep track of all allocated (block) pointers;
        void *p_arr[10] = {0};
        int   p_arr_idx = 0;

        // Test 1: Basic allocation and free
        char *src;
        char *dst;
        printf("Test 1: Basic allocation and free\n");
        src                = "Hello, World!";
        dst                = xalloc(strlen(src) + 1);
        p_arr[p_arr_idx++] = dst;

        if (dst == NULL) {
                printf("Allocation failed\n");
                return 1;
        }

        strcpy(dst, src);
        printf("Allocation of %s at address: %p\n", dst, (void *)src);

        printf("Free result: %s\n", xfree(dst) == 0 ? "Success" : "Failed");
        // -----------------------------------------------------------------------------------------

        // Test 2: Multiple allocations and frees
        printf("\nTest 2: Multiple allocations and frees\n");
        int *nums_first;
        int *nums_second;
        int *nums_third;

        nums_first  = xalloc(5 * sizeof(int));
        nums_second = xalloc(10 * sizeof(int));
        nums_third  = xalloc(15 * sizeof(int));

        p_arr[p_arr_idx++] = nums_first;
        p_arr[p_arr_idx++] = nums_second;
        p_arr[p_arr_idx++] = nums_third;

        printf("Allocation of integer array at address %p\n", (void *)nums_first);
        printf("Allocation of integer array at address %p\n", (void *)nums_second);
        printf("Allocation of integer array at address %p\n", (void *)nums_third);

        if (nums_first && nums_second && nums_third) {
                for (int i = 0; i < 5; i++)
                        nums_first[i] = i;
                for (int i = 0; i < 10; i++)
                        nums_second[i] = i * 10;
                for (int i = 0; i < 15; i++)
                        nums_third[i] = i * 100;

                printf("Freeing %s at address %s: %p\n", VARNAME(nums_second),
                       xfree(nums_second) == 0 ? "success" : "failed", (void *)nums_second);
                printf("Freeing %s at address %s: %p\n", VARNAME(nums_first),
                       xfree(nums_first) == 0 ? "success" : "failed", (void *)nums_first);
                printf("Freeing %s at address %s: %p\n", VARNAME(nums_third),
                       xfree(nums_third) == 0 ? "success" : "failed", (void *)nums_third);
        } else {
                printf("Some allocations failed\n");
        }
        // -----------------------------------------------------------------------------------------

        // Test 3: Reallocate after calling xfree
        printf("\nTest 3: Reallocation after free\n");
        char *buf;
        if ((buf = xalloc(50 * sizeof(char))) == NULL) {
                printf("Second allocation failed\n");
        }
        p_arr[p_arr_idx++] = buf;

        // reuse free block
        strcpy(buf, "Reused block");

        bool found = false;
        int  i     = 0;
        while (p_arr[i])
                if (p_arr[i++] == buf)
                        found = true;

        if (found == false)
                printf("Reuse of previously freed buffer failed!\n");
        else
                printf("Second allocation: \"%s\" at address %p\n", buf, (void *)buf);

        xfree(buf);
        // -----------------------------------------------------------------------------------------

        // Test 4: Invalid free (attempt to free stack memory)
        printf("\nTest 4: Invalid free\n");
        char local_buffer[20] = "Hello, World!";
        printf("Trying to free invalid pointer: %s\n",
               xfree(local_buffer) == -1 ? "correctly rejected" : "errounsly accepted");

        return 0;
}
