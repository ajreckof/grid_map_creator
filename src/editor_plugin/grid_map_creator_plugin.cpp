#include "grid_map_creator_plugin.h"
#include "tile_to_grid_data_inspector_plugin.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GridMapCreatorPlugin::_bind_methods() {
}

GridMapCreatorPlugin::GridMapCreatorPlugin() {
}

GridMapCreatorPlugin::~GridMapCreatorPlugin() {
}

void GridMapCreatorPlugin::_enter_tree() {
	tile_to_grid_data_inspector_plugin.instantiate();
	add_inspector_plugin(tile_to_grid_data_inspector_plugin);
}

void GridMapCreatorPlugin::_exit_tree() {
	if (tile_to_grid_data_inspector_plugin.is_valid()) {
		remove_inspector_plugin(tile_to_grid_data_inspector_plugin);
		tile_to_grid_data_inspector_plugin.unref();
	}
}
