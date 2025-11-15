#ifndef ORACON_ENGINE_ENTITY_H
#define ORACON_ENGINE_ENTITY_H

#include "oracon/engine/component.h"
#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace oracon {
namespace engine {

using core::u64;

// Entity - game object that holds components
class Entity {
public:
    explicit Entity(u64 id, const String& name = "Entity");
    ~Entity() = default;

    // Entity ID
    u64 getId() const { return m_id; }

    // Name
    const String& getName() const { return m_name; }
    void setName(const String& name) { m_name = name; }

    // Active state
    bool isActive() const { return m_active; }
    void setActive(bool active) { m_active = active; }

    // Component management
    template<typename T, typename... Args>
    T* addComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = component.get();

        std::type_index typeIndex(typeid(T));
        m_components[typeIndex] = std::move(component);

        return ptr;
    }

    template<typename T>
    T* getComponent() {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        std::type_index typeIndex(typeid(T));
        auto it = m_components.find(typeIndex);

        if (it != m_components.end()) {
            return static_cast<T*>(it->second.get());
        }

        return nullptr;
    }

    template<typename T>
    const T* getComponent() const {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        std::type_index typeIndex(typeid(T));
        auto it = m_components.find(typeIndex);

        if (it != m_components.end()) {
            return static_cast<const T*>(it->second.get());
        }

        return nullptr;
    }

    template<typename T>
    bool hasComponent() const {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        std::type_index typeIndex(typeid(T));
        return m_components.find(typeIndex) != m_components.end();
    }

    template<typename T>
    void removeComponent() {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        std::type_index typeIndex(typeid(T));
        m_components.erase(typeIndex);
    }

    // Get all components
    const std::unordered_map<std::type_index, std::unique_ptr<Component>>& getComponents() const {
        return m_components;
    }

private:
    u64 m_id;
    String m_name;
    bool m_active;
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
};

} // namespace engine
} // namespace oracon

#endif // ORACON_ENGINE_ENTITY_H
