#include "merging_tile_to_grid_data.h"

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/tile_set.hpp>

#include "mesh_utility.h"

namespace godot {

MergingTileToGridData::MergingTileToGridData() : TileToGridData() {
    // Additional initialization specific to merging functionality
}

void MergingTileToGridData::_setup_local_to_scene() {
    print_line("MergingTileToGridData::_setup_local_to_scene : ", get_local_scene());
    init_tile_2D_preview();
}


Transform3D MergingTileToGridData::symmetry_tranform = Transform3D(
    Vector3(0,0,1),
    Vector3(0,1,0), 
    Vector3(1,0,0),
    Vector3(0,0,0)
);

Vector3 vec_mult(const Vector3 &a, const Vector3 &b) {
    return Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
}

TypedDictionary<String, TileItemData> MergingTileToGridData::_generate_tiles_data() {
    print_line("MergingTileToGridData::_generate_full_tiles -- changed_ids : ", changed_ids, " for source id : ", data_id);
    if (_list_count == 0) {
        return TypedDictionary<String, TileItemData>();
    }

    if(!initialised){
        init_tile_2D_preview();
    }
    // Generate all tuples of length k with numbers between 0 and n (inclusive)
    int k = 4;
    TypedArray<PackedInt32Array> all_tuples;
    PackedInt32Array index_tuple;
    PackedInt32Array id_tuple;
    index_tuple.resize(k);
    id_tuple.resize(k);
    index_tuple.fill(-_list_count);
    id_tuple.fill(get_ids(-_list_count));
    if (check_tuple_validity(id_tuple)) {
        // If the tuple is valid, add it to the list
        all_tuples.push_back(id_tuple);
    }

    int idx = 0;
    while (idx < k) {
        // Increment tuple
        if(index_tuple[idx] == _list_count - 1) {
            idx ++;
        } else {
            index_tuple[idx]++;
            id_tuple[idx] = get_ids(index_tuple[idx]);
            for (int j = 0; j < idx; ++j) {
                index_tuple[j] = - _list_count; // Reset all previous indices 
                id_tuple[j] = get_ids(-_list_count); // Reset all previous ids 
            }
            if (check_tuple_validity(id_tuple) && !all_tuples.has(id_tuple)) {
                // If the tuple is valid, add it to the list
                all_tuples.push_back(id_tuple);
            }
            idx = 0;
        }
    }

    TypedDictionary<String, TileItemData> tiles_data;
    // Now we have all valid tuples, we can generate the meshes
    for (const PackedInt32Array &t : all_tuples) {
        Ref<MeshTileItemData> item_data = memnew(MeshTileItemData);
        item_data->item_name = tuple_to_name(t);

        // Create the mesh for the tuple
        TypedArray<ArrayMesh> meshes;
        for (int i = 0; i < t.size(); i++){
            int id = t[i];
            meshes.append(
                transform_mesh_to_destination(
                    get_mesh(id),
                    nullptr,
                    Vector3(-0.25,0,-0.25) + vec_mult(Vector3(0.25,0,0.25).rotated(Vector3(0,1,0), i * Math_PI/2), grid_size), // position
                    0.5 * grid_size,
                    Vector3(0, i * Math_PI / 2, 0) // rotation
                )
            );
        }
        item_data->mesh = merge_meshes_to_destination(meshes, nullptr);

        
        item_data->peering_bits[TileSet::CELL_NEIGHBOR_BOTTOM_SIDE] = get_bottom_bits(t[0]);
        item_data->peering_bits[TileSet::CELL_NEIGHBOR_RIGHT_SIDE] = get_bottom_bits(t[1]);
        item_data->peering_bits[TileSet::CELL_NEIGHBOR_TOP_SIDE] = get_bottom_bits(t[2]);
        item_data->peering_bits[TileSet::CELL_NEIGHBOR_LEFT_SIDE] = get_bottom_bits(t[3]);
        item_data->peering_bits[TileSet::CELL_NEIGHBOR_BOTTOM_RIGHT_CORNER] = get_bottom_right_bits(t[0]);
        item_data->peering_bits[TileSet::CELL_NEIGHBOR_TOP_RIGHT_CORNER] = get_bottom_right_bits(t[1]);
        item_data->peering_bits[TileSet::CELL_NEIGHBOR_TOP_LEFT_CORNER] = get_bottom_right_bits(t[2]);
        item_data->peering_bits[TileSet::CELL_NEIGHBOR_BOTTOM_LEFT_CORNER] = get_bottom_right_bits(t[3]);
        item_data->center_bit = get_center_bit();
        tiles_data[item_data->item_name] = item_data;
    }
    return tiles_data;
}

void MergingTileToGridData::_bind_methods() {
    
    ClassDB::bind_method(D_METHOD("get_center_bit"), &MergingTileToGridData::get_center_bit);
    ClassDB::bind_method(D_METHOD("set_center_bit", "center_bit"), &MergingTileToGridData::set_center_bit);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "center_bit", PROPERTY_HINT_ENUM, "", PROPERTY_USAGE_DEFAULT), "set_center_bit", "get_center_bit");

    ClassDB::bind_method(D_METHOD("set_list_count", "count"), &MergingTileToGridData::set_list_count);
    ClassDB::bind_method(D_METHOD("get_list_count"), &MergingTileToGridData::get_list_count);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "mesh_count", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_ARRAY | PROPERTY_USAGE_DEFAULT, "Meshes,mesh_"), "set_list_count", "get_list_count");
    
}
void MergingTileToGridData::_get_property_list(List<PropertyInfo> *p_list) const {



    String hint_string;
    if (terrain_set < terrain_set_to_hint_string.size() && terrain_set >= 0) {
        hint_string = terrain_set_to_hint_string[terrain_set] + String("|_terrain_bit_to_icon");
    } else {
        hint_string = "No terrain : -1|_terrain_bit_to_icon"; // Default to -1 if terrain_set is out of bounds
    }


    for (int i = 0; i < _list_count; ++i) {
        Array arr_i;
        arr_i.push_back(i);
        p_list->push_back(PropertyInfo(Variant::INT, (base_name + ids_property_name).format(arr_i), PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY));
        p_list->push_back(PropertyInfo(Variant::OBJECT, (base_name + mesh_property_name).format(arr_i), PROPERTY_HINT_RESOURCE_TYPE, "ArrayMesh", PROPERTY_USAGE_DEFAULT));
        p_list->push_back(PropertyInfo(Variant::VECTOR3, (base_name + rotation_property_name).format(arr_i), PROPERTY_HINT_RANGE , "-180,180,0.1,or_less,or_greater,radians_as_degrees", PROPERTY_USAGE_DEFAULT));
        p_list->push_back(PropertyInfo(Variant::BOOL, (base_name + generate_symmetric_property_name).format(arr_i), PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT));
        p_list->push_back(PropertyInfo(Variant::INT, (base_name + bottom_bits_property_name).format(arr_i), PROPERTY_HINT_ENUM, hint_string, PROPERTY_USAGE_DEFAULT, "_terrain_bit_to_icon"));
        p_list->push_back(PropertyInfo(Variant::INT, (base_name + right_bits_property_name).format(arr_i), PROPERTY_HINT_ENUM, hint_string, PROPERTY_USAGE_DEFAULT, "_terrain_bit_to_icon"));
        p_list->push_back(PropertyInfo(Variant::INT, (base_name + bottom_right_bits_property_name).format(arr_i), PROPERTY_HINT_ENUM, hint_string, PROPERTY_USAGE_DEFAULT, "_terrain_bit_to_icon"));
        p_list->push_back(PropertyInfo(Variant::OBJECT, (base_name + tile_2D_preview_property_name).format(arr_i), PROPERTY_HINT_RESOURCE_TYPE, "MeshPreviewTexture", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY));
        
        // Check if symmetric generation is enabled for this tile using ID
        int id = ids[i];
        if (_generate_symmetric[id]) {
            p_list->push_back(PropertyInfo(Variant::OBJECT, (base_name + tile_2D_preview_symmetric_property_name).format(arr_i), PROPERTY_HINT_RESOURCE_TYPE, "MeshPreviewTexture", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY));
        }
    }
}

