#include "editor_property_enum_icon.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/text_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void EditorPropertyEnumIcon::_bind_methods() {

}

EditorPropertyEnumIcon::EditorPropertyEnumIcon() {
	no_label = false;
	update_option_requested = false;
	
	option_button = memnew(OptionButton);
	option_button->set_text_overrun_behavior(TextServer::OVERRUN_TRIM_ELLIPSIS);
	option_button->connect("item_selected", Callable(this, "_on_item_selected"));
	add_child(option_button);

	update_options();
}

EditorPropertyEnumIcon::EditorPropertyEnumIcon(const Vector<String> &p_enum_names, const Vector<Ref<Texture2D>> &p_enum_icons, const Vector<int> &p_enum_values) {
	no_label = false;
	update_option_requested = false;
	
	enum_names = p_enum_names;
	enum_icons = p_enum_icons;
	enum_values = p_enum_values;
	
	option_button = memnew(OptionButton);
	option_button->set_text_overrun_behavior(TextServer::OVERRUN_TRIM_ELLIPSIS);
	option_button->connect("item_selected", callable_mp(this, &EditorPropertyEnumIcon::_on_item_selected));
	add_child(option_button);

	update_options();
}

EditorPropertyEnumIcon::~EditorPropertyEnumIcon() {
}

void EditorPropertyEnumIcon::_ready() {
	Object *edited_object = get_edited_object();
	if (edited_object) {
		Variant current_value = edited_object->get(get_edited_property());
		UtilityFunctions::print("Current value: ", current_value);
		
		int current_index = enum_values.find(current_value);
		UtilityFunctions::print("Current index: ", current_index);
		
		if (current_index >= 0) {
			option_button->select(current_index);
		}
		
		UtilityFunctions::print("Selected ID: ", option_button->get_selected_id());
		UtilityFunctions::print("Item count: ", option_button->get_item_count());
	}
}

void EditorPropertyEnumIcon::_on_item_selected(int index) {
	UtilityFunctions::print("reached here: ", get_edited_property());
	Object *edited_object = get_edited_object();
	if (edited_object && index >= 0 && index < enum_values.size()) {
		edited_object->set(get_edited_property(), enum_values[index]);
	}
}

void EditorPropertyEnumIcon::request_update_options() {
	if (update_option_requested) {
		return;
	}
	update_option_requested = true;
	call_deferred("update_options");
}

void EditorPropertyEnumIcon::update_options() {
	update_option_requested = false;
	option_button->clear();
	
	if (enum_icons.size() != enum_names.size()) {
		UtilityFunctions::printerr("Enum icons and names arrays must have the same size.");
		return;
	}
	
	for (int i = 0; i < enum_icons.size(); i++) {
		if (enum_icons[i].is_valid()) {
			String text = no_label ? "" : enum_names[i];
			option_button->add_icon_item(enum_icons[i], text);
		} else {
			option_button->add_item(enum_names[i]);
		}
	}
	
	Object *edited_object = get_edited_object();
	if (edited_object) {
		Variant current_value = edited_object->get(get_edited_property());
		int current_index = enum_values.find(current_value);
		if (current_index >= 0) {
			option_button->select(current_index);
		}
	}
}

void EditorPropertyEnumIcon::set_enum_names(const Vector<String> &p_enum_names) {
	enum_names = p_enum_names;
	request_update_options();
}

Vector<String> EditorPropertyEnumIcon::get_enum_names() const {
	return enum_names;
}

void EditorPropertyEnumIcon::set_enum_icons(const Vector<Ref<Texture2D>> &p_enum_icons) {
	enum_icons = p_enum_icons;
	request_update_options();
}

Vector<Ref<Texture2D>> EditorPropertyEnumIcon::get_enum_icons() const {
	return enum_icons;
}

void EditorPropertyEnumIcon::set_enum_values(const Vector<int> &p_enum_values) {
	enum_values = p_enum_values;
}

Vector<int> EditorPropertyEnumIcon::get_enum_values() const {
	return enum_values;
}

void EditorPropertyEnumIcon::set_no_label(bool p_no_label) {
	no_label = p_no_label;
	request_update_options();
}

bool EditorPropertyEnumIcon::get_no_label() const {
	return no_label;
}
