#ifndef PREVIEW_TEXTURE_H
#define PREVIEW_TEXTURE_H

#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>

namespace godot {
class PreviewTexture : public ImageTexture {
    GDCLASS(PreviewTexture, ImageTexture);

protected:
    int width = 128;
    Vector3 rotation = Vector3(-Math_PI/2, 0, 0);
    Viewport::DebugDraw debug_draw = Viewport::DEBUG_DRAW_UNSHADED;
    bool ready = false;
    bool update_requested = false;

    Camera3D *camera;
    SubViewport *viewport;
    AABB aabb;

    static void _bind_methods();

public:
    PreviewTexture();
    virtual ~PreviewTexture();

    bool is_ready() const;

    void set_rotation(const Vector3 &p_rotation);
    Vector3 get_rotation() const;

    Viewport::DebugDraw get_debug_draw() const;
    void set_debug_draw(Viewport::DebugDraw p_debug_draw);

    void set_width(int p_width);
    int get_width() const;

    virtual void request_update();

    // Virtual methods to be implemented by derived classes
    void update_texture() ;
    virtual void _update_texture() = 0;
    void assign_image();
    void _validate_property(PropertyInfo &property) const;
};

}

#endif // PREVIEW_TEXTURE_H
