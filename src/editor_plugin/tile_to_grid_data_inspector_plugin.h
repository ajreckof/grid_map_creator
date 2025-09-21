#ifndef TILE_TO_GRID_DATA_INSPECTOR_PLUGIN_H
#define TILE_TO_GRID_DATA_INSPECTOR_PLUGIN_H

#include <godot_cpp/classes/editor_inspector_plugin.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class TileToGridDataInspectorPlugin : public EditorInspectorPlugin {
	GDCLASS(TileToGridDataInspectorPlugin, EditorInspectorPlugin)

protected:
	static void _bind_methods();

public:
	TileToGridDataInspectorPlugin();
	~TileToGridDataInspectorPlugin();

	bool _can_handle(Object *object) const override;
	bool _parse_property(Object *object, Variant::Type type, const String &name, PropertyHint hint_type, const String &hint_string, BitField<PropertyUsageFlags> usage_flags, bool wide) override;
};

}

#endif // TILE_TO_GRID_DATA_INSPECTOR_PLUGIN_H
