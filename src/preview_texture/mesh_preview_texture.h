#ifndef MESH_PREVIEW_TEXTURE_H
#define MESH_PREVIEW_TEXTURE_H

#include "preview_texture.h"
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

namespace godot {
class MeshPreviewTexture : public PreviewTexture {
    GDCLASS(MeshPreviewTexture, PreviewTexture);

private:
    MeshInstance3D *mesh_instance;

protected:
    static void _bind_methods();

public:
    MeshPreviewTexture();
    ~MeshPreviewTexture();

    void set_mesh(const Ref<ArrayMesh> &p_mesh);
    Ref<ArrayMesh> get_mesh() const;

    void _update_texture() override;
};

}

#endif // MESH_PREVIEW_TEXTURE_H
