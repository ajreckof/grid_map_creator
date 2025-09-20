#include "scene_preview_texture.h"

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport_texture.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/classes/visual_instance3d.hpp>

#include "mesh_utility.h"
#include "object_utility.h"

using namespace godot;
using namespace Math;

ScenePreviewTexture::ScenePreviewTexture() {
    // Initialization will be done when packed_scene is set
}

ScenePreviewTexture::~ScenePreviewTexture() {}

void ScenePreviewTexture::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_packed_scene", "scene"), &ScenePreviewTexture::set_packed_scene);
    ClassDB::bind_method(D_METHOD("get_packed_scene"), &ScenePreviewTexture::get_packed_scene);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "packed_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_packed_scene", "get_packed_scene");
}

void ScenePreviewTexture::set_packed_scene(const Ref<PackedScene> &p_scene) {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return;
    }
    
    if (packed_scene.is_valid()) {
        packed_scene->disconnect("changed", callable_mp(this, &ScenePreviewTexture::_packed_scene_changed));
    }
    
    packed_scene = p_scene;
    
    if (p_scene.is_valid()) {
        p_scene->connect("changed", callable_mp(this, &ScenePreviewTexture::_packed_scene_changed));
    }
    
    _packed_scene_changed();
}

Ref<PackedScene> ScenePreviewTexture::get_packed_scene() const {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return nullptr;
    }
    return packed_scene;
}

void ScenePreviewTexture::set_aabb(const AABB &p_aabb) {
    aabb = p_aabb;
    request_update();
}
AABB ScenePreviewTexture::get_aabb() const {
    return aabb;
}

void ScenePreviewTexture::_packed_scene_changed() {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    if (scene_instance) {
        scene_instance->queue_free();
    }
    if (!packed_scene.is_valid()){
        scene_instance = nullptr;
        set_image(Image::create_empty(width, width, false, Image::FORMAT_RGBA8));
        return;
    }
    scene_instance = packed_scene->instantiate();
    viewport->add_child(scene_instance);
    if (object_has_property(scene_instance, "actual_size")) {
        Vector3 actual_size = scene_instance->get("actual_size");
        aabb = AABB(-actual_size/2, actual_size);
    }
    if (object_has_property(scene_instance, "preview")) {
        scene_instance->set("preview", true);
    }

    request_update();
}

