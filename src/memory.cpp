#include "memory.hpp"

Allocator::Allocator(std::size_t size) __THROW
{
    if (size == 0) exit(0);
    //this->raw = new byte[size];
    //this->free_list = (MemoryBlock_ *)this->raw;
    //this->free_list->size = size;
    //this->free_list->next = NULL;
}

Allocator::~Allocator() __THROW
{
    delete[] this->raw;
}

// Can't figure out how to use freeblock logic. Use standard allocation and call it a day
void* Allocator::allocate(std::size_t size) __THROW
{
    return malloc(size);
}

// Can't figure out how to use freeblock logic. Use standard deallocation and call it a day
void Allocator::deallocate(void* ptr) __THROW
{
    free(ptr);
}
