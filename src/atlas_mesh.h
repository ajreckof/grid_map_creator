#ifndef ATLAS_MESH_H
#define ATLAS_MESH_H

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

class AtlasMesh : public ArrayMesh {
    GDCLASS(AtlasMesh, ArrayMesh);

public:
    AtlasMesh() : ArrayMesh() {};
    ~AtlasMesh() {};

    void request_update_clipped_mesh();

    void set_source_mesh(const Ref<ArrayMesh> &mesh);
    Ref<ArrayMesh> get_source_mesh() const;

    void set_bounding_box(const AABB &box);
    AABB get_bounding_box() const;

protected:
    static void _bind_methods();

private:
    Ref<ArrayMesh> source_mesh;
    AABB bounding_box = AABB(Vector3(0, 0, 0), Vector3(1, 1, 1));
    void update_clipped_mesh();
    void clip_mesh_arrays_against_plane(Plane plane);
    AABB get_scaled_aabb();
    Ref<ArrayMesh> clip_mesh_to_AABB(const Ref<ArrayMesh> &mesh, const AABB &box);
    bool update_requested = false;
};

} // namespace godot

#endif // ATLAS_MESH_H
