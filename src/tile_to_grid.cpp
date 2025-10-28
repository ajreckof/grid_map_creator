#include "tile_to_grid.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/mesh_library.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

TileToGrid::TileToGrid() : GridMap() {
    grid_height = 0;
    hide_on_run = true;
    tile_map_layers = TypedArray<TileMapLayer>();
}

TileToGrid::~TileToGrid() {
}

void TileToGrid::_bind_methods() {
    // Property bindings
    ClassDB::bind_method(D_METHOD("set_grid_height", "grid_height"), &TileToGrid::set_grid_height);
    ClassDB::bind_method(D_METHOD("get_grid_height"), &TileToGrid::get_grid_height);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "grid_height"), "set_grid_height", "get_grid_height");

    ClassDB::bind_method(D_METHOD("set_hide_on_run", "hide_on_run"), &TileToGrid::set_hide_on_run);
    ClassDB::bind_method(D_METHOD("get_hide_on_run"), &TileToGrid::get_hide_on_run);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "hide_on_run"), "set_hide_on_run", "get_hide_on_run");

    ClassDB::bind_method(D_METHOD("set_tile_to_grid_set", "tile_to_grid_set"), &TileToGrid::set_tile_to_grid_set);
    ClassDB::bind_method(D_METHOD("get_tile_to_grid_set"), &TileToGrid::get_tile_to_grid_set);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "tile_to_grid_set", PROPERTY_HINT_RESOURCE_TYPE, "TileToGridSet"), "set_tile_to_grid_set", "get_tile_to_grid_set");

    // Method bindings
    ClassDB::bind_static_method(get_class_static(), D_METHOD("tile_map_to_grid_map_position", "tile_map_pos", "height"), &TileToGrid::tile_map_to_grid_map_position);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("grid_map_to_tile_map_position", "grid_map_pos"), &TileToGrid::grid_map_to_tile_map_position);
    ClassDB::bind_method(D_METHOD("global_to_grid", "global_position"), &TileToGrid::global_to_grid);
    ClassDB::bind_method(D_METHOD("global_to_height", "global_position"), &TileToGrid::global_to_height);
    ClassDB::bind_method(D_METHOD("grid_to_global", "tile_position", "height_layer"), &TileToGrid::grid_to_global);
    ClassDB::bind_method(D_METHOD("get_tile_map_layers"), &TileToGrid::get_tile_map_layers);
    ClassDB::bind_method(D_METHOD("get_first_tile_map_layer"), &TileToGrid::get_first_tile_map_layer);
    ClassDB::bind_method(D_METHOD("get_cell_tile_data", "coords"), &TileToGrid::get_cell_tile_data);
    ClassDB::bind_method(D_METHOD("get_used_rect"), &TileToGrid::get_used_rect);
    ClassDB::bind_method(D_METHOD("create_tile_from_scene", "tile_pos", "tile_scene", "tile_layer"), &TileToGrid::create_tile_from_scene, DEFVAL(Variant()));
    ClassDB::bind_method(D_METHOD("get_build_grid_map"), &TileToGrid::get_build_grid_map);
    ClassDB::bind_method(D_METHOD("get_clear_grid_map"), &TileToGrid::get_clear_grid_map);

    // Tool button bindings (equivalent to @export_tool_button)
    ADD_PROPERTY(PropertyInfo(Variant::CALLABLE, "BuildGridmap", PROPERTY_HINT_TOOL_BUTTON, "BuildGridmap", PROPERTY_USAGE_EDITOR), "", "get_build_grid_map");
    ADD_PROPERTY(PropertyInfo(Variant::CALLABLE, "ClearGridmap", PROPERTY_HINT_TOOL_BUTTON, "ClearGridmap", PROPERTY_USAGE_EDITOR), "", "get_clear_grid_map");
}

Callable TileToGrid::get_build_grid_map() {
    return callable_mp(this, &TileToGrid::copy_tiles);
}

Callable TileToGrid::get_clear_grid_map() {
    return callable_mp(this, &TileToGrid::clear_tiles);
}

// Property setters/getters
void TileToGrid::set_grid_height(int p_grid_height) {
    grid_height = p_grid_height;
}

int TileToGrid::get_grid_height() const {
    return grid_height;
}

void TileToGrid::set_hide_on_run(bool p_hide_on_run) {
    hide_on_run = p_hide_on_run;
}

bool TileToGrid::get_hide_on_run() const {
    return hide_on_run;
}

void TileToGrid::set_tile_to_grid_set(const Ref<TileToGridSet> &p_tile_to_grid_set) {
    tile_to_grid_set = p_tile_to_grid_set;
    if (tile_to_grid_set.is_valid()) {
        tile_to_grid_set->set_mesh_library(get_mesh_library());
        tile_to_grid_set->set_grid_size(get_cell_size());
    }
}

