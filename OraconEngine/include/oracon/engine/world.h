#ifndef ORACON_ENGINE_WORLD_H
#define ORACON_ENGINE_WORLD_H

#include "oracon/engine/entity.h"
#include <vector>
#include <memory>

namespace oracon {
namespace engine {

class World {
public:
    World();

    Entity* createEntity(const String& name = "Entity");
    void destroyEntity(Entity* entity);
    Entity* findEntityByName(const String& name);
    
    const std::vector<std::unique_ptr<Entity>>& getEntities() const { return m_entities; }
    
    void clear();

private:
    std::vector<std::unique_ptr<Entity>> m_entities;
    u64 m_nextEntityId = 1;
};

} // namespace engine
} // namespace oracon

#endif
