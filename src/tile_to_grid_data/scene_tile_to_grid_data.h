#ifndef SCENE_TILE_TO_GRID_DATA_H
#define SCENE_TILE_TO_GRID_DATA_H

#include "tile_to_grid_data.h"
#include <godot_cpp/classes/packed_scene.hpp>

#include "preview_texture/scene_preview_texture.h"

namespace godot {

class SceneTileToGridData : public TileToGridData {
    GDCLASS(SceneTileToGridData, TileToGridData);

public:
    SceneTileToGridData();
    ~SceneTileToGridData();

    virtual void _setup_local_to_scene() override;
    
    virtual TypedDictionary<String, TileItemData> _generate_tiles_data() override;
    
    // Override methods from parent class
    void _get_property_list(List<PropertyInfo> *p_list) const ;
    void _validate_property(PropertyInfo &p_property) const;
    bool _get(const StringName &p_name, Variant &r_ret) const;
    bool _set(const StringName &p_name, const Variant &p_value);
    
    // Scene-specific methods
    void set_tiles_count(int count);
    int get_tiles_count() const;
    void init_tile_2D_preview();

protected:
    static void _bind_methods();

private:
    bool initialised = false; // Flag to check if the preview has been initialized

    int _tiles_count = 0; // Number of tiles in the grid

    String base_name = "tiles_{0}/";

    TypedArray<String> _unique_names;
    const StringName unique_name_property_name = "unique_name";

    TypedDictionary<String, Ref<PackedScene>> _scenes;
    const StringName scene_property_name = "scene";

    HashMap<String, Ref<ScenePreviewTexture>> _preview_textures; // Preview textures for each tile
    const StringName preview_texture_property_name = "preview_texture";

    HashMap<int, String> renames;
    void _rename_items();
};
}
#endif // SCENE_TILE_TO_GRID_DATA_H
