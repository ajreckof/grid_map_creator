#include "transformed_mesh.h"
#include <godot_cpp/classes/mesh_data_tool.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/material.hpp>
#include "mesh_utility.h"

namespace godot {



void TransformedMesh::request_update_transformed_mesh() {
    if (!update_requested) {
        update_requested = true;
        callable_mp(this, &TransformedMesh::update_transformed_mesh).call_deferred();
    }
}

void TransformedMesh::set_source_mesh(const Ref<ArrayMesh> &mesh) {
    if (source_mesh == mesh) {
        return;
    }
    if (source_mesh.is_valid()) {
        source_mesh->disconnect("changed", callable_mp(this, &TransformedMesh::request_update_transformed_mesh));
    }
    source_mesh = mesh;
    if (source_mesh.is_valid()) {
        source_mesh->connect("changed", callable_mp(this, &TransformedMesh::request_update_transformed_mesh));
    }
    request_update_transformed_mesh();
}

Ref<ArrayMesh> TransformedMesh::get_source_mesh() const {
    return source_mesh;
}

void TransformedMesh::set_transform(const Transform3D &p_transform) {
    transform = p_transform;
    request_update_transformed_mesh();
}

Transform3D TransformedMesh::get_transform() const {
    return transform;
}

void TransformedMesh::set_position(const Vector3 &p_position) {
    transform.origin = p_position;
    request_update_transformed_mesh();
}

Vector3 TransformedMesh::get_position() const {
    return transform.origin;
}

void TransformedMesh::set_rotation(const Vector3 &p_rotation, EulerOrder p_order) {
    transform.basis.set_euler(p_rotation, p_order);
    request_update_transformed_mesh();
}

Vector3 TransformedMesh::get_rotation(EulerOrder p_order) const {
    return transform.basis.get_euler(p_order);
}

void TransformedMesh::set_size(const Vector3 &p_size) {
    Vector3 current_size = get_size();
    float scale = MAX(
        p_size.x / current_size.x,
        MAX(
            p_size.y / current_size.y,
            p_size.z / current_size.z
        )
    );
    transform.basis.scale(Vector3(
        scale,
        scale,
        scale
    ));
    request_update_transformed_mesh();
}

Vector3 TransformedMesh::get_size() const {
    return get_aabb().size;
}



void TransformedMesh::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_source_mesh", "mesh"), &TransformedMesh::set_source_mesh);
    ClassDB::bind_method(D_METHOD("get_source_mesh"), &TransformedMesh::get_source_mesh);
    ClassDB::bind_method(D_METHOD("set_transform", "transform"), &TransformedMesh::set_transform);
    ClassDB::bind_method(D_METHOD("get_transform"), &TransformedMesh::get_transform);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "source_mesh", PROPERTY_HINT_RESOURCE_TYPE, "ArrayMesh"), "set_source_mesh", "get_source_mesh");
    ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM3D, "transform"), "set_transform", "get_transform");
}

void TransformedMesh::update_transformed_mesh() {
    update_requested = false;
    
    if (!source_mesh.is_valid()) {
        print_error("No source mesh set.");
        return;
    }
    
    transform_mesh_to_destination(source_mesh, this, transform);
    emit_changed();
}

} // namespace godot
