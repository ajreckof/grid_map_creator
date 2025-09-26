#include "tile_to_grid_set.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/editor_interface.hpp>

namespace godot {

Ref<Texture2D> create_texture_from_color(const Color &color, int width = 16) {

    if (!Engine::get_singleton()->is_editor_hint()) {
        // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
        return nullptr;
    }
    width *= EditorInterface::get_singleton()->get_editor_scale();
    Ref<Image> image = Image::create(width, width, false, Image::FORMAT_RGBA8);
    image->fill(color);
    Ref<ImageTexture> texture = ImageTexture::create_from_image(image);
    return texture;
}

// TileToGridSet implementation
TileToGridSet::TileToGridSet() : Resource() {

    if (!Engine::get_singleton()->is_editor_hint()) {
        // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
        return;
    }
    default_terrain_bit_to_icon = TypedArray<Texture2D>();
    default_terrain_bit_to_icon.resize(1);
    UtilityFunctions::print("tileToGrid _ready : creating tile set");
}

TileToGridSet::~TileToGridSet() {
}

void TileToGridSet::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_tile_set"), &TileToGridSet::get_tile_set);
    ClassDB::bind_method(D_METHOD("set_tile_set", "tile_set"), &TileToGridSet::set_tile_set);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "tile_set", PROPERTY_HINT_RESOURCE_TYPE, "TileSet", PROPERTY_USAGE_READ_ONLY | PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT), "set_tile_set", "get_tile_set");

    ClassDB::bind_method(D_METHOD("get_tile_size"), &TileToGridSet::get_tile_size);
    ClassDB::bind_method(D_METHOD("set_tile_size", "tile_size"), &TileToGridSet::set_tile_size);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "tile_preview_size", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_tile_size", "get_tile_size");

    ClassDB::bind_method(D_METHOD("get_tile_to_grid_data_count"), &TileToGridSet::get_tile_to_grid_data_count);
    ClassDB::bind_method(D_METHOD("set_tile_to_grid_data_count", "count"), &TileToGridSet::set_tile_to_grid_data_count);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "tile_to_grid_data_count", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_ARRAY | PROPERTY_USAGE_DEFAULT, "TileToGridData,tile_to_grid_data_"), "set_tile_to_grid_data_count", "get_tile_to_grid_data_count");

    ClassDB::bind_method(D_METHOD("get_grid_size"), &TileToGridSet::get_grid_size);
    ClassDB::bind_method(D_METHOD("set_grid_size", "grid_size"), &TileToGridSet::set_grid_size);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "grid_size", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_grid_size", "get_grid_size");

    ClassDB::bind_method(D_METHOD("get_mesh_library"), &TileToGridSet::get_mesh_library);
    ClassDB::bind_method(D_METHOD("set_mesh_library", "mesh_library"), &TileToGridSet::set_mesh_library);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh_library", PROPERTY_HINT_RESOURCE_TYPE, "MeshLibrary", PROPERTY_USAGE_NONE), "set_mesh_library", "get_mesh_library");

    ClassDB::bind_method(D_METHOD("get_terrain_set_count"), &TileToGridSet::get_terrain_set_count);
    ClassDB::bind_method(D_METHOD("set_terrain_set_count", "terrain_set_count"), &TileToGridSet::set_terrain_set_count);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "terrain_set_count", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_ARRAY | PROPERTY_USAGE_DEFAULT, "Terrain Sets,terrain_set_"), "set_terrain_set_count", "get_terrain_set_count");

    ClassDB::bind_method(D_METHOD("add_tile_to_grid_data", "data"), &TileToGridSet::add_tile_to_grid_data);
    ClassDB::bind_method(D_METHOD("remove_tile_to_grid_data_by_index", "index"), &TileToGridSet::remove_tile_to_grid_data_by_index);
    ClassDB::bind_method(D_METHOD("remove_tile_to_grid_data_by_name", "name"), &TileToGridSet::remove_tile_to_grid_data_by_name);
    ClassDB::bind_method(D_METHOD("find_tile_to_grid_data_by_name", "name"), &TileToGridSet::find_tile_to_grid_data_by_name);
    ClassDB::bind_method(D_METHOD("update_ids"), &TileToGridSet::update_ids);
}

