#ifndef TRANSFORMED_MESH_H
#define TRANSFORMED_MESH_H

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/variant/transform3d.hpp>

namespace godot {
class TransformedMesh : public ArrayMesh {
    GDCLASS(TransformedMesh, ArrayMesh);

private:
    Ref<ArrayMesh> source_mesh;
    Transform3D transform;
    bool update_requested = false;

protected:
    static void _bind_methods();

public:
    TransformedMesh() : ArrayMesh() {};
    ~TransformedMesh() {};

    void set_source_mesh(const Ref<ArrayMesh> &mesh);
    Ref<ArrayMesh> get_source_mesh() const;

    void set_transform(const Transform3D &p_transform);
    Transform3D get_transform() const;
    
    void set_position(const Vector3 &p_position);
    Vector3 get_position() const;

    void set_rotation(const Vector3 &p_rotation, EulerOrder p_order = EulerOrder::EULER_ORDER_YXZ);
    Vector3 get_rotation(EulerOrder p_order = EulerOrder::EULER_ORDER_YXZ) const;

    void set_size(const Vector3 &p_scale);
    Vector3 get_size() const;

    void scale(const Vector3 &p_scale);
    void rotate(const Vector3 &p_axis, real_t p_angle);
    void translate(const Vector3 &p_translation);

    void update_transformed_mesh();
    void request_update_transformed_mesh();
};


} // namespace godot

#endif // TRANSFORMED_MESH_H