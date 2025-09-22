#include "atlas_mesh.h"
#include <godot_cpp/classes/mesh_data_tool.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/variant/plane.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/material.hpp>

namespace godot {



void AtlasMesh::request_update_clipped_mesh() {
    print_line("request_update_clipped_mesh : " + String::num(update_requested));
    if (!update_requested) {
        update_requested = true;

        callable_mp(this, &AtlasMesh::update_clipped_mesh).call_deferred();
    }
}


void AtlasMesh::set_source_mesh(const Ref<ArrayMesh> &mesh) {
    if (source_mesh == mesh) {
        return;
    }
    if (source_mesh.is_valid()) {
        source_mesh->disconnect("changed", callable_mp(this, &AtlasMesh::request_update_clipped_mesh));
    }
    source_mesh = mesh;
    if (source_mesh.is_valid()) {
        source_mesh->connect("changed", callable_mp(this, &AtlasMesh::request_update_clipped_mesh));
    }
    request_update_clipped_mesh();
}

Ref<ArrayMesh> AtlasMesh::get_source_mesh() const {
    return source_mesh;
}

void AtlasMesh::set_bounding_box(const AABB &box) {
    bounding_box = box.intersection(AABB(Vector3(0, 0, 0), Vector3(1, 1, 1)));
    request_update_clipped_mesh();
}

AABB AtlasMesh::get_bounding_box() const {
    return bounding_box;
}

void AtlasMesh::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_source_mesh", "mesh"), &AtlasMesh::set_source_mesh);
    ClassDB::bind_method(D_METHOD("get_source_mesh"), &AtlasMesh::get_source_mesh);
    ClassDB::bind_method(D_METHOD("set_bounding_box", "box"), &AtlasMesh::set_bounding_box);
    ClassDB::bind_method(D_METHOD("get_bounding_box"), &AtlasMesh::get_bounding_box);
    ClassDB::bind_method(D_METHOD("request_update_clipped_mesh"), &AtlasMesh::request_update_clipped_mesh);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "source_mesh", PROPERTY_HINT_RESOURCE_TYPE, "ArrayMesh"), "set_source_mesh", "get_source_mesh");
    ADD_PROPERTY(PropertyInfo(Variant::AABB, "bounding_box"), "set_bounding_box", "get_bounding_box");
}

void AtlasMesh::update_clipped_mesh() {
    print_line("update_clipped_mesh");
    update_requested = false;
    

    this->clear_surfaces();
    if (!source_mesh.is_valid() || source_mesh->get_surface_count() == 0 ) {
        print_error("No source mesh set or source mesh has no surfaces.");
        return;
    }

    if (!bounding_box.has_volume()) {
        print_error("Bounding box has no volume.");
        return;
    }
    // copy everything from the source mesh
    add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, source_mesh->surface_get_arrays(0));
    Ref<Material> material = source_mesh->surface_get_material(0);

    AABB scaled_bounding_box = get_scaled_aabb();
    TypedArray<Plane> planes = TypedArray<Plane>();
    planes.push_back(Plane(Vector3( 1, 0, 0), scaled_bounding_box.position.x )); // minX
    planes.push_back(Plane(Vector3(-1, 0, 0), -(scaled_bounding_box.position.x + scaled_bounding_box.size.x) )); // maxX
    planes.push_back(Plane(Vector3( 0, 1, 0), scaled_bounding_box.position.y )); // minY
    planes.push_back(Plane(Vector3( 0,-1, 0), -(scaled_bounding_box.position.y + scaled_bounding_box.size.y) )); // maxY
    planes.push_back(Plane(Vector3( 0, 0, 1), scaled_bounding_box.position.z )); // minZ
    planes.push_back(Plane(Vector3( 0, 0,-1), -(scaled_bounding_box.position.z + scaled_bounding_box.size.z) )); // maxZ

    for (int i = 0; i < planes.size(); i++) {
        clip_mesh_arrays_against_plane(planes[i]);
    }

    // move the mesh so that center of AAB is at 0,0,0
    Vector3 center = scaled_bounding_box.position + (scaled_bounding_box.size / 2);
    Ref<MeshDataTool> mesh_data_tool(memnew(MeshDataTool));
    mesh_data_tool->create_from_surface(this, 0);
    for (int i = 0; i < mesh_data_tool->get_vertex_count(); i++) {
        mesh_data_tool->set_vertex(i, mesh_data_tool->get_vertex(i) - center);
    }
    clear_surfaces();
    mesh_data_tool->commit_to_surface(this);
    surface_set_material(0, material);
}

