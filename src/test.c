#include "xalloc.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// debug
#define VARNAME(variable) (#variable)

int main(void) {
        // keep track of all allocated (block) pointers;
        void *p_arr[10] = {0};
        int   p_arr_idx = 0;

        // Test 1: Basic allocation and free
        char *src;
        char *dst;
        printf("Test 1: Basic allocation and free\n");
        src                = "Hello, World!";
        dst                = xalloc(strlen(src) + 1);
        p_arr[p_arr_idx++] = dst;

        if (!dst)
                printf("Allocation failed\n");

        strcpy(dst, src);
        printf("Allocation of \"%s\" at address: %p\n", dst, (void *)src);
        printf("Freeing %s at address: %p%s successfull\n", VARNAME(dst), (void *)dst,
               xfree(dst) == 0 ? "" : " NOT");
        // -----------------------------------------------------------------------------------------

        // Test 2: Multiple allocations and frees
        printf("\nTest 2: Multiple allocations and frees\n");
        int *nums_first;
        int *nums_second;
        int *nums_third;

        nums_first  = xalloc(5 * sizeof(int));
        nums_second = xalloc(10 * sizeof(int));
        nums_third  = xalloc(15 * sizeof(int));

        printf("Allocation of integer array (%s) at address %p\n", VARNAME(nums_first),
               (void *)nums_first);
        printf("Allocation of integer array (%s) at address %p\n", VARNAME(nums_second),
               (void *)nums_second);
        printf("Allocation of integer array (%s) at address %p\n", VARNAME(nums_third),
               (void *)nums_third);
        printf("\n");

        if (nums_first && nums_second && nums_third) {
                for (int i = 0; i < 5; i++)
                        nums_first[i] = i;
                for (int i = 0; i < 10; i++)
                        nums_second[i] = i * 10;
                for (int i = 0; i < 15; i++)
                        nums_third[i] = i * 100;

                printf("Freeing %s at address %p%s successfull\n", VARNAME(nums_second),
                       (void *)nums_second, xfree(nums_second) == 0 ? "" : " NOT");
                printf("Freeing %s at address %p%s successfull\n", VARNAME(nums_first),
                       (void *)nums_first, xfree(nums_first) == 0 ? "" : " NOT");
                printf("Freeing %s at address %p%s successfull\n", VARNAME(nums_third),
                       (void *)nums_third, xfree(nums_third) == 0 ? "" : " NOT");
        } else {
                printf("Some allocations failed\n");
        }
        // -----------------------------------------------------------------------------------------

        // Test 3: Reallocate after calling xfree
        printf("\nTest 3: Reallocation after free\n");
        char *buf;
        if (!(buf = xalloc(50 * sizeof(char)))) {
                printf("Allocation failed\n");
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
                printf("Allocation of \"%s\" at address %p\n", buf, (void *)buf);

        printf("Freeing %s at address %p%s successfull\n", VARNAME(buf), (void *)buf,
               xfree(buf) == 0 ? "" : " NOT");
        // -----------------------------------------------------------------------------------------

        // Test 4: Prevent invalid freeing (attempt to free stack memory)
        printf("\nTest 4: Prevent invalid freeing\n");
        char local_buffer[20] = "Hello, World!";
        printf("Trying to free invalid pointer: %s\n",
               xfree(local_buffer) == -1 ? "correctly rejected" : "errounsly accepted");
        // -----------------------------------------------------------------------------------------

        // Test 5: Block merging (defragmentation)
        printf("\nTest 5: Block merging (defragmentation)\n");

        // allocate three blocks in a row
        int *first_buf  = xalloc(100 * sizeof(int));
        int *second_buf = xalloc(100 * sizeof(int));
        int *third_buf  = xalloc(100 * sizeof(int));

        printf("Allocated %s at address: %p\n", VARNAME(first_buf), (void *)first_buf);
        printf("Allocated %s at address: %p\n", VARNAME(second_buf), (void *)second_buf);
        printf("Allocated %s at address: %p\n", VARNAME(third_buf), (void *)third_buf);
        printf("\n");

        // free blocks to create adjacent free blocks
        printf("Freeing %s at address %p%s successfull\n", VARNAME(first_buf), (void *)first_buf,
               xfree(first_buf) == 0 ? "" : " NOT");
        printf("Freeing %s at address %p%s successfull\n", VARNAME(second_buf), (void *)second_buf,
               xfree(second_buf) == 0 ? "" : " NOT");
        printf("Freeing %s at address %p%s successfull\n", VARNAME(third_buf), (void *)third_buf,
               xfree(third_buf) == 0 ? "" : " NOT");

        int *new_buf = xalloc(180 * sizeof(int));

        if (!(new_buf <= first_buf &&
              ((char *)new_buf + 180 * sizeof(new_buf) > (char *)first_buf)))
                printf("Merging of previously freed block failed.\n");

        // free the remaining blocks
        printf("Freeing %s at address %p%s successfull\n", VARNAME(new_buf), (void *)new_buf,
               xfree(new_buf) == 0 ? "" : " NOT");
        // -----------------------------------------------------------------------------------------
}