Ref<TileToGridSet> TileToGrid::get_tile_to_grid_set() const {
    return tile_to_grid_set;
}

void TileToGrid::_ready() {
    GridMap::_ready();
    
    // Connect signals
    connect("child_entered_tree", callable_mp(this, &TileToGrid::_on_child_entered_tree));
    connect("child_exiting_tree", callable_mp(this, &TileToGrid::_on_child_exiting_tree));
    if(!get_mesh_library().is_valid()) {
        set_mesh_library(memnew(MeshLibrary));
        if(tile_to_grid_set.is_valid()){
            tile_to_grid_set->set_mesh_library(get_mesh_library());
        }
    }
    add_to_group("tiletogridgroup", true);

    if (!Engine::get_singleton()->is_editor_hint()) {
        if (hide_on_run) {
            // Hide all TileMapLayer children
            for (int i = 0; i < tile_map_layers.size(); i++) {
                TileMapLayer *tile_layer = cast_to<TileMapLayer>(tile_map_layers[i]);
                if (tile_layer) {
                    tile_layer->set_visible(false);
                }
            }
        }
    }
}

void TileToGrid::_on_child_entered_tree(Node *node) {
    TileMapLayer *tile_layer = cast_to<TileMapLayer>(node);
    if (tile_layer) {
        tile_map_layers.append(tile_layer);
        _setup_tile_layer(tile_layer);
    }
}

void TileToGrid::_on_child_exiting_tree(Node *node) {
    TileMapLayer *tile_layer = cast_to<TileMapLayer>(node);
    if (tile_layer) {
        for (int i = 0; i < tile_map_layers.size(); i++) {
            if (tile_layer == tile_map_layers[i]) {
                tile_map_layers.remove_at(i);
                break;
            }
        }
    }
}

void TileToGrid::_setup_tile_layer(TileMapLayer *tile_layer) {
    if (Engine::get_singleton()->is_editor_hint()) {
        if (tile_to_grid_set.is_valid()) {
            tile_layer->set_tile_set(tile_to_grid_set->get_tile_set());
        }
    } else {
        tile_layer->set_visible(!hide_on_run);
    }
}

void TileToGrid::copy_tiles() {
    UtilityFunctions::print("tile to gridmap : tiles copied");
    clear_tiles();
    
    for (int i = 0; i < tile_map_layers.size(); i++) {
        TileMapLayer *tile_layer = cast_to<TileMapLayer>(tile_map_layers[i]);
        if (tile_layer) {
            _copy_tiles_from_layer(tile_layer);
        }
    }
}

void TileToGrid::_copy_tiles_from_layer(TileMapLayer *tile_layer) {
    TypedArray<Vector2i> used_cells = tile_layer->get_used_cells();
    
    for (int i = 0; i < used_cells.size(); i++) {
        Vector2i tile_pos = used_cells[i];
        TileData *tile_data = tile_layer->get_cell_tile_data(tile_pos);

        if (!tile_data) {
            continue;
        }
        
        Variant scene_name_variant = tile_data->get_custom_data("SceneName");
        if (scene_name_variant.get_type() != Variant::NIL) {
            Ref<PackedScene> tile_scene = scene_name_variant;
            if (tile_scene.is_valid()) {
                continue; // Skip scene tiles for now, they would be handled separately
            }
        }
        
        Variant mesh_name_variant = tile_data->get_custom_data("MeshName");
        String mesh_name = mesh_name_variant;
        
        Ref<MeshLibrary> mesh_lib = get_mesh_library();
        if (!mesh_lib.is_valid()) {
            continue;
        }
        
        int mesh_int = mesh_lib->find_item_by_name(mesh_name);
        if (mesh_int == -1) {
            continue;
        }
        
        Basis grid_basis;
        
        // Apply rotations
        Variant rotation_y_variant = tile_data->get_custom_data("Rotation_Y");
        if (rotation_y_variant.get_type() != Variant::NIL) {
            real_t rotation_y = rotation_y_variant;
            grid_basis = grid_basis.rotated(Vector3(0,1,0), rotation_y / 180.0 * Math_PI);
        }
        
        Variant rotation_z_variant = tile_data->get_custom_data("Rotation_Z");
        if (rotation_z_variant.get_type() != Variant::NIL) {
            real_t rotation_z = rotation_z_variant;
            grid_basis = grid_basis.rotated(Vector3(0,0,1), rotation_z / 180.0 * Math_PI);
        }
        
        Variant rotation_x_variant = tile_data->get_custom_data("Rotation_X");
        if (rotation_x_variant.get_type() != Variant::NIL) {
            real_t rotation_x = rotation_x_variant;
            grid_basis = grid_basis.rotated(Vector3(-1,0,0), rotation_x / 180.0 * Math_PI);
        }
        
        int basis_index = get_orthogonal_index_from_basis(grid_basis);
        Vector3i grid_pos = tile_map_to_grid_map_position(tile_pos, grid_height);
        set_cell_item(grid_pos, mesh_int, basis_index);
    }
}

