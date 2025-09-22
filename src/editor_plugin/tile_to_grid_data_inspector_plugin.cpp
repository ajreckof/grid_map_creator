#include "tile_to_grid_data_inspector_plugin.h"
#include "editor_property_enum_icon.h"
#include "read_only_resource_preview.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/classes/texture2d.hpp>

using namespace godot;

void TileToGridDataInspectorPlugin::_bind_methods() {
}

TileToGridDataInspectorPlugin::TileToGridDataInspectorPlugin() {
}

TileToGridDataInspectorPlugin::~TileToGridDataInspectorPlugin() {
}

bool TileToGridDataInspectorPlugin::_can_handle(Object *object) const {
	return true;
}

bool TileToGridDataInspectorPlugin::_parse_property(Object *object, Variant::Type type, const String &name, PropertyHint hint_type, const String &hint_string, BitField<PropertyUsageFlags> usage_flags, bool wide) {
	if (type == Variant::INT && (hint_type & PROPERTY_HINT_ENUM) != 0 && hint_string.contains("|")) {
		String class_name_of_property = hint_string.get_slice("|", 1);
		if (!class_name_of_property.is_empty()) {
			// Assertion equivalent - check if the property exists on the object
			if (!object->has_method("get") || object->get(class_name_of_property).get_type() == Variant::NIL) {
				UtilityFunctions::printerr("Property does not exist on object. Object class: ", object->get_class(), ", Property: ", class_name_of_property);
				return false;
			}
			
			Variant get_enum_values_to_icon = object->get(class_name_of_property);
			if (get_enum_values_to_icon.get_type() != Variant::DICTIONARY && get_enum_values_to_icon.get_type() != Variant::ARRAY) {
				UtilityFunctions::printerr("Property should be Dictionary or Array: ", class_name_of_property, " -> ", get_enum_values_to_icon);
				return false;
			}
			
			Vector<String> enum_names;
			Vector<int> enum_values;
			int current_value = 0;
			
			UtilityFunctions::print("hint_string: ", hint_string);
			
			String enum_part = hint_string.get_slice("|", 0);
			PackedStringArray enum_entries = enum_part.split(",");
			
			for (int i = 0; i < enum_entries.size(); i++) {
				PackedStringArray enum_name_value = enum_entries[i].split(":");
				switch (enum_name_value.size()) {
					case 2:
						current_value = enum_name_value[1].to_int();
						break;
					case 1:
						current_value += 1;
						break;
					default:
						UtilityFunctions::printerr("Wrong format in hint string");
						break;
				}
				
				enum_values.append(current_value);
				enum_names.append(enum_name_value[0]);
			}
			
			Vector<Ref<Texture2D>> enum_icons;
			if (get_enum_values_to_icon.get_type() == Variant::ARRAY) {
				Array icon_array = get_enum_values_to_icon;
				UtilityFunctions::print("array");
				for (int i = 0; i < icon_array.size(); i++) {
					enum_icons.append(icon_array[i]);
				}
			} else {
				Dictionary icon_dict = get_enum_values_to_icon;
				for (int i = 0; i < enum_values.size(); i++) {
					if (icon_dict.has(enum_values[i])) {
						enum_icons.append(icon_dict[enum_values[i]]);
					} else {
						enum_icons.append(Ref<Texture2D>());
					}
				}
			}
			
			EditorPropertyEnumIcon *editor_property_enum_icon = memnew(EditorPropertyEnumIcon(enum_names, enum_icons, enum_values));
			add_property_editor(name, editor_property_enum_icon);
			return true;
		}
	} else if (type == Variant::OBJECT && (hint_type & PROPERTY_HINT_RESOURCE_TYPE) != 0 && (usage_flags & PROPERTY_USAGE_READ_ONLY) != 0) {
		ReadOnlyResourcePreview *read_only_preview = memnew(ReadOnlyResourcePreview);
		add_property_editor(name, read_only_preview);
		return true;
	}
	
	return false;
}
