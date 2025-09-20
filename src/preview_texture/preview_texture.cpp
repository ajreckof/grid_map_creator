#include "preview_texture.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/viewport_texture.hpp>
#include <godot_cpp/classes/control.hpp>

namespace godot {

PreviewTexture::PreviewTexture() {
    if (!Engine::get_singleton()->is_editor_hint()) {
        // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
        return;
    }
    viewport = memnew(SubViewport);
    viewport->set_clear_mode(SubViewport::CLEAR_MODE_ALWAYS);
    viewport->set_update_mode(SubViewport::UPDATE_DISABLED);
    viewport->set_use_own_world_3d(true);
    viewport->set_transparent_background(true);
    // ajouter le viewport a la scène
    EditorInterface::get_singleton()->get_base_control()->add_child(viewport);

    camera = memnew(Camera3D);
    camera->set_projection(Camera3D::PROJECTION_ORTHOGONAL);
    camera->set_keep_aspect_mode(Camera3D::KEEP_WIDTH);
    viewport->add_child(camera);
    set_rotation(Vector3(-Math_PI/2, 0, 0));
    set_debug_draw(Viewport::DEBUG_DRAW_UNSHADED);
    request_update();
}

PreviewTexture::~PreviewTexture() {}

void PreviewTexture::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_debug_draw", "debug_draw"), &PreviewTexture::set_debug_draw);
    ClassDB::bind_method(D_METHOD("get_debug_draw"), &PreviewTexture::get_debug_draw);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "debug_draw", PROPERTY_HINT_ENUM, "DISABLED,UNSHADED,LIGHTING,OVERDRAW,WIREFRAME,NORMAL_BUFFER,VOXEL_GI_ALBEDO,VOXEL_GI_LIGHT,VOXEL_GI_EMISSION, SHADOW_ATLAS,SHADOW_CUBEMAP,SHADOW_CUBEMAP_ARRAY,SHADOW_CUBEMAP_DEPTH,SHADOW_CUBEMAP_ARRAY_DEPTH,DEPTH_PREPASS,SSAO,SSIL,SSS,TRANSPARENT,FOG,GI,SCENE,FINAL"), "set_debug_draw", "get_debug_draw");
    
    ClassDB::bind_method(D_METHOD("set_rotation", "rotation"), &PreviewTexture::set_rotation);
    ClassDB::bind_method(D_METHOD("get_rotation"), &PreviewTexture::get_rotation);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "rotation", PROPERTY_HINT_RANGE, "-360,360,0.1,or_less,or_greater,radians_as_degrees"), "set_rotation", "get_rotation");
    
    ClassDB::bind_method(D_METHOD("set_width", "width"), &PreviewTexture::set_width);
    ClassDB::bind_method(D_METHOD("get_width"), &PreviewTexture::get_width);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "width"), "set_width", "get_width");
}

void PreviewTexture::set_rotation(const Vector3 &p_rotation) {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return;
    }
    rotation = p_rotation;
    request_update();
}

Vector3 PreviewTexture::get_rotation() const {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return Vector3();
    }
    return rotation;
}

Viewport::DebugDraw PreviewTexture::get_debug_draw() const {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return Viewport::DEBUG_DRAW_DISABLED;
    }
    return debug_draw;
}

void PreviewTexture::set_debug_draw(Viewport::DebugDraw p_debug_draw) {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return;
    }
    debug_draw = p_debug_draw;
    request_update();
}

void PreviewTexture::set_width(int p_width) {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return;
    }
    width = p_width;
    request_update();
}

int PreviewTexture::get_width() const {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return 0;
    }
    return width;
}

void PreviewTexture::request_update() {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return;
    }
    
    if (update_requested) {
        return;
    }
    
    update_requested = true;
    ready = false;
    print_line("PreviewTexture: update requested");
    
    // Use deferred call to ensure proper timing
    callable_mp(this, &PreviewTexture::update_texture).call_deferred();
}


void PreviewTexture::update_texture() {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return;
    }
    
    print_line("MeshPreviewTexture::update_texture");
    update_requested = false;

    // Set camera rotation from base class rotation property
    camera->set_rotation(rotation);
    
    // Set viewport debug draw from base class debug_draw property
    viewport->set_debug_draw(debug_draw);

    _update_texture();


    // Set debug draw mode
    viewport->set_debug_draw(debug_draw);
    
    // Calculate camera position and settings based on AABB
    Basis camera_basis = camera->get_global_transform().get_basis();
    Vector3 direction = camera_basis.get_column(Vector3::AXIS_Z);
    Vector3 width_vector = camera_basis.get_column(Vector3::AXIS_X);
    Vector3 height_vector = camera_basis.get_column(Vector3::AXIS_Y);
    

    if (!aabb.has_volume()) {
        print_line("render_node_to_image: AABB has no volume, creating a default AABB");
        aabb = AABB(aabb.position, Vector3(1, 1, 1));
    }
    
    AABB enlarged_aabb = aabb.merge(AABB(-aabb.get_end(), aabb.size));
    Vector3 camera_pos_near = aabb.get_support(direction).project(direction);
    Vector3 camera_pos_far = aabb.get_support(-direction).project(direction);
    
    camera->set_position(direction.normalized() * (camera_pos_near.length() + 1));
    camera->set_near(0.5f);
    camera->set_far((camera_pos_far - camera_pos_near).length() + 5);
    
    float mesh_width = enlarged_aabb.get_support(width_vector).project(width_vector).length() * 2;
    float mesh_height = enlarged_aabb.get_support(height_vector).project(height_vector).length() * 2;
    camera->set_size(mesh_width);

    viewport->set_size(Vector2i(width, ceilf(width * Math::snapped(mesh_height / mesh_width, 1e-3))));
    camera->force_update_transform();
    
    // Update viewport once
    viewport->set_update_mode(SubViewport::UPDATE_ONCE);

    RenderingServer::get_singleton()->connect("frame_post_draw", callable_mp(this, &PreviewTexture::assign_image), CONNECT_ONE_SHOT);
}




void PreviewTexture::assign_image() {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    
    
    print_line("PreviewTexture: assign_image");
    set_image(viewport->get_texture()->get_image());
    ready = true;
    emit_changed();
}

bool PreviewTexture::is_ready() const {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return false;
    }
    return ready;
}

void PreviewTexture::_validate_property(PropertyInfo &property) const {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return;
    }
    
    if (property.name == StringName("viewport_path")) {
        property.usage &= ~PROPERTY_USAGE_EDITOR;
    }
}

} // namespace godot