void TileToGridSet::set_tile_set(Ref<TileSet> p_tile_set) {
    UtilityFunctions::print("TileToGridSet: Setting TileSet");
    if (!p_tile_set.is_valid()){
        p_tile_set = (Ref<TileSet>)memnew(TileSet);
    }
    if (tile_set == p_tile_set) {
        return; // No change, do nothing
    }
    tile_set = p_tile_set;
    if (!tile_set->has_custom_data_layer_by_name(MESH_NAME_LAYER)) {
        tile_set->add_custom_data_layer();
        tile_set->set_custom_data_layer_name(tile_set->get_custom_data_layers_count() - 1, MESH_NAME_LAYER);
        tile_set->set_custom_data_layer_type(tile_set->get_custom_data_layers_count() - 1, Variant::STRING);
    }
    if (!tile_set->has_custom_data_layer_by_name(ROTATION_X_LAYER)) {
        tile_set->add_custom_data_layer();
        tile_set->set_custom_data_layer_name(tile_set->get_custom_data_layers_count() - 1, ROTATION_X_LAYER);
        tile_set->set_custom_data_layer_type(tile_set->get_custom_data_layers_count() - 1, Variant::FLOAT);
    }
    if (!tile_set->has_custom_data_layer_by_name(ROTATION_Y_LAYER)) {
        tile_set->add_custom_data_layer();
        tile_set->set_custom_data_layer_name(tile_set->get_custom_data_layers_count() - 1, ROTATION_Y_LAYER);
        tile_set->set_custom_data_layer_type(tile_set->get_custom_data_layers_count() - 1, Variant::FLOAT);
    }
    if (!tile_set->has_custom_data_layer_by_name(ROTATION_Z_LAYER)) {
        tile_set->add_custom_data_layer();
        tile_set->set_custom_data_layer_name(tile_set->get_custom_data_layers_count() - 1, ROTATION_Z_LAYER);
        tile_set->set_custom_data_layer_type(tile_set->get_custom_data_layers_count() - 1, Variant::FLOAT);
    }
    if (!tile_set->has_custom_data_layer_by_name(SCENE_NAME_LAYER)) {
        tile_set->add_custom_data_layer();
        tile_set->set_custom_data_layer_name(tile_set->get_custom_data_layers_count() - 1, SCENE_NAME_LAYER);
        tile_set->set_custom_data_layer_type(tile_set->get_custom_data_layers_count() - 1, Variant::OBJECT);
    }

    terrain_set_hint_string = "No terrain set:-1";
    terrain_set_to_terrain_bit_to_icon.clear();
    terrain_set_to_hint_string.clear();
    for (int i = 0; i < tile_set->get_terrain_sets_count(); i++) {
        terrain_set_to_terrain_bit_to_icon.push_back(default_terrain_bit_to_icon.duplicate());
        terrain_set_to_hint_string.push_back("No terrain:-1");
        terrain_set_hint_string += vformat(",Terrain Set %d", terrain_set_to_hint_string.size());
        for (int j = 0; j < tile_set->get_terrains_count(i); j++) {
            terrain_set_to_terrain_bit_to_icon.get(i).push_back(create_texture_from_color(tile_set->get_terrain_color(i, j)));
            terrain_set_to_hint_string[i] = (String)terrain_set_to_hint_string[i] + vformat(",%s", tile_set->get_terrain_name(i, j));
        }
    }
    print_line("terrain_set_to_hint_string : ", terrain_set_to_hint_string);
    for (TypedArray<Texture2D> terrain_bit_to_icon : terrain_set_to_terrain_bit_to_icon) {
        print_line("terrain_set_to_terrain_bit_to_icon : ", terrain_bit_to_icon);
    }
    for (Ref<TileToGridData> data : tile_to_grid_data_list) {
        setup_data(data);
    }
}

Ref<TileSet> TileToGridSet::get_tile_set() const {
    return tile_set;
}


void TileToGridSet::set_tile_size(Vector2i p_tile_size) {

    if (!Engine::get_singleton()->is_editor_hint()) {
        // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
        return;
    }
    tile_set->set_tile_size(p_tile_size);
    for (int i = 0; i < tile_to_grid_data_list.size(); i++) {
        Ref<TileToGridData> data = tile_to_grid_data_list[i];
        if (data.is_valid()) {
            data->set_tile_size(p_tile_size);
        }
    }
}

Vector2i TileToGridSet::get_tile_size() const {
    if (!tile_set.is_valid()) {
        return Vector2i(16, 16); // Default tile size
    }
    return tile_set->get_tile_size();
}

