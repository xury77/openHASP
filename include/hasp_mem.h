/* MIT License - Copyright (c) 2019-2026 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_MEM_H
#define HASP_MEM_H

#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LODEPNG_NO_COMPILE_ALLOCATORS
void* lodepng_calloc(size_t num, size_t size);
void* lodepng_malloc(size_t size);
void* lodepng_realloc(void* ptr, size_t new_size);
void lodepng_free(void* ptr);
#endif // LODEPNG_NO_COMPILE_ALLOCATORS

void hasp_mem_setup(void);
bool hasp_use_psram(void);
void* hasp_calloc(size_t num, size_t size);
void* hasp_malloc(size_t size);
void* hasp_realloc(void* ptr, size_t new_size);
void hasp_free(void* ptr);

#ifdef __cplusplus
}
#endif

// --- C++ SPECIFIEKE CODE (Alleen voor .cpp bestanden) ---
#ifdef __cplusplus
#include <ArduinoJson.h>

struct HaspJsonAllocator : ArduinoJson::Allocator {
    void* allocate(size_t size) override {
        return hasp_malloc(size); 
    }
    void deallocate(void* ptr) override {
        hasp_free(ptr);
    }
    void* reallocate(void* ptr, size_t new_size) override {
        return hasp_realloc(ptr, new_size);
    }
};

// We maken een globale instantie van de allocator beschikbaar
extern HaspJsonAllocator haspJsonAllocator;

#endif // __cplusplus

#endif // HASP_MEM_H