#ifndef TILE_TO_GRID_SET_H
#define TILE_TO_GRID_SET_H

#include <godot_cpp/core/type_info.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/tile_set.hpp>
#include <godot_cpp/classes/tile_set_atlas_source.hpp>
#include <godot_cpp/classes/mesh_library.hpp>
#include <godot_cpp/classes/gradient_texture2d.hpp>
#include <godot_cpp/classes/gradient.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/typed_dictionary.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "tile_to_grid_data.h"

namespace godot {

enum Mode {
    MATCH_CORNERS_AND_SIDES = 0,
    MATCH_CORNERS = 1,
    MATCH_SIDES = 2
};

class TileToGridSet : public Resource {
    GDCLASS(TileToGridSet, Resource);

public:
    TileToGridSet();
    ~TileToGridSet();

    void set_tile_size(Vector2i p_tile_size);
    Vector2i get_tile_size() const;

    void set_tile_to_grid_data_list(TypedArray<TileToGridData> p_tile_to_grid_data_list);
    TypedArray<TileToGridData> get_tile_to_grid_data_list() const;

    void set_grid_size(Vector3 p_grid_size);
    Vector3 get_grid_size() const;

    void set_tile_set(Ref<TileSet> p_tile_set);
    Ref<TileSet> get_tile_set() const;

    void set_mesh_library(Ref<MeshLibrary> p_mesh_library);
    Ref<MeshLibrary> get_mesh_library() const;

    // Méthodes publiques
    void add_tile_to_grid_data(Ref<TileToGridData> data);
    void remove_tile_to_grid_data_by_index(int index);
    void remove_tile_to_grid_data_by_name(const String& name);
    int get_tile_to_grid_data_count() const;
    void set_tile_to_grid_data_count(int count);
    int find_tile_to_grid_data_by_name(const String& name) const;

    void set_terrain_set_count(int p_terrain_set_count);
    int get_terrain_set_count() const;
    void update_ids();

protected:
    static void _bind_methods();
    void _get_property_list(List<PropertyInfo> *p_list) const;
    bool _get(const StringName &p_name, Variant &r_ret) const;
    bool _set(const StringName &p_name, const Variant &p_value);
    void setup_data(Ref<TileToGridData> data);
    void unsetup_data(Ref<TileToGridData> data);

private:
    Ref<TileSet> tile_set;
    Vector<TypedArray<Texture2D>> terrain_set_to_terrain_bit_to_icon;
    PackedStringArray terrain_set_to_hint_string;
    String terrain_set_hint_string;
    TypedArray<TileToGridData> tile_to_grid_data_list;
    Vector3 grid_size;
    Ref<MeshLibrary> mesh_library;
    TypedArray<Texture2D> default_terrain_bit_to_icon;
};

} // namespace godot

VARIANT_ENUM_CAST(Mode);

#endif // TILE_TO_GRID_SET_H