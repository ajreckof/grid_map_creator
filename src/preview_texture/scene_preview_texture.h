#ifndef SCENE_PREVIEW_TEXTURE_H
#define SCENE_PREVIEW_TEXTURE_H

#include "preview_texture.h"
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/node3d.hpp>

namespace godot {
class ScenePreviewTexture : public PreviewTexture {
    GDCLASS(ScenePreviewTexture, PreviewTexture);

private:
    Ref<PackedScene> packed_scene;
    Node *scene_instance;
    
protected:
    static void _bind_methods();

public:
    ScenePreviewTexture();
    ~ScenePreviewTexture();

    void set_packed_scene(const Ref<PackedScene> &p_scene);
    Ref<PackedScene> get_packed_scene() const;

    void set_aabb(const AABB &p_aabb);
    AABB get_aabb() const;

    void _update_texture() override {};
    void _packed_scene_changed();
};
}

#endif // SCENE_PREVIEW_TEXTURE_H
