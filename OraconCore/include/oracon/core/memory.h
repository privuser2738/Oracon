#ifndef ORACON_CORE_MEMORY_H
#define ORACON_CORE_MEMORY_H

#include "types.h"
#include <memory>
#include <cstdlib>

namespace oracon {
namespace core {

// Memory allocation functions
void* allocate(usize size);
void* reallocate(void* ptr, usize newSize);
void deallocate(void* ptr);

// Arena allocator for fast temporary allocations
class Arena {
public:
    explicit Arena(usize capacity = 1024 * 1024); // 1MB default
    ~Arena();

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
    Arena(Arena&&) noexcept;
    Arena& operator=(Arena&&) noexcept;

    void* allocate(usize size, usize alignment = alignof(std::max_align_t));
    void reset();
    usize getUsed() const { return m_used; }
    usize getCapacity() const { return m_capacity; }

private:
    u8* m_buffer;
    usize m_capacity;
    usize m_used;
};

// Memory pool for fixed-size allocations
template<typename T, usize PoolSize = 256>
class Pool {
public:
    Pool() : m_nextFree(nullptr) {
        for (usize i = 0; i < PoolSize; ++i) {
            Node* node = reinterpret_cast<Node*>(&m_storage[i]);
            node->next = m_nextFree;
            m_nextFree = node;
        }
    }

    ~Pool() {
        // Objects must be manually destroyed before pool destruction
    }

    T* allocate() {
        if (!m_nextFree) {
            return nullptr;
        }
        Node* node = m_nextFree;
        m_nextFree = node->next;
        return reinterpret_cast<T*>(node);
    }

    void deallocate(T* ptr) {
        if (!ptr) return;
        Node* node = reinterpret_cast<Node*>(ptr);
        node->next = m_nextFree;
        m_nextFree = node;
    }

private:
    union Node {
        alignas(T) u8 data[sizeof(T)];
        Node* next;
    };

    alignas(T) u8 m_storage[PoolSize][sizeof(T)];
    Node* m_nextFree;
};

// Helper to create unique pointers with custom deleters
template<typename T, typename... Args>
UniquePtr<T> makeUnique(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

} // namespace core
} // namespace oracon

#endif // ORACON_CORE_MEMORY_H
