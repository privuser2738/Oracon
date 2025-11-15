#include "oracon/engine/world.h"

namespace oracon {
namespace engine {

World::World() {}

Entity* World::createEntity(const String& name) {
    auto entity = std::make_unique<Entity>(m_nextEntityId++, name);
    Entity* ptr = entity.get();
    m_entities.push_back(std::move(entity));
    return ptr;
}

void World::destroyEntity(Entity* entity) {
    m_entities.erase(
        std::remove_if(m_entities.begin(), m_entities.end(),
            [entity](const std::unique_ptr<Entity>& e) { return e.get() == entity; }),
        m_entities.end()
    );
}

Entity* World::findEntityByName(const String& name) {
    for (auto& entity : m_entities) {
        if (entity->getName() == name) {
            return entity.get();
        }
    }
    return nullptr;
}

void World::clear() {
    m_entities.clear();
}

} // namespace engine
} // namespace oracon
