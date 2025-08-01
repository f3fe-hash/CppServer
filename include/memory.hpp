#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <cstring> // For memcpy

#include <memory>
#include <iostream>

#define KiB 1024        // 1 KiB
#define MiB 1024 * KiB  // 1 MiB
#define GiB 1024 * MiB  // 1 GiB

class Allocator
{
public:
    Allocator(std::size_t size) __THROW;
    ~Allocator() __THROW;

    // Reallocate isn't necessary
    void* allocate(std::size_t size) __THROW;
    void  deallocate(void* ptr) __THROW __nonnull((1));
};

#endif