void TileToGridSet::set_tile_to_grid_data_list(TypedArray<TileToGridData> p_tile_to_grid_data_list) {

    if (!Engine::get_singleton()->is_editor_hint()) {
        // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
        return;
    }
    // Unsetup old data
    for (int i = 0; i < tile_to_grid_data_list.size(); i++) {
        Ref<TileToGridData> data = tile_to_grid_data_list[i];
        if (data.is_valid()) {
            bool found_in_new = false;
            for (int j = 0; j < p_tile_to_grid_data_list.size(); j++) {
                if (p_tile_to_grid_data_list[j] == data) {
                    found_in_new = true;
                    break;
                }
            }
            if (!found_in_new) {
                unsetup_data(data);
            }
        }
    }

    // Setup new data
    for (int i = 0; i < p_tile_to_grid_data_list.size(); i++) {
        Ref<TileToGridData> data = p_tile_to_grid_data_list[i];
        if (data.is_valid()) {
            bool found_in_old = false;
            for (int j = 0; j < tile_to_grid_data_list.size(); j++) {
                if (tile_to_grid_data_list[j] == data) {
                    found_in_old = true;
                    break;
                }
            }
            if (!found_in_old) {
                setup_data(data);
            }
        }
    }

    tile_to_grid_data_list = p_tile_to_grid_data_list;
}

TypedArray<TileToGridData> TileToGridSet::get_tile_to_grid_data_list() const {
    return tile_to_grid_data_list;
}

void TileToGridSet::set_grid_size(Vector3 p_grid_size) {

    if (!Engine::get_singleton()->is_editor_hint()) {
        // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
        return;
    }
    grid_size = p_grid_size;
    for (int i = 0; i < tile_to_grid_data_list.size(); i++) {
        Ref<TileToGridData> data = tile_to_grid_data_list[i];
        if (data.is_valid()) {
            data->set_grid_size(grid_size);
        }
    }
}

Vector3 TileToGridSet::get_grid_size() const {
    return grid_size;
}

void TileToGridSet::set_mesh_library(Ref<MeshLibrary> p_mesh_library) {

    if (!Engine::get_singleton()->is_editor_hint()) {
        // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
        return;
    }
    mesh_library = p_mesh_library;
    for (int i = 0; i < tile_to_grid_data_list.size(); i++) {
        Ref<TileToGridData> data = tile_to_grid_data_list[i];
        if (data.is_valid()) {
            data->set_mesh_library(mesh_library);
        }
    }
}

Ref<MeshLibrary> TileToGridSet::get_mesh_library() const {
    return mesh_library;
}

int TileToGridSet::get_terrain_set_count() const {
    if (tile_set.is_valid()) {
        return tile_set->get_terrain_sets_count();
    }
    return 0;
}

void TileToGridSet::set_terrain_set_count(int p_terrain_set_count) {

    if (!Engine::get_singleton()->is_editor_hint()) {
        // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
        return;
    }
    while (tile_set->get_terrain_sets_count() < p_terrain_set_count) {
        tile_set->add_terrain_set();
        terrain_set_to_terrain_bit_to_icon.push_back(default_terrain_bit_to_icon.duplicate());
        terrain_set_to_hint_string.push_back("No terrain:-1");
        terrain_set_hint_string += vformat(",Terrain Set %d", terrain_set_to_hint_string.size());
    }
    while (tile_set->get_terrain_sets_count() > p_terrain_set_count) {
        tile_set->remove_terrain_set(tile_set->get_terrain_sets_count() - 1);
        terrain_set_to_terrain_bit_to_icon.remove_at(terrain_set_to_terrain_bit_to_icon.size() - 1);
        terrain_set_to_hint_string.remove_at(terrain_set_to_hint_string.size() - 1);
        terrain_set_hint_string = terrain_set_hint_string.substr(0, terrain_set_hint_string.rfind(","));
    }
    print_line("terrain_set_to_hint_string : ", terrain_set_to_hint_string);

    for (Ref<TileToGridData> data : tile_to_grid_data_list) {
        if (data.is_valid()) {
            data->set_terrain_set_hint_string(terrain_set_hint_string);
        }
    }
    notify_property_list_changed();
}

