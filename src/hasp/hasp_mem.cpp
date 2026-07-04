
/* MIT License - Copyright (c) 2019-2026 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include <stdlib.h>
#include "hasplib.h"
#include "hasp_mem.h"

// Definieer functie-pointers voor de allocators
static void* (*malloc_impl)(size_t) = malloc; 
static void* (*calloc_impl)(size_t, size_t) = calloc;
static void* (*realloc_impl)(void*, size_t) = realloc;

#ifdef ESP32
#include "esp_heap_caps.h"

bool hasp_use_psram()
{
    return psramFound() && ESP.getPsramSize() > 0;
}
#endif

void hasp_mem_setup()
{
#ifdef ESP32
    if (hasp_use_psram()) {
        LOG_INFO(TAG_HASP, F("PSRAM detected and will be used for memory allocation"));
        malloc_impl  = [](size_t size) { return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); };
        calloc_impl  = [](size_t num, size_t size) { return heap_caps_calloc(num, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); };
        realloc_impl = [](void* ptr, size_t size) { return heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); };
    } else {
        // Fallback to standard SRAM allocators
        malloc_impl  = malloc;
        calloc_impl  = calloc;
        realloc_impl = realloc;
    }
#endif
}

void* hasp_calloc(size_t num, size_t size) {
    return calloc_impl(num, size);
}

void* hasp_malloc(size_t size) {
    return malloc_impl(size);
}

void* hasp_realloc(void* ptr, size_t new_size) {
    return realloc_impl(ptr, new_size);
}

void hasp_free(void* ptr) {
    free(ptr); 
}

#ifdef LODEPNG_NO_COMPILE_ALLOCATORS
void* lodepng_malloc(size_t size)
{
#ifdef LODEPNG_MAX_ALLOC
    if(size > LODEPNG_MAX_ALLOC) return 0;
#endif

    // void* ptr = hasp_malloc(size);
    // if(ptr) return ptr;

    // /* PSram was full retry after clearing cache*/
    // lv_img_cache_invalidate_src(NULL);
    return hasp_malloc(size);
}

/* NOTE: when realloc returns NULL, it leaves the original memory untouched */
void* lodepng_realloc(void* ptr, size_t new_size)
{
#ifdef LODEPNG_MAX_ALLOC
    if(new_size > LODEPNG_MAX_ALLOC) return 0;
#endif

    return hasp_realloc(ptr, new_size);
}

void lodepng_free(void* ptr)
{
    hasp_free(ptr);
}
#endif // LODEPNG_NO_COMPILE_ALLOCATORS

HaspJsonAllocator haspJsonAllocator;