void MergingTileToGridData::_validate_property(PropertyInfo &p_property) const {
    if (p_property.name == StringName("terrain_set")) {
            p_property.hint_string = terrain_set_hint_string;
    }

    if (p_property.name == StringName("center_bit")) {
        if (terrain_set < terrain_set_to_hint_string.size() && terrain_set >= 0) {
            p_property.hint_string = terrain_set_to_hint_string[terrain_set] + String("|_terrain_bit_to_icon");
        } else {
            p_property.hint_string = "No terrain : -1|_terrain_bit_to_icon"; // Default to -1 if terrain_set is out of bounds
        }
    }
}


bool MergingTileToGridData::_get(const StringName &p_name, Variant &r_ret) const {
    if (p_name.begins_with("mesh_")) {
        String string_index = p_name.get_slice("_", 1).get_slice("/", 0);
        int index = string_index.to_int();
        if (string_index != itos(index)) {
            print_line("MergingTileToGridData::_get: Invalid index format: ", string_index, " for name: ", p_name);
            return false; // Invalid index format
        }
        if (index >= 0 && index < _list_count) {
            int id = ids[index];
            String variable = p_name.get_slice("/", 1);
            if (variable == mesh_property_name) {
                r_ret = _meshes[id];
            } else if (variable == rotation_property_name) {
                Transform3D transform = _transforms[id];
                r_ret = transform.basis.get_euler();
            } else if (variable == generate_symmetric_property_name) {
                r_ret = _generate_symmetric[id];
            } else if (variable == tile_2D_preview_property_name) {
                r_ret = _tile_2D_previews[id];
            } else if (variable == tile_2D_preview_symmetric_property_name) {
                r_ret = _tile_2D_previews_symmetric[id];
            } else if (variable == bottom_bits_property_name) {
                r_ret = bottom_bits[id];
            } else if (variable == right_bits_property_name) {
                r_ret = right_bits[id];
            } else if (variable == bottom_right_bits_property_name) {
                r_ret = bottom_right_bits[id];
            } else if (variable == ids_property_name){
                r_ret = id;
            } else {
                return false;
            }
            return true;
        }
    }
    return false;
}