void TileToGridSet::add_tile_to_grid_data(Ref<TileToGridData> data) {

    if (!Engine::get_singleton()->is_editor_hint()) {
        // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
        return;
    }
    if (data.is_valid()) {
        if (tile_to_grid_data_list.has(data)){
            return; // Data already exists, no need to add again
        }
        
        setup_data(data);
        tile_to_grid_data_list.append(data);
    }
}

void TileToGridSet::setup_data(Ref<TileToGridData> data) {

    if (!Engine::get_singleton()->is_editor_hint()) {
        // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
        return;
    }
    if (!data.is_valid()) return;

    Ref<TileSetAtlasSource> tile_set_source;
    int data_id = data->get_data_id();

    if (tile_set->has_source(data_id)) {
        tile_set_source = tile_set->get_source(data_id);
    } else {
        Array source_ids;
        for (int i = 0; i < tile_set->get_source_count(); i++) {
            source_ids.append(tile_set->get_source_id(i));
        }
        UtilityFunctions::print("source not found id was ", data_id, " ids found are :", source_ids);
        
        tile_set_source = (Ref<TileSetAtlasSource>)memnew(TileSetAtlasSource);
        tile_set->add_source(tile_set_source, data_id);
        data->set_data_id(data_id);
    }
    
    data->set_tile_set_source(tile_set_source);
    data->set_mesh_library(mesh_library);
    data->set_grid_size(grid_size);
    data->set_tile_size(tile_set->get_tile_size());
    data->set_terrain_set_to_terrain_bit_to_icon(terrain_set_to_terrain_bit_to_icon);
    data->set_terrain_set_to_hint_string(terrain_set_to_hint_string);
    data->set_terrain_set_hint_string(terrain_set_hint_string);
}

void TileToGridSet::_get_property_list(List<PropertyInfo> *p_list) const {
    // Terrain sets (inchangé)
    for (int i = 0; i < get_terrain_set_count(); i++) {
        p_list->push_back(PropertyInfo(Variant::INT, vformat("terrain_set_%d/mode", i), PROPERTY_HINT_ENUM, "Match Corners and Sides,Match Corners,Match Sides", PROPERTY_USAGE_DEFAULT));
        p_list->push_back(PropertyInfo(Variant::INT, vformat("terrain_set_%d/terrain_count", i), PROPERTY_HINT_NONE, "", PROPERTY_USAGE_ARRAY | PROPERTY_USAGE_DEFAULT, vformat("Terrains,terrain_set_%d/terrain_", i)));
        for (int j = 0; j < tile_set->get_terrains_count(i); j++) {
            p_list->push_back(PropertyInfo(Variant::STRING, vformat("terrain_set_%d/terrain_%d/name", i, j), PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT));
            p_list->push_back(PropertyInfo(Variant::COLOR, vformat("terrain_set_%d/terrain_%d/color", i, j), PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT));
        }
    }
    // TileToGridData list exposée par nombre
    for (int i = 0; i < tile_to_grid_data_list.size(); i++) {
        p_list->push_back(PropertyInfo(Variant::OBJECT, vformat("tile_to_grid_data_%d", i), PROPERTY_HINT_RESOURCE_TYPE, "TileToGridData", PROPERTY_USAGE_DEFAULT));
    }
}

