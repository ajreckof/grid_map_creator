#include "scene_tile_to_grid_data.h"

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/tile_set.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

namespace godot {

SceneTileToGridData::SceneTileToGridData() : TileToGridData() {
    // Additional initialization specific to scene functionality
}

SceneTileToGridData::~SceneTileToGridData() {
    // Cleanup if needed
}

TypedDictionary<String, TileItemData> SceneTileToGridData::_generate_tiles_data() {
    print_line("SceneTileToGridData::_generate_full_tiles");
    if (_tiles_count == 0) {
        return TypedDictionary<String, TileItemData>();
    }
    TypedDictionary<String, TileItemData> tiles_data;


    if(!initialised){
        init_tile_2D_preview();
    }
    
    for (int i = 0; i < _tiles_count; ++i) {
        Ref<SceneTileItemData> item_data = memnew(SceneTileItemData);
        item_data->item_name = _unique_names[i];
        item_data->scene = _scenes[_unique_names[i]];
        tiles_data[_unique_names[i]] = item_data;
    }
    return tiles_data;
}

void SceneTileToGridData::_setup_local_to_scene() {
    print_line("SceneTileToGridData::_setup_local_to_scene : ", get_local_scene());
    init_tile_2D_preview();
}

void SceneTileToGridData::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_tiles_count", "count"), &SceneTileToGridData::set_tiles_count);
    ClassDB::bind_method(D_METHOD("get_tiles_count"), &SceneTileToGridData::get_tiles_count);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "tiles_count", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_ARRAY | PROPERTY_USAGE_DEFAULT, "Tiles,tiles_"), "set_tiles_count", "get_tiles_count");
}

void SceneTileToGridData::_get_property_list(List<PropertyInfo> *p_list) const {
    String hint_string = "";
    if (terrain_set < terrain_set_to_hint_string.size()) {
        hint_string = terrain_set_to_hint_string[terrain_set];
    }

    for (int i = 0; i < _tiles_count; ++i) {
        Array arr_i;
        arr_i.push_back(i);
        String property_base = base_name.format(arr_i);
        
        p_list->push_back(PropertyInfo(Variant::STRING, property_base + unique_name_property_name, PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT));
        p_list->push_back(PropertyInfo(Variant::OBJECT, property_base + scene_property_name, PROPERTY_HINT_RESOURCE_TYPE, "PackedScene", PROPERTY_USAGE_DEFAULT));
        
        // Add preview (read-only)
        p_list->push_back(PropertyInfo(Variant::OBJECT, property_base + preview_texture_property_name, PROPERTY_HINT_RESOURCE_TYPE, "ScenePreviewTexture", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY));
    }
}

void SceneTileToGridData::_validate_property(PropertyInfo &p_property) const {
    if (p_property.name == StringName("terrain_set")) {
        if (terrain_set_hint_string.length() > 0) {
            p_property.hint_string = terrain_set_hint_string;
        }
    }
}

bool SceneTileToGridData::_get(const StringName &p_name, Variant &r_ret) const {
    if (p_name.begins_with("tiles_")) {
        String string_index = p_name.get_slice("_", 1).get_slice("/", 0);
        int index = string_index.to_int();
        if (string_index != itos(index)) {
            print_line("SceneTileToGridData::_get: Invalid index format: ", string_index, " for name: ", p_name);
            return false;
        }
        if (index >= 0 && index < _tiles_count) {
            String variable = p_name.get_slice("/", 1);
            String unique_name = _unique_names[index];
            
            if (variable == unique_name_property_name) {
                r_ret = unique_name;
            } else if (variable == scene_property_name) {
                r_ret = _scenes.get(unique_name, Ref<PackedScene>());
            } else if (variable == preview_texture_property_name) {
                r_ret = _preview_textures[unique_name];
            } else {
                return false;
            }
            return true;
        }
    }
    return false;
}

