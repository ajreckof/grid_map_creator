#ifndef GRID_MAP_CREATOR_PLUGIN_H
#define GRID_MAP_CREATOR_PLUGIN_H

#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

class TileToGridDataInspectorPlugin;

class GridMapCreatorPlugin : public EditorPlugin {
	GDCLASS(GridMapCreatorPlugin, EditorPlugin)

private:
	Ref<TileToGridDataInspectorPlugin> tile_to_grid_data_inspector_plugin;

protected:
	static void _bind_methods();

public:
	GridMapCreatorPlugin();
	~GridMapCreatorPlugin();

	void _enter_tree() override;
	void _exit_tree() override;
};

}

#endif // GRID_MAP_CREATOR_PLUGIN_H
