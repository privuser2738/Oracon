#include "oracon/engine/entity.h"

namespace oracon {
namespace engine {

Entity::Entity(u64 id, const String& name)
    : m_id(id)
    , m_name(name)
    , m_active(true)
{}

} // namespace engine
} // namespace oracon
