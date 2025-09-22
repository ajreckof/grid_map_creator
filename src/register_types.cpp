#include "register_types.h"

#include "editor_plugin/grid_map_creator_plugin.h"
#include "editor_plugin/read_only_resource_preview.h"
#include "mesh_utility/atlas_mesh.h"
#include "mesh_utility/transformed_mesh.h"
#include "mesh_utility/merged_mesh.h"
#include "mesh_utility/mesh_utility.h"
#include "preview_texture/mesh_preview_texture.h"
#include "preview_texture/scene_preview_texture.h"
#include "tile_to_grid_data/tile_to_grid_data.h"
#include "tile_to_grid_data/merging_tile_to_grid_data.h"
#include "tile_to_grid_data/scene_tile_to_grid_data.h"
#include "tile_to_grid_set.h"
#include "tile_to_grid.h"
#include "merged_image.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_module(ModuleInitializationLevel p_level) {
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_SCENE:
			GDREGISTER_CLASS(AtlasMesh);
			GDREGISTER_CLASS(TransformedMesh);
			GDREGISTER_CLASS(MergedMesh);
			GDREGISTER_ABSTRACT_CLASS(PreviewTexture);
			GDREGISTER_CLASS(MeshPreviewTexture);
			GDREGISTER_CLASS(ScenePreviewTexture);
			GDREGISTER_CLASS(MergedImage);
			GDREGISTER_CLASS(TileItemData);
			GDREGISTER_CLASS(MeshTileItemData);
			GDREGISTER_CLASS(SceneTileItemData);
			GDREGISTER_CLASS(TileToGridData);
			GDREGISTER_CLASS(MergingTileToGridData);
			GDREGISTER_CLASS(SceneTileToGridData);
			GDREGISTER_CLASS(TileToGridSet);
			GDREGISTER_CLASS(TileToGrid);
			break;
		case MODULE_INITIALIZATION_LEVEL_EDITOR:
			GDREGISTER_CLASS(GridMapCreatorPlugin);
			EditorPlugins::add_by_type<GridMapCreatorPlugin>();
			break;
		default:
			break;
	}

}

void uninitialize_module(ModuleInitializationLevel p_level) {
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_EDITOR:
			EditorPlugins::remove_by_type<GridMapCreatorPlugin>();
			break;
		default:
			break;
	}

}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT grid_map_creator_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_module);
	init_obj.register_terminator(uninitialize_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}