#ifndef MESH_UTILITY_H
#define MESH_UTILITY_H

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh_data_tool.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/viewport_texture.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

namespace godot {


// Static utility functions for mesh operations
Ref<ArrayMesh> transform_mesh_to_destination(Ref<ArrayMesh> source_mesh, Ref<ArrayMesh> destination_mesh, const Transform3D &transform);
Ref<ArrayMesh> transform_mesh_to_destination(Ref<ArrayMesh> source_mesh, Ref<ArrayMesh> destination_mesh, const Vector3 &position, const Vector3 &size, const Vector3 &rotation_euler);


Array merge_two_surfaces(Array &surface_arrays_a, Ref<Material> &surface_material_a, Array &surface_arrays_b, Ref<Material> &surface_material_b, Ref<Material> &r_surface_material, BitField<Mesh::ArrayFormat> p_flags);
Array merge_surface_arrays(Vector<Array> &p_surface_arrays, Vector<Ref<Material>> &p_surface_materials, Ref<Material> &r_surface_material, BitField<Mesh::ArrayFormat> p_flags);
Ref<ArrayMesh> merge_meshes_to_destination(const TypedArray<ArrayMesh> &source_meshes, Ref<ArrayMesh> destination_mesh);


} // namespace godot

#endif // MESH_UTILITY_H
