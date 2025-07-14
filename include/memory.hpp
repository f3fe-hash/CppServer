#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <cstring> // For memcpy

#include <memory>
#include <iostream>

#define KiB 1024        // 1 KiB
#define MiB 1024 * KiB  // 1 MiB
#define GiB 1024 * MiB  // 1 GiB

#define byte long long int

#define ALIGNMENT 8

#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define HEADER_SIZE ALIGN(sizeof(MemoryBlock_))

class Allocator
{
    typedef struct MemoryBlock_
    {
        std::size_t size;
        struct MemoryBlock_* next;
    } MemoryBlock_;

    byte* raw;

    MemoryBlock_* free_list;
public:
    Allocator(std::size_t size);
    ~Allocator();

    void* allocate(std::size_t size);
    void* reallocate(void* ptr, std::size_t new_size);
    void  deallocate(void* ptr);
};

#endif
