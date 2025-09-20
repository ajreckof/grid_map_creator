#include "merged_mesh.h"
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/material.hpp>
#include "mesh_utility.h"

namespace godot {

void MergedMesh::set_source_meshes(const  TypedArray<ArrayMesh> &meshes) {
    if (source_meshes == meshes) {
        return;
    }
    if (source_meshes.size() > 0) {
        for (int i = 0; i < source_meshes.size(); ++i) {
            Ref<ArrayMesh> mesh = source_meshes[i];
            if (mesh.is_valid()) {
                mesh->disconnect("changed", callable_mp(this, &MergedMesh::request_update_merged_mesh));
            }
        }
    }
    source_meshes = meshes;
    if (source_meshes.size() > 0) {
        for (int i = 0; i < source_meshes.size(); ++i) {
            Ref<ArrayMesh> mesh = source_meshes[i];
            if (mesh.is_valid()) {
                mesh->connect("changed", callable_mp(this, &MergedMesh::request_update_merged_mesh));
            }
        }
    }
    request_update_merged_mesh();
}

void MergedMesh::add_source_mesh(const Ref<ArrayMesh> &mesh) {
    if (!mesh.is_valid()) {
        return;
    }
    source_meshes.push_back(mesh);
    mesh->connect("changed", callable_mp(this, &MergedMesh::request_update_merged_mesh));
    request_update_merged_mesh();
}

TypedArray<ArrayMesh> MergedMesh::get_source_meshes() const {
    return source_meshes;
}

void MergedMesh::request_update_merged_mesh() {
    if (!update_requested) {
        update_requested = true;
        callable_mp(this, &MergedMesh::update_merged_mesh).call_deferred();
    }
}

void MergedMesh::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_source_meshes", "meshes"), &MergedMesh::set_source_meshes);
    ClassDB::bind_method(D_METHOD("add_source_mesh", "mesh"), &MergedMesh::add_source_mesh);
    ClassDB::bind_method(D_METHOD("get_source_meshes"), &MergedMesh::get_source_meshes);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "source_meshes", PROPERTY_HINT_TYPE_STRING, "24/17:ArrayMesh"), "set_source_meshes", "get_source_meshes");
}

void MergedMesh::update_merged_mesh() {
    update_requested = false;
    merge_meshes_to_destination(source_meshes, this);
}

} // namespace godot
