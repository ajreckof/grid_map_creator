#ifndef EDITOR_PROPERTY_ENUM_ICON_H
#define EDITOR_PROPERTY_ENUM_ICON_H

#include <godot_cpp/classes/editor_property.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/templates/vector.hpp>

namespace godot {

class EditorPropertyEnumIcon : public EditorProperty {
	GDCLASS(EditorPropertyEnumIcon, EditorProperty)

private:
	OptionButton *option_button;
	Vector<String> enum_names;
	Vector<Ref<Texture2D>> enum_icons;
	Vector<int> enum_values;
	bool no_label;
	bool update_option_requested;

	void _on_item_selected(int index);
	void request_update_options();
	void update_options();

protected:
	static void _bind_methods();

public:
	EditorPropertyEnumIcon();
	EditorPropertyEnumIcon(const Vector<String> &p_enum_names, const Vector<Ref<Texture2D>> &p_enum_icons, const Vector<int> &p_enum_values);
	~EditorPropertyEnumIcon();

	void _ready() override;

	void set_enum_names(const Vector<String> &p_enum_names);
	Vector<String> get_enum_names() const;

	void set_enum_icons(const Vector<Ref<Texture2D>> &p_enum_icons);
	Vector<Ref<Texture2D>> get_enum_icons() const;

	void set_enum_values(const Vector<int> &p_enum_values);
	Vector<int> get_enum_values() const;

	void set_no_label(bool p_no_label);
	bool get_no_label() const;
};

}

#endif // EDITOR_PROPERTY_ENUM_ICON_H
