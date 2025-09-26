#ifndef TILE_TO_GRID_H
#define TILE_TO_GRID_H

#include <godot_cpp/classes/grid_map.hpp>
#include <godot_cpp/classes/tile_map_layer.hpp>
#include <godot_cpp/classes/tile_data.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "tile_to_grid_set.h"

namespace godot {

class TileToGrid : public GridMap {
    GDCLASS(TileToGrid, GridMap);

public:
    TileToGrid();
    ~TileToGrid();

    // Property setters/getters
    void set_grid_height(int p_grid_height);
    int get_grid_height() const;

    void set_hide_on_run(bool p_hide_on_run);
    bool get_hide_on_run() const;

    void set_tile_to_grid_set(const Ref<TileToGridSet> &p_tile_to_grid_set);
    Ref<TileToGridSet> get_tile_to_grid_set() const;

    // Main functionality methods
    void copy_tiles();
    void clear_tiles();

    // Utility methods for coordinate conversion (non-static as they need to be exposed to Godot)
    static Vector3i tile_map_to_grid_map_position(const Vector2i &tile_map_pos, int height);
    static Vector2i grid_map_to_tile_map_position(const Vector3i &grid_map_pos);
    Vector2i global_to_grid(const Vector3 &global_position);
    real_t global_to_height(const Vector3 &global_position);
    Vector3 grid_to_global(const Vector2i &tile_position, real_t height_layer);

    // TileMapLayer utility methods
    TypedArray<TileMapLayer> get_tile_map_layers() const;
    TileMapLayer *get_first_tile_map_layer() const;
    TileData *get_cell_tile_data(const Vector2i &coords);
    Rect2i get_used_rect();

    // Scene creation method
    void create_tile_from_scene(const Vector2i &tile_pos, const Ref<PackedScene> &tile_scene, TileMapLayer *tile_layer = nullptr);

    // Godot overrides
    void _ready() override;
    void _notification(int p_what) ;
    void _validate_property(PropertyInfo &p_property) const ;

protected:
    static void _bind_methods();

	Callable get_build_grid_map();
    Callable get_clear_grid_map();

private:
    // Properties
    TypedArray<TileMapLayer> tile_map_layers;
    int grid_height;
    bool hide_on_run;
    Ref<TileToGridSet> tile_to_grid_set;

    // Private helper methods
    void _on_child_entered_tree(Node *node);
    void _on_child_exiting_tree(Node *node);
    void _setup_tile_layer(TileMapLayer *tile_layer);
    void _copy_tiles_from_layer(TileMapLayer *tile_layer);
};

} // namespace godot

#endif // TILE_TO_GRID_H