bool MergingTileToGridData::_set(const StringName &p_name, const Variant &p_value) {
    if (p_name.begins_with("mesh_")) {
        String string_index = p_name.get_slice("_", 1).get_slice("/", 0);
        int index = string_index.to_int();
        if (string_index != itos(index)) {
            print_line("MergingTileToGridData::_set: Invalid index format: ", string_index, " for name: ", p_name);
            return false; // Invalid index format
        }
        if (index >= 0 && index < _list_count) {
            int id = ids[index];
            String variable = p_name.get_slice("/", 1);
            print_line("MergingTileToGridData::_set: Setting ", variable," for index ", index, " (id ", id, ") to ", p_value);
            if (variable == mesh_property_name) {
                Ref<ArrayMesh> current_mesh = _meshes[id];
                if(current_mesh == p_value) {
                    return true; // No change
                }
                if (current_mesh.is_valid()) {
                    current_mesh->disconnect("changed", callable_mp(this, &MergingTileToGridData::notify_changed_id));
                }
                _meshes[id] = p_value;
                Ref<ArrayMesh> new_mesh = _meshes[id];
                if (new_mesh.is_valid()) {
                    new_mesh->connect("changed", callable_mp(this, &MergingTileToGridData::update_transformed_mesh).bind(id));
                }
                update_transformed_mesh(id);

            } else if (variable == rotation_property_name) {
                Transform3D transform = _transforms[id];
                if (transform.basis.get_euler() == p_value) {
                    return true; // No change
                }
                transform.basis.set_euler(p_value);
                _transforms[id] = transform;
                
                update_transformed_mesh(id);
                    
            } else if (variable == generate_symmetric_property_name) {
                bool current_value = _generate_symmetric[id];
                bool new_value = p_value;
                if (current_value == new_value) {
                    return true; // No change
                }
                _generate_symmetric[id] = new_value;
                notify_property_list_changed();
            } else if (variable == right_bits_property_name) {
                right_bits[id] = p_value;
            } else if (variable == bottom_bits_property_name) {
                bottom_bits[id] = p_value;
            } else if (variable == bottom_right_bits_property_name) {
                bottom_right_bits[id] = p_value;
            } else if (variable == ids_property_name) {
                int new_id = p_value;
                if (id == new_id) {
                    return true; // No change
                }
                // ERR_FAIL_COND_V_MSG(ids.has(new_id), true, "MergingTileToGridData::_set: Trying to set an id at index : " + itos(index) + " that is already in use: " + itos(new_id) + ". Please choose a different id. Here are the current ids: " + Variant(ids).stringify());
                // IDS are never to be changed, if they are it actually means the id is moved to a new index. 
                ids[index] = new_id;
                
            } else {
                return false;
            }
            request_generation();
            return true;
        }
    }
    return false;
}

