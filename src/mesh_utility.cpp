#include "mesh_utility.h"
#include <godot_cpp/classes/mesh_data_tool.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/control.hpp>

namespace godot {

Ref<ArrayMesh> transform_mesh_to_destination(Ref<ArrayMesh> source_mesh, Ref<ArrayMesh> destination_mesh, const Transform3D &transform) {
    ERR_FAIL_COND_V_MSG(!source_mesh.is_valid(), nullptr, "Source mesh is not valid.");

    if(!destination_mesh.is_valid()) {
        destination_mesh = (Ref<ArrayMesh>)memnew(ArrayMesh);
    }
    destination_mesh->clear_surfaces();
    if(source_mesh->get_surface_count() == 0){
        print_line("Source mesh has no surfaces to transform. Returning empty destination mesh.");
    }
    
    for (int s = 0; s < source_mesh->get_surface_count(); s++) {
        Ref<MeshDataTool> mesh_data_tool = memnew(MeshDataTool);
        mesh_data_tool->create_from_surface(source_mesh, s);
        
        // collect all face normals and tangents
        Vector<Vector3> face_normals;
        for (int i = 0; i < mesh_data_tool->get_face_count(); i++) {
            face_normals.push_back(mesh_data_tool->get_face_normal(i));
        }

        for (int i = 0; i < mesh_data_tool->get_vertex_count(); i++) {
            mesh_data_tool->set_vertex(i, transform.xform(mesh_data_tool->get_vertex(i)));
            mesh_data_tool->set_vertex_normal(i, transform.basis.xform(mesh_data_tool->get_vertex_normal(i)).normalized());
            Plane tangent = mesh_data_tool->get_vertex_tangent(i);
            Vector3 center = tangent.get_center();
            tangent = Plane(
                transform.basis.xform(tangent.normal).normalized(),
                transform.xform(center)
            );
            mesh_data_tool->set_vertex_tangent(i, tangent);
        }

        mesh_data_tool->commit_to_surface(destination_mesh);

        if (transform.basis.determinant() < 0) {
            Array arrays = destination_mesh->surface_get_arrays(s);
            if (arrays.size() == ArrayMesh::ARRAY_MAX) {
                // If the mesh has indices, we need to swap indices for the symmetry
                PackedInt32Array indices = arrays[ArrayMesh::ARRAY_INDEX];
                int temp = 0;
                for (int i = 0; i < indices.size(); i += 3) {
                    temp = indices[i + 1];
                    indices[i + 1] = indices[i + 2];
                    indices[i + 2] = temp;
                }
                arrays[ArrayMesh::ARRAY_INDEX] = indices;

            }
            // Set the transformed arrays back to the surface
            BitField<Mesh::ArrayFormat> format = destination_mesh->surface_get_format(s);
            Mesh::PrimitiveType type = destination_mesh->surface_get_primitive_type(s);
            destination_mesh->surface_remove(s);
            destination_mesh->add_surface_from_arrays(type, arrays, {}, {}, format);
        }
            
        Ref<Material> mat = source_mesh->surface_get_material(s);
        if (mat.is_valid()) {
            destination_mesh->surface_set_material(s, mat);
        }
    }
    return destination_mesh;
}

Ref<ArrayMesh> transform_mesh_to_destination(Ref<ArrayMesh> source_mesh, Ref<ArrayMesh> destination_mesh, const Vector3 &p_position, const Vector3 &p_size, const Vector3 &p_rotation_euler) {
    ERR_FAIL_COND_V(!source_mesh.is_valid(), nullptr);
    

    if(!destination_mesh.is_valid()) {
        destination_mesh = (Ref<ArrayMesh>)memnew(ArrayMesh);
    }
    // Create transform from components
    Transform3D transform;

    Vector3 source_size = source_mesh->get_aabb().size;
    float scale = MAX(
        p_size.x / source_size.x,
        MAX(
            p_size.y / source_size.y,
            p_size.z / source_size.z
        )
    );
    transform.basis.scale(Vector3(scale, scale, scale));
    transform.basis.set_euler(p_rotation_euler);
    Vector3 position = source_mesh->get_aabb().position;
    transform.origin = p_position - position;
    
    // Call the existing transform function
    transform_mesh_to_destination(source_mesh, destination_mesh, transform);
    return destination_mesh;
}


Ref<ArrayMesh> merge_meshes_to_destination(const TypedArray<ArrayMesh> &source_meshes, Ref<ArrayMesh> destination_mesh) {

    if(!destination_mesh.is_valid()) {
        destination_mesh = (Ref<ArrayMesh>)memnew(ArrayMesh);
    }
    
    Vector<Ref<ArrayMesh>> meshes;
    for (int i = 0; i < source_meshes.size(); i++) {
        Ref<ArrayMesh> mesh = source_meshes[i];
        if (mesh.is_valid()) {
            meshes.push_back(mesh);
        }
    }
    destination_mesh->clear_surfaces();

    if (meshes.size() == 0) {
        return destination_mesh;
    }

    HashMap<int, long> surface_formats;
    Ref<ArrayMesh> first_mesh = meshes[0];
    int max_surfaces = first_mesh->get_surface_count();
    for (int s = 0; s < max_surfaces; ++s) {
        BitField<Mesh::ArrayFormat> format = first_mesh->surface_get_format(s);
        surface_formats[s] = format;
    }
    // Find the maximum number of surfaces across all meshes
    for (int m = 0; m < meshes.size(); ++m) {
        Ref<ArrayMesh> mesh = meshes[m];
        if (mesh.is_valid()) {
            max_surfaces = MAX(max_surfaces, mesh->get_surface_count());
        }
        for (int s = 0; s < mesh->get_surface_count(); ++s) {
            BitField<Mesh::ArrayFormat> format = mesh->surface_get_format(s);
            if (surface_formats.has(s)) {
                ERR_FAIL_COND_V_MSG(surface_formats[s] != format, destination_mesh, "Incompatible surface formats for surface index " + String::num_int64(s) + " out of " + String::num_int64(max_surfaces) + ". All meshes must have the same format for each surface index. Found " + String::num_int64(surface_formats[s]) + " and " + String::num_int64(format) + ".");
            } else {
                surface_formats[s] = format;
            }
        }
    }
    HashMap<int, String> array_type_names = {
        {Mesh::ARRAY_VERTEX, "VERTEX"},
        {Mesh::ARRAY_NORMAL, "NORMAL"},
        {Mesh::ARRAY_TANGENT, "TANGENT"},
        {Mesh::ARRAY_COLOR, "COLOR"},
        {Mesh::ARRAY_TEX_UV, "TEX_UV"},
        {Mesh::ARRAY_TEX_UV2, "TEX_UV2"},
        {Mesh::ARRAY_CUSTOM0, "CUSTOM0"},
        {Mesh::ARRAY_CUSTOM1, "CUSTOM1"},
        {Mesh::ARRAY_CUSTOM2, "CUSTOM2"},
        {Mesh::ARRAY_CUSTOM3, "CUSTOM3"},
        {Mesh::ARRAY_BONES, "BONES"},
        {Mesh::ARRAY_WEIGHTS, "WEIGHTS"},
        {Mesh::ARRAY_INDEX, "INDEX"}
    };
    // Merge each surface index separately
    for (int s = 0; s < max_surfaces; ++s) {
        Vector<Array> surface_arrays_to_merge;
        Vector<Ref<Material>> surface_materials_to_merge;
        
        // Collect all arrays for this surface index from all meshes
        for (int m = 0; m < meshes.size(); ++m) {
            Ref<ArrayMesh> mesh = meshes[m];
            if (!mesh.is_valid() || s >= mesh->get_surface_count()) continue;
            
            Array arrays = mesh->surface_get_arrays(s);
            if (arrays.size() > 0) {
                surface_arrays_to_merge.push_back(arrays);
                surface_materials_to_merge.push_back(mesh->surface_get_material(s));
            }
        }
        
        // If we have arrays to merge for this surface
        if (surface_arrays_to_merge.size() > 0) {
            Ref<Material> surface_material;
            Array merged_arrays = merge_surface_arrays(
                surface_arrays_to_merge, 
                surface_materials_to_merge, 
                surface_material,
                surface_formats[s]
            );
            for (int i = 0; i < merged_arrays.size(); i++) {
                print_line("Merged array " + array_type_names[i] + ": " + String(merged_arrays[i].call("size")));
                if ((int)merged_arrays[i].call("size") == 0) {
                    merged_arrays[i] = nullptr; // Set empty arrays to nullptr
                }
            }
            destination_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, merged_arrays, {}, {}, surface_formats[s]);
            
            if (surface_material.is_valid()) {
                destination_mesh->surface_set_material(s, surface_material);
            }
        }
    }
    