// Helper: Clip polygon against a single plane
void AtlasMesh::clip_mesh_arrays_against_plane(Plane plane) {
    Ref<MeshDataTool> mesh_data_tool(memnew(MeshDataTool));
    mesh_data_tool->create_from_surface(this, 0);
    Ref<SurfaceTool> surface_tool(memnew(SurfaceTool));
    surface_tool->begin(Mesh::PRIMITIVE_TRIANGLES);

    if (mesh_data_tool->get_vertex_count() == 0) {
        UtilityFunctions::printerr("MeshDataTool could not be created or has no vertices.");
        return ;
    }


    for( int idx = 0; idx < mesh_data_tool->get_face_count(); idx+=1) {
        int i = mesh_data_tool->get_face_vertex(idx, 0);
        int j = mesh_data_tool->get_face_vertex(idx, 1);
        int k = mesh_data_tool->get_face_vertex(idx, 2);
        
        Vector3 a = mesh_data_tool->get_vertex(i);
        Vector3 b = mesh_data_tool->get_vertex(j);
        Vector3 c = mesh_data_tool->get_vertex(k);

        float dist_a = plane.distance_to(a);
        float dist_b = plane.distance_to(b);
        float dist_c = plane.distance_to(c);

        if (dist_a >= 0 && dist_b >= 0 && dist_c >= 0) {
            // All points are inside the plane
            surface_tool->add_triangle_fan(
                {a, b, c},
                {mesh_data_tool->get_vertex_uv(i), mesh_data_tool->get_vertex_uv(j), mesh_data_tool->get_vertex_uv(k)},
                {mesh_data_tool->get_vertex_color(i), mesh_data_tool->get_vertex_color(j), mesh_data_tool->get_vertex_color(k)},
                {mesh_data_tool->get_vertex_uv2(i), mesh_data_tool->get_vertex_uv2(j), mesh_data_tool->get_vertex_uv2(k)},
                {mesh_data_tool->get_vertex_normal(i), mesh_data_tool->get_vertex_normal(j), mesh_data_tool->get_vertex_normal(k)}
            );
        } else if (dist_a < 0 && dist_b < 0 && dist_c < 0) {
            // All points are outside the plane
            continue;
        } else {
            // Some points are inside and some are outside
            // check if a is inside and c outside if not rotate until both are
            if(dist_a < 0 || dist_c >= 0){
                if (dist_b >= 0){
                    int tmp = i;
                    i = j;
                    j = k;
                    k = tmp;
                    Vector3 tmp_v = a;
                    a = b;
                    b = c;
                    c = tmp_v;
                    float tmp_d = dist_a;
                    dist_a = dist_b;
                    dist_b = dist_c;
                    dist_c = tmp_d;
                } else {
                    int tmp = i;
                    i = k;
                    k = j;
                    j = tmp;
                    Vector3 tmp_v = a;
                    a = c;
                    c = b;
                    b = tmp_v;
                    float tmp_d = dist_a;
                    dist_a = dist_c;
                    dist_c = dist_b;
                    dist_b = tmp_d;
                }
            } 
            if (dist_b >= 0) {
                float dist_b_to_c = dist_b / (dist_b - dist_c);
                float dist_a_to_c = dist_a / (dist_a - dist_c);
                // b is inside c is outside
                surface_tool->add_triangle_fan(
                    {a, b.lerp(c, dist_b_to_c), a.lerp(c, dist_a_to_c)},
                    {
                        mesh_data_tool->get_vertex_uv(i), 
                        mesh_data_tool->get_vertex_uv(j).lerp(mesh_data_tool->get_vertex_uv(k), dist_b_to_c), 
                        mesh_data_tool->get_vertex_uv(i).lerp(mesh_data_tool->get_vertex_uv(k), dist_a_to_c)
                    },
                    {
                        mesh_data_tool->get_vertex_color(i), 
                        mesh_data_tool->get_vertex_color(j).lerp(mesh_data_tool->get_vertex_color(k), dist_b_to_c), 
                        mesh_data_tool->get_vertex_color(i).lerp(mesh_data_tool->get_vertex_color(k), dist_a_to_c)
                    },
                    {
                        mesh_data_tool->get_vertex_uv2(i), 
                        mesh_data_tool->get_vertex_uv2(j).lerp(mesh_data_tool->get_vertex_uv2(k), dist_b_to_c), 
                        mesh_data_tool->get_vertex_uv2(i).lerp(mesh_data_tool->get_vertex_uv2(k), dist_a_to_c)
                    },
                    {
                        mesh_data_tool->get_vertex_normal(i), 
                        mesh_data_tool->get_vertex_normal(j).lerp(mesh_data_tool->get_vertex_normal(k), dist_b_to_c), 
                        mesh_data_tool->get_vertex_normal(i).lerp(mesh_data_tool->get_vertex_normal(k), dist_a_to_c)
                    }
                );
                surface_tool->add_triangle_fan(
                    {a, b, b.lerp(c, dist_b_to_c)},
                    {
                        mesh_data_tool->get_vertex_uv(i), 
                        mesh_data_tool->get_vertex_uv(j), 
                        mesh_data_tool->get_vertex_uv(j).lerp(mesh_data_tool->get_vertex_uv(k), dist_b_to_c)
                    },
                    {
                        mesh_data_tool->get_vertex_color(i), 
                        mesh_data_tool->get_vertex_color(j), 
                        mesh_data_tool->get_vertex_color(j).lerp(mesh_data_tool->get_vertex_color(k), dist_b_to_c)
                    },
                    {
                        mesh_data_tool->get_vertex_uv2(i), 
                        mesh_data_tool->get_vertex_uv2(j), 
                        mesh_data_tool->get_vertex_uv2(j).lerp(mesh_data_tool->get_vertex_uv2(k), dist_b_to_c)
                    },
                    {
                        mesh_data_tool->get_vertex_normal(i), 
                        mesh_data_tool->get_vertex_normal(j), 
                        mesh_data_tool->get_vertex_normal(j).lerp(mesh_data_tool->get_vertex_normal(k), dist_b_to_c)
                    }
                );
            } else {
                float dist_a_to_b = dist_a / (dist_a - dist_b);
                float dist_a_to_c = dist_a / (dist_a - dist_c);


                surface_tool->add_triangle_fan(
                    {a, a.lerp(b, dist_a_to_b), a.lerp(c, dist_a_to_c)},
                    {
                        mesh_data_tool->get_vertex_uv(i), 
                        mesh_data_tool->get_vertex_uv(i).lerp(mesh_data_tool->get_vertex_uv(j), dist_a_to_b), 
                        mesh_data_tool->get_vertex_uv(i).lerp(mesh_data_tool->get_vertex_uv(k), dist_a_to_c)
                    },
                    {
                        mesh_data_tool->get_vertex_color(i), 
                        mesh_data_tool->get_vertex_color(i).lerp(mesh_data_tool->get_vertex_color(j), dist_a_to_b), 
                        mesh_data_tool->get_vertex_color(i).lerp(mesh_data_tool->get_vertex_color(k), dist_a_to_c)
                    },
                    {
                        mesh_data_tool->get_vertex_uv2(i), 
                        mesh_data_tool->get_vertex_uv2(i).lerp(mesh_data_tool->get_vertex_uv2(j), dist_a_to_b), 
                        mesh_data_tool->get_vertex_uv2(i).lerp(mesh_data_tool->get_vertex_uv2(k), dist_a_to_c)
                    },
                    {
                        mesh_data_tool->get_vertex_normal(i), 
                        mesh_data_tool->get_vertex_normal(i).lerp(mesh_data_tool->get_vertex_normal(j), dist_a_to_b), 
                        mesh_data_tool->get_vertex_normal(i).lerp(mesh_data_tool->get_vertex_normal(k), dist_a_to_c)
                    }
                );
            }
        }

    }

    surface_tool->index();
    surface_tool->generate_tangents();
    surface_tool->optimize_indices_for_cache();
    clear_surfaces();
    surface_tool->commit(this);
}

AABB AtlasMesh::get_scaled_aabb() {
    AABB result;
    AABB mesh_aabb = source_mesh->get_aabb();
    result.position = mesh_aabb.position + bounding_box.position * mesh_aabb.size;
    result.set_end(mesh_aabb.position + bounding_box.get_end() * mesh_aabb.size);

    return result;
}

} // namespace godot
