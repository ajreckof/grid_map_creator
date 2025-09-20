#ifndef READ_ONLY_RESOURCE_PREVIEW_HPP
#define READ_ONLY_RESOURCE_PREVIEW_HPP

#include <godot_cpp/classes/editor_property.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/editor_resource_preview.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/texture2d.hpp>

namespace godot {

class ReadOnlyResourcePreview : public EditorProperty {
    GDCLASS(ReadOnlyResourcePreview, EditorProperty);

private:
    Ref<Resource> resource;
    TextureRect *icon_rect = nullptr;
    TextureRect *preview_rect = nullptr;
    Label *label = nullptr;

protected:
    static void _bind_methods();

public:
    ReadOnlyResourcePreview();
    virtual void _update_property() override;
    void update_preview();
    void set_preview(String p_path, Ref<Texture2D> p_preview, Ref<Texture2D> p_thumbnail_preview, Variant p_userdata);
};

} // namespace godot
#endif // READ_ONLY_RESOURCE_PREVIEW_HPP
