#include "oracon/core/memory.h"
#include <cstring>
#include <new>

namespace oracon {
namespace core {

void* allocate(usize size) {
    void* ptr = std::malloc(size);
    if (!ptr && size > 0) {
        throw std::bad_alloc();
    }
    return ptr;
}

void* reallocate(void* ptr, usize newSize) {
    void* newPtr = std::realloc(ptr, newSize);
    if (!newPtr && newSize > 0) {
        throw std::bad_alloc();
    }
    return newPtr;
}

void deallocate(void* ptr) {
    std::free(ptr);
}

// Arena implementation
Arena::Arena(usize capacity)
    : m_buffer(nullptr)
    , m_capacity(capacity)
    , m_used(0)
{
    m_buffer = static_cast<u8*>(allocate(capacity));
}

Arena::~Arena() {
    if (m_buffer) {
        deallocate(m_buffer);
    }
}

Arena::Arena(Arena&& other) noexcept
    : m_buffer(other.m_buffer)
    , m_capacity(other.m_capacity)
    , m_used(other.m_used)
{
    other.m_buffer = nullptr;
    other.m_capacity = 0;
    other.m_used = 0;
}

Arena& Arena::operator=(Arena&& other) noexcept {
    if (this != &other) {
        if (m_buffer) {
            deallocate(m_buffer);
        }

        m_buffer = other.m_buffer;
        m_capacity = other.m_capacity;
        m_used = other.m_used;

        other.m_buffer = nullptr;
        other.m_capacity = 0;
        other.m_used = 0;
    }
    return *this;
}

void* Arena::allocate(usize size, usize alignment) {
    // Align the current position
    usize alignedUsed = (m_used + alignment - 1) & ~(alignment - 1);

    if (alignedUsed + size > m_capacity) {
        return nullptr; // Out of memory
    }

    void* ptr = m_buffer + alignedUsed;
    m_used = alignedUsed + size;
    return ptr;
}

void Arena::reset() {
    m_used = 0;
}

} // namespace core
} // namespace oracon