void TileToGridSet::set_terrain_set_terrain_count(int terrain_set_index, int p_value){
    int new_count = p_value;
    while (tile_set->get_terrains_count(terrain_set_index) < new_count) {
        tile_set->add_terrain(terrain_set_index);
        print_line("terrain_set_to_hint_string pre : ", terrain_set_to_hint_string);
        for (TypedArray<Texture2D> terrain_bit_to_icon : terrain_set_to_terrain_bit_to_icon) {
            print_line("terrain_set_to_terrain_bit_to_icon pre : ", terrain_bit_to_icon);
        }
        terrain_set_to_hint_string[terrain_set_index] = (String) terrain_set_to_hint_string[terrain_set_index] + ", " + tile_set->get_terrain_name(terrain_set_index, tile_set->get_terrains_count(terrain_set_index) - 1);
        print_line("terrain_set_to_hint_string post : ", terrain_set_to_hint_string);
        for (TypedArray<Texture2D> terrain_bit_to_icon : terrain_set_to_terrain_bit_to_icon) {
            print_line("terrain_set_to_terrain_bit_to_icon post : ", terrain_bit_to_icon);
        }
        terrain_set_to_terrain_bit_to_icon.get(terrain_set_index).push_back(create_texture_from_color(tile_set->get_terrain_color(terrain_set_index, tile_set->get_terrains_count(terrain_set_index) - 1)));
    }
    while (tile_set->get_terrains_count(terrain_set_index) > new_count) {
        tile_set->remove_terrain(terrain_set_index, tile_set->get_terrains_count(terrain_set_index) - 1);
        print_line("terrain_set_to_hint_string pre : ", terrain_set_to_hint_string);
        for (TypedArray<Texture2D> terrain_bit_to_icon : terrain_set_to_terrain_bit_to_icon) {
            print_line("terrain_set_to_terrain_bit_to_icon pre : ", terrain_bit_to_icon);
        }
        terrain_set_to_hint_string[terrain_set_index] = ((String)terrain_set_to_hint_string[terrain_set_index]).rsplit(",",true, 1)[0]; // Remove last terrain name
        print_line("terrain_set_to_hint_string post : ", terrain_set_to_hint_string);
        for (TypedArray<Texture2D> terrain_bit_to_icon : terrain_set_to_terrain_bit_to_icon) {
            print_line("terrain_set_to_terrain_bit_to_icon post : ", terrain_bit_to_icon);
        }
        terrain_set_to_terrain_bit_to_icon.get(terrain_set_index).pop_back(); // Remove last terrain icon
    }
    notify_property_list_changed();
}

bool TileToGridSet::_get(const StringName &p_name, Variant &r_ret) const {
    String name_str = p_name;
    if (name_str.begins_with("terrain_set_")) {
        PackedStringArray parts = name_str.split("/");
        if (parts.size() >= 2) {
            int terrain_set_index = parts[0].get_slice("_", 2).to_int();
            if (terrain_set_index >= 0 && terrain_set_index < get_terrain_set_count()) {
                String property = parts[1];
                if (property == "mode") {
                    r_ret = tile_set->get_terrain_set_mode(terrain_set_index);
                    return true;
                } else if (property == "terrain_count") {
                    r_ret = tile_set->get_terrains_count(terrain_set_index);
                    return true;
                } else if (property.begins_with("terrain_") && parts.size() >= 3) {
                    int terrain_index = property.get_slice("_", 1).to_int();
                    String terrain_property = parts[2];
                    if (terrain_index >= 0 && terrain_index < tile_set->get_terrains_count(terrain_set_index)) {
                        if (terrain_property == "name") {
                            r_ret = tile_set->get_terrain_name(terrain_set_index, terrain_index);
                            return true;
                        } else if (terrain_property == "color") {
                            r_ret = tile_set->get_terrain_color(terrain_set_index, terrain_index);
                            return true;
                        }
                    }
                }
            }
        }
    } else if (name_str.begins_with("tile_to_grid_data_")) {
        String string_id = name_str.get_slice("_", 4);
        int idx = string_id.to_int();
        // Ensure string_id is a valid int
        if (itos(idx) != string_id){
            return false;
        }
        if (idx >= 0 && idx < tile_to_grid_data_list.size()) {
            r_ret = tile_to_grid_data_list[idx];
            return true;
        }
    }
    return false;
}
bool TileToGridSet::_set(const StringName &p_name, const Variant &p_value) {

    String name_str = p_name;
    if (name_str.begins_with("terrain_set_")) {
        PackedStringArray parts = name_str.split("/");
        if (parts.size() >= 2) {
            int terrain_set_index = parts[0].get_slice("_", 2).to_int();
            if (terrain_set_index >= 0 && terrain_set_index < get_terrain_set_count()) {
                String property = parts[1];

                if (!Engine::get_singleton()->is_editor_hint()) {
                    // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
                    return true;
                }
                if (property == "mode") {
                    tile_set->set_terrain_set_mode(terrain_set_index, (TileSet::TerrainMode)(int)p_value);
                    return true;
                } else if (property == "terrain_count") {
                    set_terrain_set_terrain_count(terrain_set_index, p_value);
                    return true;
                } else if (property.begins_with("terrain_") && parts.size() >= 3) {
                    int terrain_index = property.get_slice("_", 1).to_int();
                    String terrain_property = parts[2];
                    if (terrain_index >= 0 && terrain_index < tile_set->get_terrains_count(terrain_set_index)) {
                        if (terrain_property == "name") {
                            tile_set->set_terrain_name(terrain_set_index, terrain_index, p_value);
                            PackedStringArray hint_string_split = ((String) terrain_set_to_hint_string[terrain_set_index]).split(",");
                            hint_string_split[terrain_index + 1] = p_value;
                            terrain_set_to_hint_string[terrain_set_index] = String(",").join(hint_string_split);
                            notify_tile_data_property_list_changed();
                            return true;
                        } else if (terrain_property == "color") {
                            tile_set->set_terrain_color(terrain_set_index, terrain_index, p_value);
                            terrain_set_to_terrain_bit_to_icon.get(terrain_set_index)[terrain_index + 1] = create_texture_from_color(p_value);
                            notify_tile_data_property_list_changed();
                            return true;
                        }
                    }
                }
            }
        }
    } else if (name_str.begins_with("tile_to_grid_data_")) {

        if (!Engine::get_singleton()->is_editor_hint()) {
            // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
            return true;
        }
        String string_id = name_str.get_slice("_", 4);
        int idx = string_id.to_int();
        // Ensure string_id is a valid int
        if (itos(idx) != string_id){
            return false;
        }

        if (idx >= 0 && idx < tile_to_grid_data_list.size()) {
            Ref<TileToGridData> old_data = tile_to_grid_data_list[idx];
            Ref<TileToGridData> new_data = p_value;
            if (old_data.is_valid()) unsetup_data(old_data);
            tile_to_grid_data_list[idx] = new_data;
            if (new_data.is_valid()) setup_data(new_data);
            return true;
        }
    }
    return false;
}

