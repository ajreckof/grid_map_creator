#include "mesh_preview_texture.h"

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport_texture.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/math.hpp>

#include "mesh_utility/mesh_utility.h"

using namespace godot;
using namespace Math;

MeshPreviewTexture::MeshPreviewTexture() : PreviewTexture() { 

    if (!Engine::get_singleton()->is_editor_hint()) {
        // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
        return;
    }
    // Créez une instance de MeshInstance3D pour le rendu
    mesh_instance = memnew(MeshInstance3D);
    viewport->add_child(mesh_instance);
}
MeshPreviewTexture::~MeshPreviewTexture() {}

void MeshPreviewTexture::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &MeshPreviewTexture::set_mesh);
    ClassDB::bind_method(D_METHOD("get_mesh"), &MeshPreviewTexture::get_mesh);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
}

void MeshPreviewTexture::set_mesh(const Ref<ArrayMesh> &p_mesh) {

    if (!Engine::get_singleton()->is_editor_hint()) {
        // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
        return;
    }
    if (get_mesh().is_valid()) {
        get_mesh()->disconnect("changed", callable_mp((PreviewTexture *)this, &PreviewTexture::request_update));
    }
    mesh_instance->set_mesh(p_mesh);
    if (p_mesh.is_valid()) {
        p_mesh->connect("changed", callable_mp((PreviewTexture *)this, &PreviewTexture::request_update));
    }
    request_update();
}

Ref<ArrayMesh> MeshPreviewTexture::get_mesh() const {

    if (!Engine::get_singleton()->is_editor_hint()) {
        // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
        return nullptr;
    }
    return mesh_instance->get_mesh();
}

void MeshPreviewTexture::_update_texture() {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    if (!mesh_instance->get_mesh().is_valid()) {
        return;
    }

    aabb = get_mesh()->get_aabb();
    if (!aabb.has_volume()) {
        print_line("MeshPreviewTexture::update_texture: Mesh has no volume creating a default AABB");
        aabb = AABB(aabb.position, Vector3(1, 1, 1));
    }

}