    return destination_mesh;
}


Array merge_surface_arrays(Vector<Array> &p_surface_arrays, Vector<Ref<Material>> &p_surface_materials, Ref<Material> &r_surface_material, BitField<Mesh::ArrayFormat> p_flags) {
    if (p_surface_arrays.size() == 0) {
        return Array();
    }
    if (p_surface_arrays.size() != p_surface_materials.size()) {
        ERR_FAIL_V_MSG(Array(), "Surface arrays and materials must have the same size.");
    }
    
    if (p_surface_arrays.size() == 1) {
        r_surface_material = p_surface_materials[0];
        return p_surface_arrays[0];
    }

    Array merged_arrays = p_surface_arrays[0];
    r_surface_material = p_surface_materials[0];
    for (int i = 1; i < p_surface_arrays.size(); i++) {
        merged_arrays = merge_two_surfaces(
            merged_arrays, 
            r_surface_material, 
            const_cast<Array &>(p_surface_arrays[i]), 
            const_cast<Ref<Material> &>(p_surface_materials[i]), 
            r_surface_material,
            p_flags
        );
    }
    return merged_arrays;
}


Array merge_two_surfaces(Array &surface_arrays_a, Ref<Material> &surface_material_a, Array &surface_arrays_b, Ref<Material> &surface_material_b, Ref<Material> &r_surface_material, BitField<Mesh::ArrayFormat> p_flags) {
    Array merged_arrays;
    merged_arrays.resize(ArrayMesh::ARRAY_MAX);
    
    // Initialize arrays
    PackedInt32Array indices;
    PackedFloat32Array weights;
    PackedInt32Array bones;
    Array custom3;
    Array custom2;
    Array custom1;
    Array custom0;
    PackedVector2Array uv2s;
    PackedVector2Array uvs;
    PackedColorArray colors;
    PackedFloat32Array tangents;
    PackedVector3Array normals;
    PackedVector3Array vertices;

    int vertex_offset = ((PackedInt32Array)surface_arrays_a[ArrayMesh::ARRAY_VERTEX]).size();
    int full_size = vertex_offset + ((PackedInt32Array)surface_arrays_b[ArrayMesh::ARRAY_VERTEX]).size();
    bool empty_a;
    bool empty_b;

    switch (MAX(surface_arrays_a.size(), surface_arrays_b.size()) - 1) {
        case Mesh::ARRAY_INDEX:
            empty_a = surface_arrays_a[ArrayMesh::ARRAY_INDEX].get_type() == Variant::NIL || (int)surface_arrays_a[ArrayMesh::ARRAY_INDEX].call(StringName("size")) == 0;
            empty_b = surface_arrays_b[ArrayMesh::ARRAY_INDEX].get_type() == Variant::NIL || (int)surface_arrays_b[ArrayMesh::ARRAY_INDEX].call(StringName("size")) == 0;
            ERR_FAIL_COND_V_MSG(empty_a ^ empty_b, merged_arrays, "Incompatible surface arrays");
            if (!empty_a) {
                indices.append_array(surface_arrays_a[ArrayMesh::ARRAY_INDEX]);
                for (int idx : (PackedInt32Array)surface_arrays_b[ArrayMesh::ARRAY_INDEX]) {
                    indices.push_back(idx + vertex_offset);
                }
            }
        case Mesh::ARRAY_WEIGHTS :
            empty_a = surface_arrays_a[ArrayMesh::ARRAY_WEIGHTS].get_type() == Variant::NIL || (int)surface_arrays_a[ArrayMesh::ARRAY_WEIGHTS].call(StringName("size")) == 0;
            empty_b = surface_arrays_b[ArrayMesh::ARRAY_WEIGHTS].get_type() == Variant::NIL || (int)surface_arrays_b[ArrayMesh::ARRAY_WEIGHTS].call(StringName("size")) == 0;
            ERR_FAIL_COND_V_MSG(empty_a ^ empty_b, merged_arrays, "Incompatible surface arrays");
            if (!empty_a) {
                weights.append_array(surface_arrays_a[ArrayMesh::ARRAY_WEIGHTS]);
                weights.append_array(surface_arrays_b[ArrayMesh::ARRAY_WEIGHTS]);
            }
        case Mesh::ARRAY_BONES :
            empty_a = surface_arrays_a[ArrayMesh::ARRAY_BONES].get_type() == Variant::NIL || (int)surface_arrays_a[ArrayMesh::ARRAY_BONES].call(StringName("size")) == 0;
            empty_b = surface_arrays_b[ArrayMesh::ARRAY_BONES].get_type() == Variant::NIL || (int)surface_arrays_b[ArrayMesh::ARRAY_BONES].call(StringName("size")) == 0;
            ERR_FAIL_COND_V_MSG(empty_a ^ empty_b, merged_arrays, "Incompatible surface arrays");
            if (!empty_a) {
                bones.append_array(surface_arrays_a[ArrayMesh::ARRAY_BONES]);
                bones.append_array(surface_arrays_b[ArrayMesh::ARRAY_BONES]);
            }
        case Mesh::ARRAY_CUSTOM3 :
            empty_a = surface_arrays_a[ArrayMesh::ARRAY_CUSTOM3].get_type() == Variant::NIL || (int)surface_arrays_a[ArrayMesh::ARRAY_CUSTOM3].call(StringName("size")) == 0;
            empty_b = surface_arrays_b[ArrayMesh::ARRAY_CUSTOM3].get_type() == Variant::NIL || (int)surface_arrays_b[ArrayMesh::ARRAY_CUSTOM3].call(StringName("size")) == 0;
            ERR_FAIL_COND_V_MSG(empty_a ^ empty_b, merged_arrays, "Incompatible surface arrays");
            if (!empty_a) {
                custom3.append_array(surface_arrays_a[ArrayMesh::ARRAY_CUSTOM3]);
                custom3.append_array(surface_arrays_b[ArrayMesh::ARRAY_CUSTOM3]);
            }
        case Mesh::ARRAY_CUSTOM2 :
            empty_a = surface_arrays_a[ArrayMesh::ARRAY_CUSTOM2].get_type() == Variant::NIL || (int)surface_arrays_a[ArrayMesh::ARRAY_CUSTOM2].call(StringName("size")) == 0;
            empty_b = surface_arrays_b[ArrayMesh::ARRAY_CUSTOM2].get_type() == Variant::NIL || (int)surface_arrays_b[ArrayMesh::ARRAY_CUSTOM2].call(StringName("size")) == 0;
            ERR_FAIL_COND_V_MSG(empty_a ^ empty_b, merged_arrays, "Incompatible surface arrays");
            if (!empty_a) {
                custom2.append_array(surface_arrays_a[ArrayMesh::ARRAY_CUSTOM2]);
                custom2.append_array(surface_arrays_b[ArrayMesh::ARRAY_CUSTOM2]);

            }
        case Mesh::ARRAY_CUSTOM1 :
            empty_a = surface_arrays_a[ArrayMesh::ARRAY_CUSTOM1].get_type() == Variant::NIL || (int)surface_arrays_a[ArrayMesh::ARRAY_CUSTOM1].call(StringName("size")) == 0;
            empty_b = surface_arrays_b[ArrayMesh::ARRAY_CUSTOM1].get_type() == Variant::NIL || (int)surface_arrays_b[ArrayMesh::ARRAY_CUSTOM1].call(StringName("size")) == 0;
            ERR_FAIL_COND_V_MSG(empty_a ^ empty_b, merged_arrays, "Incompatible surface arrays");
            if (!empty_a) {
                custom1.append_array(surface_arrays_a[ArrayMesh::ARRAY_CUSTOM1]);
                custom1.append_array(surface_arrays_b[ArrayMesh::ARRAY_CUSTOM1]);
            }
        case Mesh::ARRAY_CUSTOM0 :
            empty_a = surface_arrays_a[ArrayMesh::ARRAY_CUSTOM0].get_type() == Variant::NIL || (int)surface_arrays_a[ArrayMesh::ARRAY_CUSTOM0].call(StringName("size")) == 0;
            empty_b = surface_arrays_b[ArrayMesh::ARRAY_CUSTOM0].get_type() == Variant::NIL || (int)surface_arrays_b[ArrayMesh::ARRAY_CUSTOM0].call(StringName("size")) == 0;
            ERR_FAIL_COND_V_MSG(empty_a ^ empty_b, merged_arrays, "Incompatible surface arrays");
            if (!empty_a) {
                custom0.append_array(surface_arrays_a[ArrayMesh::ARRAY_CUSTOM0]);
                custom0.append_array(surface_arrays_b[ArrayMesh::ARRAY_CUSTOM0]);
            }
        case Mesh::ARRAY_TEX_UV2 :
            empty_a = surface_arrays_a[ArrayMesh::ARRAY_TEX_UV2].get_type() == Variant::NIL || (int)surface_arrays_a[ArrayMesh::ARRAY_TEX_UV2].call(StringName("size")) == 0;
            empty_b = surface_arrays_b[ArrayMesh::ARRAY_TEX_UV2].get_type() == Variant::NIL || (int)surface_arrays_b[ArrayMesh::ARRAY_TEX_UV2].call(StringName("size")) == 0;
            ERR_FAIL_COND_V_MSG(empty_a ^ empty_b, merged_arrays, "Incompatible surface arrays");
            if (!empty_a) {
                uv2s.append_array(surface_arrays_a[ArrayMesh::ARRAY_TEX_UV2]);
                uv2s.append_array(surface_arrays_b[ArrayMesh::ARRAY_TEX_UV2]);
            }
        case Mesh::ARRAY_TEX_UV :
            empty_a = surface_arrays_a[ArrayMesh::ARRAY_TEX_UV].get_type() == Variant::NIL || (int)surface_arrays_a[ArrayMesh::ARRAY_TEX_UV].call(StringName("size")) == 0;
            empty_b = surface_arrays_b[ArrayMesh::ARRAY_TEX_UV].get_type() == Variant::NIL || (int)surface_arrays_b[ArrayMesh::ARRAY_TEX_UV].call(StringName("size")) == 0;
            ERR_FAIL_COND_V_MSG(empty_a ^ empty_b, merged_arrays, "Incompatible surface arrays");
            if (!empty_a) {
                uvs.append_array(surface_arrays_a[ArrayMesh::ARRAY_TEX_UV]);
                uvs.append_array(surface_arrays_b[ArrayMesh::ARRAY_TEX_UV]);
            }
        case Mesh::ARRAY_COLOR :
            empty_a = surface_arrays_a[ArrayMesh::ARRAY_COLOR].get_type() == Variant::NIL || (int)surface_arrays_a[ArrayMesh::ARRAY_COLOR].call(StringName("size")) == 0;
            empty_b = surface_arrays_b[ArrayMesh::ARRAY_COLOR].get_type() == Variant::NIL || (int)surface_arrays_b[ArrayMesh::ARRAY_COLOR].call(StringName("size")) == 0;
            ERR_FAIL_COND_V_MSG(empty_a ^ empty_b, merged_arrays, "Incompatible surface arrays");
            if (!empty_a) {
                colors.append_array(surface_arrays_a[ArrayMesh::ARRAY_COLOR]);
                colors.append_array(surface_arrays_b[ArrayMesh::ARRAY_COLOR]);
            }
        case Mesh::ARRAY_TANGENT :
            empty_a = surface_arrays_a[ArrayMesh::ARRAY_TANGENT].get_type() == Variant::NIL || (int)surface_arrays_a[ArrayMesh::ARRAY_TANGENT].call(StringName("size")) == 0;
            empty_b = surface_arrays_b[ArrayMesh::ARRAY_TANGENT].get_type() == Variant::NIL || (int)surface_arrays_b[ArrayMesh::ARRAY_TANGENT].call(StringName("size")) == 0;
            ERR_FAIL_COND_V_MSG(empty_a ^ empty_b, merged_arrays, "Incompatible surface arrays");
            if (!empty_a) {
                tangents.append_array(surface_arrays_a[ArrayMesh::ARRAY_TANGENT]);
                tangents.append_array(surface_arrays_b[ArrayMesh::ARRAY_TANGENT]);
            }
        case Mesh::ARRAY_NORMAL :
            empty_a = surface_arrays_a[ArrayMesh::ARRAY_NORMAL].get_type() == Variant::NIL || (int)surface_arrays_a[ArrayMesh::ARRAY_NORMAL].call(StringName("size")) == 0;
            empty_b = surface_arrays_b[ArrayMesh::ARRAY_NORMAL].get_type() == Variant::NIL || (int)surface_arrays_b[ArrayMesh::ARRAY_NORMAL].call(StringName("size")) == 0;
            ERR_FAIL_COND_V_MSG(empty_a ^ empty_b, merged_arrays, "Incompatible surface arrays");
            if (!empty_a) {
                normals.append_array(surface_arrays_a[ArrayMesh::ARRAY_NORMAL]);
                normals.append_array(surface_arrays_b[ArrayMesh::ARRAY_NORMAL]);
            }
        case Mesh::ARRAY_VERTEX :
            empty_a = surface_arrays_a[ArrayMesh::ARRAY_VERTEX].get_type() == Variant::NIL || (int)surface_arrays_a[ArrayMesh::ARRAY_VERTEX].call(StringName("size")) == 0;
            empty_b = surface_arrays_b[ArrayMesh::ARRAY_VERTEX].get_type() == Variant::NIL || (int)surface_arrays_b[ArrayMesh::ARRAY_VERTEX].call(StringName("size")) == 0;
            if (!empty_a || !empty_b) {
                vertices.append_array(surface_arrays_a[ArrayMesh::ARRAY_VERTEX]); 
                vertices.append_array(surface_arrays_b[ArrayMesh::ARRAY_VERTEX]);
            }
    }

    // Set the merged arrays
    merged_arrays[ArrayMesh::ARRAY_VERTEX] = vertices;
    merged_arrays[ArrayMesh::ARRAY_NORMAL] = normals;
    merged_arrays[ArrayMesh::ARRAY_TANGENT] = tangents;
    merged_arrays[ArrayMesh::ARRAY_COLOR] = colors;
    merged_arrays[ArrayMesh::ARRAY_TEX_UV] = uvs;
    merged_arrays[ArrayMesh::ARRAY_TEX_UV2] = uv2s;
    if ((p_flags >> Mesh::ARRAY_FORMAT_CUSTOM0_SHIFT & Mesh::ARRAY_FORMAT_CUSTOM_MASK) < 4) {
        merged_arrays[ArrayMesh::ARRAY_CUSTOM0] = PackedByteArray(custom0);
    } else {
        merged_arrays[ArrayMesh::ARRAY_CUSTOM0] = PackedFloat32Array(custom0);
    }
    if ((p_flags >> Mesh::ARRAY_FORMAT_CUSTOM1_SHIFT & Mesh::ARRAY_FORMAT_CUSTOM_MASK) < 4) {
        merged_arrays[ArrayMesh::ARRAY_CUSTOM1] = PackedByteArray(custom1);
    } else {
        merged_arrays[ArrayMesh::ARRAY_CUSTOM1] = PackedFloat32Array(custom1);
    }
    if ((p_flags >> Mesh::ARRAY_FORMAT_CUSTOM2_SHIFT & Mesh::ARRAY_FORMAT_CUSTOM_MASK) < 4) {
        merged_arrays[ArrayMesh::ARRAY_CUSTOM2] = PackedByteArray(custom2);
    } else {
        merged_arrays[ArrayMesh::ARRAY_CUSTOM2] = PackedFloat32Array(custom2);
    }
    if ((p_flags >> Mesh::ARRAY_FORMAT_CUSTOM3_SHIFT & Mesh::ARRAY_FORMAT_CUSTOM_MASK) < 4) {
        merged_arrays[ArrayMesh::ARRAY_CUSTOM3] = PackedByteArray(custom3);
    } else {
        merged_arrays[ArrayMesh::ARRAY_CUSTOM3] = PackedFloat32Array(custom3);
    }
    merged_arrays[ArrayMesh::ARRAY_BONES] = bones;
    merged_arrays[ArrayMesh::ARRAY_WEIGHTS] = weights;
    merged_arrays[ArrayMesh::ARRAY_INDEX] = indices;

    r_surface_material = surface_material_a.is_valid() ? surface_material_a : surface_material_b;


    return merged_arrays;
}

// Helper class for handling the rendering callback




} // namespace godot