String MergingTileToGridData::tuple_to_name(PackedInt32Array tuple){
    String item_name;
    for (int id : tuple){
        item_name += "_" + itos(id);
    }
    return item_name;
}


bool is_less(const PackedInt32Array &a, const PackedInt32Array &b) {
    for (int i = 0; i < 4; ++i) {
        if (a[i] < b[i]) return true;
        if (a[i] > b[i]) return false;
    }
    return false;
};

bool MergingTileToGridData::check_tuple_validity(PackedInt32Array tuple) {
    ERR_FAIL_COND_V_MSG(tuple.size() != 4, false, "MergingTileToGridData::check_tuple_validity: Tuple must have exactly 4 elements.");
    int a = tuple[0];
    int b = tuple[1];
    int c = tuple[2];
    int d = tuple[3];
    if (a < 0 && !_generate_symmetric[-a]) {
        // trying to use a symmetric tile but symmetric has not been activated for this tile
        return false;
    }
    if (b < 0 && !_generate_symmetric[-b]) {
        // trying to use a symmetric tile but symmetric has not been activated for this tile
        return false;
    }
    if (c < 0 && !_generate_symmetric[-c]) {
        // trying to use a symmetric tile but symmetric has not been activated for this tile
        return false;
    }
    if (d < 0 && !_generate_symmetric[-d]) {
        // trying to use a symmetric tile but symmetric has not been activated for this tile
        return false;
    }

    if (!get_mesh(a).is_valid() || get_mesh(a)->get_surface_count() == 0) {
        // The mesh for the first tile is not valid or has no surfaces
        return false;
    }
    if (!get_mesh(b).is_valid() || get_mesh(b)->get_surface_count() == 0) {
        // The mesh for the second tile is not valid or has no surfaces
        return false;
    }
    if (!get_mesh(c).is_valid() || get_mesh(c)->get_surface_count() == 0) {
        // The mesh for the third tile is not valid or has no surfaces
        return false;
    }
    if (!get_mesh(d).is_valid() || get_mesh(d)->get_surface_count() == 0) {
        // The mesh for the fourth tile is not valid or has no surfaces
        return false;
    }
    
    // Check if the tuple is valid
    // A tuple is valid if it is the lowest lexicographic tuple
    // Check all rotations of the tuple to ensure this is the lowest lexicographically
    PackedInt32Array tuple1, tuple2, tuple3;
    tuple1.resize(4); tuple2.resize(4); tuple3.resize(4);
    tuple1[0] = b; tuple1[1] = c; tuple1[2] = d; tuple1[3] = a;
    tuple2[0] = c; tuple2[1] = d; tuple2[2] = a; tuple2[3] = b;
    tuple3[0] = d; tuple3[1] = a; tuple3[2] = b; tuple3[3] = c;


    
    if (is_less(tuple1, tuple) || is_less(tuple2, tuple) || is_less(tuple3, tuple)) {
        // The tuple is not the lowest rotated tuple
        return false;
    }



    if ( get_right_bits(a) != get_bottom_bits(b) ||
    get_right_bits(b) != get_bottom_bits(c) ||
    get_right_bits(c) != get_bottom_bits(d) ||
    get_right_bits(d) != get_bottom_bits(a) ) {
        // The tuple has mismatched bits
        return false;
    }
    return true;
}