void  TileToGridSet::notify_tile_data_property_list_changed() {
    for (int i = 0; i < tile_to_grid_data_list.size(); i++) {
        Ref<TileToGridData> data = tile_to_grid_data_list[i];
        if (data.is_valid()) {
            data->notify_property_list_changed();
        }
    }
}

int TileToGridSet::get_tile_to_grid_data_count() const {
    return tile_to_grid_data_list.size();
}

void TileToGridSet::set_tile_to_grid_data_count(int count) {
    tile_to_grid_data_list.resize(count);
    notify_property_list_changed();
}

void TileToGridSet::unsetup_data(Ref<TileToGridData> data) {

    if (!Engine::get_singleton()->is_editor_hint()) {
        // Si nous sommes dans l'éditeur, nous n'avons pas besoin de créer un viewport
        return;
    }
    if (!data.is_valid()) return;

    Ref<TileSetSource> data_source = data->get_tile_set_source();
    for (int i = 0; i < tile_set->get_source_count(); i++) {
        int id = tile_set->get_source_id(i);
        if (tile_set->get_source(id) == data_source) {
            tile_set->remove_source(id);
            break;
        }
    }
}

void TileToGridSet::remove_tile_to_grid_data_by_index(int index) {
    if (index >= 0 && index < tile_to_grid_data_list.size()) {
        Ref<TileToGridData> data = tile_to_grid_data_list[index];
        if (data.is_valid()) {
            unsetup_data(data);
        }
        tile_to_grid_data_list.remove_at(index);
    }
}

void TileToGridSet::remove_tile_to_grid_data_by_name(const String& name) {
    for (int i = 0; i < tile_to_grid_data_list.size(); i++) {
        Ref<TileToGridData> data = tile_to_grid_data_list[i];
        if (data.is_valid() && data->get_name() == name) {
            unsetup_data(data);
            tile_to_grid_data_list.remove_at(i);
            break;
        }
    }
}


int TileToGridSet::find_tile_to_grid_data_by_name(const String& name) const {
    for (int i = 0; i < tile_to_grid_data_list.size(); i++) {
        Ref<TileToGridData> data = tile_to_grid_data_list[i];
        if (data.is_valid() && data->get_name() == name) {
            return i;
        }
    }
    return -1;
}

void TileToGridSet::update_ids() {
    for (int i = 0; i < tile_to_grid_data_list.size(); i++) {
        Ref<TileToGridData> data = tile_to_grid_data_list[i];
        Ref<TileSetSource> tile_set_source = data->get_tile_set_source();
        for(int idx = 0; idx < tile_set->get_source_count(); idx++) {
            int id = tile_set->get_source_id(idx);
            if (tile_set->get_source(id) == tile_set_source) {
                data->set_data_id(tile_set->get_source_id(id));
                break;
            }
        }
    }
}

} // namespace godot