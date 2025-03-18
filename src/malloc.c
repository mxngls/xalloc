#include "xalloc.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void *xalloc(size_t);
int   xfree(void *);

// see https://github.com/apple-oss-distributions/dyld/blob/main/include/mach-o/dyld-interposing.h
// for a more thorough approach see https://github.com/themattrix/musings?tab=readme-ov-file
#define DYLD_INTERPOSE(_replacment, _replacee)                                                     \
        __attribute__((used)) static struct {                                                      \
                const void *replacment;                                                            \
                const void *replacee;                                                              \
        } _interpose_##_replacee __attribute__((section("__DATA,__interpose"))) = {                \
            (const void *)(unsigned long)&_replacment, (const void *)(unsigned long)&_replacee};

DYLD_INTERPOSE(xalloc, malloc)
DYLD_INTERPOSE(xfree, free)
