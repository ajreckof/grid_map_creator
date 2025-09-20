#include "read_only_resource_preview.h"
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>

using namespace godot;

void ReadOnlyResourcePreview::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_preview", "p_path", "p_preview", "p_thumbnail_preview", "p_userdata"), &ReadOnlyResourcePreview::set_preview);
}

ReadOnlyResourcePreview::ReadOnlyResourcePreview() {
    HBoxContainer *hbox = memnew(HBoxContainer);
    add_child(hbox);

    icon_rect = memnew(TextureRect);
    icon_rect->set_custom_minimum_size(Size2(24, 24));
    hbox->add_child(icon_rect);

    
    preview_rect = memnew(TextureRect);
    preview_rect->set_custom_minimum_size(Size2(48, 48));
    preview_rect->set_h_size_flags(Control::SIZE_SHRINK_CENTER);
    hbox->add_child(preview_rect);


    label = memnew(Label);
    hbox->add_child(label);

}


void ReadOnlyResourcePreview::_update_property() {
    if (resource.is_valid()) {
        resource->disconnect("changed", callable_mp(this, &ReadOnlyResourcePreview::update_preview));
    }
    resource = get_edited_object()->get(get_edited_property());
    if (resource.is_null()) {
        icon_rect->set_texture(nullptr);
        preview_rect->set_texture(nullptr);
        label->set_text("Aucune ressource");
        return;
    }
    if (resource.is_valid()) {
        resource->connect("changed", callable_mp(this, &ReadOnlyResourcePreview::update_preview));
            // Icône de la classe
        Ref<Texture2D> icon;
        if (EditorInterface::get_singleton()) {
            Ref<Theme> theme = EditorInterface::get_singleton()->get_base_control()->get_theme();
            if (theme.is_valid()) {
                icon = theme->get_icon(resource->get_class(), "EditorIcons");
            }
        }

        icon_rect->set_texture(icon);
    }
    update_preview();
}


void ReadOnlyResourcePreview::update_preview() {
    // Générer le preview
    EditorResourcePreview *preview = EditorInterface::get_singleton()->get_resource_previewer();
    if (preview) {
        preview->queue_edited_resource_preview(resource, this, "set_preview" , nullptr); 
    }
}


void ReadOnlyResourcePreview::set_preview(String p_path, Ref<Texture2D> p_preview, Ref<Texture2D> p_thumbnail_preview, Variant p_userdata) {
    print_line("set_preview : ");
    print_line(p_path);
    print_line(p_preview);
    print_line(p_thumbnail_preview);
    print_line(p_userdata);
    if(p_preview.is_valid()) {
        preview_rect->set_texture(p_preview);
        label->set_text("");
    } else {
        preview_rect->set_texture(nullptr);
        label->set_text(resource->get_class());
    }
}