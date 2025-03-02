//
// Created by WiZ on 02/03/2025.
//

#include "common.h"

void* ARRAY_ALLOC(size_t num, size_t size) {
    void* ptr = calloc(num, size);
    if (!ptr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}