bool SceneTileToGridData::_set(const StringName &p_name, const Variant &p_value) {
    if (p_name.begins_with("tiles_")) {
        String string_index = p_name.get_slice("_", 1).get_slice("/", 0);
        int index = string_index.to_int();
        if (string_index != itos(index)) {
            print_line("SceneTileToGridData::_set: Invalid index format: ", string_index, " for name: ", p_name);
            return false;
        }
        if (index >= 0 && index < _tiles_count) {
            String variable = p_name.get_slice("/", 1);
            String current_unique_name = _unique_names[index];
            
            print_line("SceneTileToGridData::_set: Setting ", variable," for index ", index, " to ", p_value);
            
            if (variable == unique_name_property_name) {
                String new_name = p_value;
                if (current_unique_name == new_name) {
                    if (renames.has(index)) {
                        renames.erase(index); // Remove if already registered
                    }
                    return true; // No change
                }
                
                // register the renaming to do it at end of frame
                callable_mp(this, &SceneTileToGridData::_rename_items).call_deferred();
                renames[index] = new_name; // Store the new name for later renaming
                
            } else if (variable == scene_property_name) {
                Ref<PackedScene> current_scene = _scenes.get(current_unique_name, Ref<PackedScene>());
                if(current_scene == p_value) {
                    return true; // No change
                }
                _scenes[current_unique_name] = p_value;
                if (initialised) {
                    _preview_textures[current_unique_name]->set_packed_scene(p_value);
                }
            } else {
                return false;
            }
            
            request_generation();
            return true;
        }
    }
    return false;
}


int SceneTileToGridData::get_tiles_count() const {
    return _tiles_count;
}

void SceneTileToGridData::set_tiles_count(int count) {
    _tiles_count = count;
    _unique_names.resize(count);

    // Initialize names if needed
    for (int i = 0; i < count; i++) {
        if (i >= _unique_names.size() || _unique_names[i] == "") {
            int j = 0;
            while (_unique_names.has("unamed_tile_" + itos(j))) {
                j++;
            }
            _unique_names[i] = "unamed_tile_" + itos(j);
            _scenes[_unique_names[i]] = Ref<PackedScene>();
            _preview_textures[_unique_names[i]] = (Ref<ScenePreviewTexture>) memnew(ScenePreviewTexture);
        }
    }
    
    notify_property_list_changed();
    request_generation();
}



void SceneTileToGridData::init_tile_2D_preview() {
    if (!get_local_scene()) {
        return;
    }
    print_line("SceneTileToGridData::init_tile_2D_preview for source id : ", data_id);
    initialised = true;
    for (int i = 0; i < _tiles_count; ++i) {
        String unique_name = _unique_names[i];
        if (_preview_textures.has(unique_name)) {
            continue; // Already initialized
        }
        Ref<ScenePreviewTexture> preview_texture = memnew(ScenePreviewTexture);
        preview_texture->set_packed_scene(_scenes[unique_name]);
        

        _preview_textures[unique_name] = preview_texture;

    }
}


void SceneTileToGridData::_rename_items() {
    if (renames.is_empty()) {
        return;
    }
    // create the array of the new names 
    Array new_names;
    for (int i = 0; i < _tiles_count; ++i) {
        if (renames.has(i)) {
            new_names.push_back(renames[i]);
        } else {
            new_names.push_back(_unique_names[i]);
        }
    }
    // find duplicates and rename the one that is a change of name
    for (int i = 0; i < _tiles_count; ++i) {
        String new_name = new_names[i];
        if (new_name == _unique_names[i]) {
            continue; // No change
        }
        if (new_names.count(new_name) > 1) {
            // Rename the one that is a change of name
            int j = 0;
            while (new_names.has(new_name + "_" + itos(j))) {
                j++;
            }
            new_names[i] = new_name + "_" + itos(j);
        }
    }

    HashMap<String, Ref<ArrayMesh>> new_meshes;
    HashMap<String, Ref<PackedScene>> new_scenes;
    HashMap<String, HashMap<TileSet::CellNeighbor, int>> new_peering_bits;
    HashMap<String, int> new_center_bits;
    HashMap<String, ScenePreviewTexture> new_preview_textures;
    for (int i = 0; i < _tiles_count; ++i) {
        String new_name = new_names[i];
        String old_name = _unique_names[i];
        new_scenes[new_name] = _scenes[old_name];
    }

    renames.clear();
    notify_property_list_changed();
}


} // namespace godot