Ref<ArrayMesh> MergingTileToGridData::get_mesh(int id) const{
    if (id < 0) {
        id = -id; // Convert negative id to positive index
        ERR_FAIL_COND_V_MSG(!_generate_symmetric[id], nullptr, "MergingTileToGridData::get_mesh: Trying to get mesh for the symmetric of a tile where it is not enabled.");
        return Object::cast_to<ArrayMesh>(transformed_symmetric_meshes[id]);
    } else {
        return Object::cast_to<ArrayMesh>(transformed_meshes[id]);
    }
}
int MergingTileToGridData::get_ids(int index) const{
    ERR_FAIL_COND_V_MSG(index < -_list_count || index >= _list_count, 0, "MergingTileToGridData::get_ids: Index : " + itos(index) + " out of bounds. Valid range is [-" + itos(_list_count) + ", " + itos(_list_count - 1) + "].");
    if (index < 0) {
        index += _list_count; // Convert negative index to positive index
        return - get_ids(index);
    }
    return ids[index];
}
int MergingTileToGridData::get_bottom_right_bits(int id) const{
    if (id < 0) {
        id = -id; // Convert negative id to positive index
        ERR_FAIL_COND_V_MSG(!_generate_symmetric[id], -1, "MergingTileToGridData::get_bottom_right_bits: Trying to get bits for the symmetric of a tile where it is not enabled.");
    } 
    return bottom_right_bits[id];
}
int MergingTileToGridData::get_bottom_bits(int id) const {
    if (id < 0) {
        id = -id; // Convert negative id to positive index
        ERR_FAIL_COND_V_MSG(!_generate_symmetric[id], -1, "MergingTileToGridData::get_bottom_bits: Trying to get bottom bits for the symmetric of a tile where it is not enabled.");
        return right_bits[id];
    } else {
        return bottom_bits[id];
    }
}
int MergingTileToGridData::get_right_bits(int id) const {
    if (id < 0) {
        id = -id; // Convert negative id to positive index
        ERR_FAIL_COND_V_MSG(!_generate_symmetric[id], -1, "MergingTileToGridData::get_right_bits: Trying to get right bits for the symmetric of a tile where it is not enabled.");
        return bottom_bits[id];
    } else {
        return right_bits[id];
    }
}


int MergingTileToGridData::get_list_count() const {
    return _list_count;
}
void MergingTileToGridData::set_list_count(int count) {
    _list_count = count;
    ids.resize(count);
    
    int last_known_used_id = 0;
    // Initialize IDs if needed
    for (int i = 0; i < count; i++) {
        if ((int)ids[i] == 0) {
            // Find next available ID
            while (ids.has(last_known_used_id)) {
                last_known_used_id++;
            }
            ids[i] = last_known_used_id;
        }
        
        int id = ids[i];
        // Initialize dictionary entries if they don't exist
        if (!_meshes.has(id)) {
            _meshes[id] = Ref<ArrayMesh>();
        }
        if (!_transforms.has(id)) {
            _transforms[id] = Transform3D();
        }
        if (!_generate_symmetric.has(id)) {
            _generate_symmetric[id] = false;
        }
        if (!_tile_2D_previews.has(id)) {
            _tile_2D_previews[id] = Ref<MeshPreviewTexture>();
        }
        if (!_tile_2D_previews_symmetric.has(id)) {
            _tile_2D_previews_symmetric[id] = Ref<MeshPreviewTexture>();
        }
        if (!bottom_bits.has(id)) {
            bottom_bits[id] = 0;
        }
        if (!right_bits.has(id)) {
            right_bits[id] = 0;
        }
        if (!bottom_right_bits.has(id)) {
            bottom_right_bits[id] = 0;
        }
        if (!transformed_meshes.has(id)) {
            transformed_meshes[id] = Ref<TransformedMesh>();
        }
        if (!transformed_symmetric_meshes.has(id)) {
            transformed_symmetric_meshes[id] = Ref<TransformedMesh>();
        }
    }
    init_tile_2D_preview();
    notify_property_list_changed();
}


