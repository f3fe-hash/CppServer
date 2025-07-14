#include "memory.hpp"

Allocator::Allocator(std::size_t size)
{
    if (size == 0) exit(0);
    this->raw = new byte[size];
    this->free_list = (MemoryBlock_ *)this->raw;
    this->free_list->size = size;
    this->free_list->next = NULL;
}

void* Allocator::allocate(std::size_t size)
{
    if (size == 0)
        return NULL;

    size_t total_size = ALIGN(size) + HEADER_SIZE;
    MemoryBlock_* prev = NULL;
    MemoryBlock_* curr = this->free_list;

    while (curr)
    {
        if (curr->size >= total_size)
        {
            // Can we split the block safely?
            size_t remaining = curr->size - total_size;

            if (remaining >= HEADER_SIZE + ALIGNMENT)
            {
                // Split: allocated part is the front, rest becomes a new block
                MemoryBlock_* next_block = (MemoryBlock_ *)((char *)curr + total_size);
                next_block->size = remaining;
                next_block->next = curr->next;

                if (prev)
                    prev->next = next_block;
                else
                    this->free_list = next_block;

                curr->size = total_size; // shrink current block to allocated size
            }
            else
            {
                // Can't split safely; give whole block
                if (prev)
                    prev->next = curr->next;
                else
                    this->free_list = curr->next;
            }

            return (void *)((char *)curr + HEADER_SIZE);
        }

        prev = curr;
        curr = curr->next;
    }

    return NULL; // No suitable block
}

void* Allocator::reallocate(void* ptr, size_t new_size)
{
    // Handle NULL pointer as allocation
    if (!ptr)
        return this->allocate(new_size);

    // Handle zero-size reallocation as free
    if (new_size == 0)
    {
        this->deallocate(ptr);
        return NULL;
    }

    // Get the block header of the current allocation
    MemoryBlock_* old_block = (MemoryBlock_ *)((char *)ptr - HEADER_SIZE);
    size_t old_data_size = old_block->size - HEADER_SIZE;

    // If the new size fits in the old block, reuse
    if (ALIGN(new_size) <= old_data_size)
        return ptr;

    // Allocate new block
    void* pNew = this->allocate(new_size);
    if (!pNew)
        return NULL;

    // Copy old data to new location
    std::memcpy(pNew, ptr, old_data_size);

    // Free old block
    this->deallocate(ptr);

    return pNew;
}


void Allocator::deallocate(void* ptr)
{
    if (!ptr)
        return;

    MemoryBlock_* block = (MemoryBlock_ *)((char *)ptr - HEADER_SIZE);
    block->next = this->free_list;
    this->free_list = block;
}
