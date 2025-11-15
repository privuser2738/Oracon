#ifndef ORACON_ENGINE_SCENE_H
#define ORACON_ENGINE_SCENE_H

#include "oracon/engine/world.h"
#include "oracon/engine/camera.h"

namespace oracon {
namespace engine {

class Scene {
public:
    Scene(const String& name);
    
    const String& getName() const { return m_name; }
    World* getWorld() { return &m_world; }
    Camera* getCamera() { return &m_camera; }

private:
    String m_name;
    World m_world;
    Camera m_camera;
};

} // namespace engine
} // namespace oracon

#endif