void MergingTileToGridData::set_center_bit(int p_bit) {
    center_bit = p_bit;
}
int MergingTileToGridData::get_center_bit() const {
    return center_bit;
}

void MergingTileToGridData::init_tile_2D_preview() {
    if (!get_local_scene()) {
        return;
    }
    print_line("MergingTileToGridData::init_tile_2D_preview for source id : ", data_id);
    for (int id : ids) {
        if(!((Ref<MeshPreviewTexture>)_tile_2D_previews[id]).is_valid()) {
            Ref<ArrayMesh> mesh = (Ref<ArrayMesh>)_meshes[id];
            if (!mesh.is_valid()) {
                mesh.instantiate();
                ERR_FAIL_COND_MSG(!mesh.is_valid(), "MergingTileToGridData::init_tile_2D_preview: Mesh is not valid for id " + itos(id) + ". Please set a valid mesh.");
            }
            Ref<MeshPreviewTexture> tile_2D_preview = memnew(MeshPreviewTexture);
            tile_2D_preview->set_width(64 * EditorInterface::get_singleton()->get_editor_scale());
            tile_2D_preview->set_rotation(Vector3(-Math_PI / 2, 0, 0));
            transformed_meshes[id] = transform_mesh_to_destination(
                mesh,
                nullptr,
                _transforms[id]
            );
            tile_2D_preview->set_mesh(transformed_meshes[id]);
            tile_2D_preview->setup_local_to_scene();
            _tile_2D_previews[id] = tile_2D_preview;

            Ref<MeshPreviewTexture> tile_2D_preview_symmetric = memnew(MeshPreviewTexture);
            tile_2D_preview_symmetric->set_width(64 * EditorInterface::get_singleton()->get_editor_scale());
            tile_2D_preview_symmetric->set_rotation(Vector3(-Math_PI / 2, 0, 0));
            transformed_symmetric_meshes[id] = transform_mesh_to_destination(
                transformed_meshes[id],
                nullptr,
                symmetry_tranform
            );
            tile_2D_preview_symmetric->set_mesh(transformed_symmetric_meshes[id]);
            tile_2D_preview_symmetric->setup_local_to_scene();
            _tile_2D_previews_symmetric[id] = tile_2D_preview_symmetric;
        }

    }
    initialised = true;
}

void MergingTileToGridData::update_transformed_mesh(int id) {
    if (!initialised) {
        return; // Ensure the tile 2D preview is initialized before updating transformed meshes
    }
    transform_mesh_to_destination(
        _meshes[id],
        transformed_meshes[id],
        _transforms[id]
    );
    transform_mesh_to_destination(
        transformed_meshes[id],
        transformed_symmetric_meshes[id],
        symmetry_tranform
    );

    notify_changed_id(id);
}


void MergingTileToGridData::notify_changed_id(int id) {
    if (!changed_ids.has(id)) {
        changed_ids.append(id);
        request_generation();
    }
}

bool MergingTileToGridData::is_id_changed(int id) const {
    if (id < 0) {
        id = -id; // Convert negative index to positive index
        ERR_FAIL_COND_V_MSG(!_generate_symmetric[id], true, "MergingTileToGridData::is_id_changed: Trying to check if a symmetric tile is changed, but it is not enabled.");
    }
    return changed_ids.has(id);
}



MergingTileToGridData::~MergingTileToGridData() {
    // Cleanup if needed
}
} // namespace godot
