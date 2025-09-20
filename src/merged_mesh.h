#ifndef MERGED_MESH_H
#define MERGED_MESH_H

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

class MergedMesh : public ArrayMesh {
    GDCLASS(MergedMesh, ArrayMesh);

public:
    MergedMesh() : ArrayMesh() {};
    ~MergedMesh() {};

    void set_source_meshes(const TypedArray<ArrayMesh> &meshes);
    void add_source_mesh(const Ref<ArrayMesh> &mesh);
    TypedArray<ArrayMesh> get_source_meshes() const;
    void request_update_merged_mesh();

protected:
    static void _bind_methods();

private:
    TypedArray<ArrayMesh> source_meshes;
    void update_merged_mesh();
    bool update_requested = false;
};

} // namespace godot

#endif // MERGED_MESH_H
