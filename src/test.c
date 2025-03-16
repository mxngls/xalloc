#include "xalloc.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// debug
#define VARNAME(variable) (#variable)

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
        printf("Allocation of \"%s\" at address: %p\n", dst, (void *)src);

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

        printf("Allocation of integer array (%s) at address %p\n", VARNAME(nums_first),
               (void *)nums_first);
        printf("Allocation of integer array (%s) at address %p\n", VARNAME(nums_second),
               (void *)nums_second);
        printf("Allocation of integer array (%s) at address %p\n", VARNAME(nums_third),
               (void *)nums_third);

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

        xfree(buf);
        // -----------------------------------------------------------------------------------------

        // Test 4: Invalid free (attempt to free stack memory)
        printf("\nTest 4: Invalid free\n");
        char local_buffer[20] = "Hello, World!";
        printf("Trying to free invalid pointer: %s\n",
               xfree(local_buffer) == -1 ? "correctly rejected" : "errounsly accepted");

        return 0;
}