void TileToGrid::create_tile_from_scene(const Vector2i &tile_pos, const Ref<PackedScene> &tile_scene, TileMapLayer *tile_layer) {
    if (!tile_scene.is_valid()) {
        return;
    }
    
    Node *scene = tile_scene->instantiate();
    if (!scene) {
        return;
    }
    
    Vector3 global_pos = grid_to_global(tile_pos, grid_height);
    scene->set("global_position", global_pos);
    
    // Set cell_size if the scene has this property
    if (scene->has_method("set_cell_size")) {
        scene->call("set_cell_size", get_cell_size());
    }
    
    // Set tile_position if the scene has this property
    if (scene->has_method("set_tile_position")) {
        scene->call("set_tile_position", tile_pos);
    }
    
    add_child(scene, true);
    Node *owner_node = get_owner();
    scene->set_owner(owner_node ? owner_node : this);
}

Vector3i TileToGrid::tile_map_to_grid_map_position(const Vector2i &tile_map_pos, int height) {
    return Vector3i(tile_map_pos.x, height, tile_map_pos.y);
}

Vector2i TileToGrid::grid_map_to_tile_map_position(const Vector3i &grid_map_pos) {
    return Vector2i(grid_map_pos.x, grid_map_pos.z);
}

Vector2i TileToGrid::global_to_grid(const Vector3 &global_position) {
    Vector3 local_pos = to_local(global_position);
    Vector3i grid_position = local_to_map(local_pos);
    return grid_map_to_tile_map_position(grid_position);
}

real_t TileToGrid::global_to_height(const Vector3 &global_position) {
    Vector3 local_pos = to_local(global_position);
    Vector3i grid_position = local_to_map(local_pos);
    Vector3 local_pos_aligned = map_to_local(grid_position);
    Vector3 cell_size = get_cell_size();
    return grid_position.y + (local_pos - local_pos_aligned).y / cell_size.y;
}

Vector3 TileToGrid::grid_to_global(const Vector2i &tile_position, real_t height_layer) {
    Vector3i grid_position = tile_map_to_grid_map_position(tile_position, height_layer);
    Vector3 local_pos = map_to_local(grid_position);
    Vector3 cell_size = get_cell_size();
    local_pos.y += cell_size.y * (height_layer - int(height_layer));
    return to_global(local_pos);
}

void TileToGrid::clear_tiles() {
    UtilityFunctions::print("tile to gridmap : tiles cleared");
    clear();
    
    // Remove all children that are not TileMapLayers
    TypedArray<Node> children = get_children();
    for (int i = 0; i < children.size(); i++) {
        Node *child = cast_to<Node>(children[i]);
        if (child && !cast_to<TileMapLayer>(child)) {
            child->queue_free();
        }
    }
}

TypedArray<TileMapLayer> TileToGrid::get_tile_map_layers() const {
    return tile_map_layers;
}

TileMapLayer *TileToGrid::get_first_tile_map_layer() const {
    if (tile_map_layers.size() > 0) {
        return cast_to<TileMapLayer>(tile_map_layers[0]);
    }
    return nullptr;
}

TileData *TileToGrid::get_cell_tile_data(const Vector2i &coords) {
    TileMapLayer *first_layer = get_first_tile_map_layer();
    if (first_layer) {
        return first_layer->get_cell_tile_data(coords);
    }
    return nullptr;
}

Rect2i TileToGrid::get_used_rect() {
    TileMapLayer *first_layer = get_first_tile_map_layer();
    if (first_layer) {
        return first_layer->get_used_rect();
    }
    return Rect2i();
}

void TileToGrid::_validate_property(PropertyInfo &p_property) const {
    if ("mesh_library" == p_property.name) {
        p_property.usage &= ~PROPERTY_USAGE_EDITOR;
    }
}

void TileToGrid::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_EDITOR_PRE_SAVE:
            if (tile_to_grid_set.is_valid()) {
                tile_to_grid_set->call("update_ids");
            }
            break;
        default:
            GridMap::_notification(p_what);
            break;
    }
}

} // namespace godot