#include "tile_to_grid_data.h"

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/gradient.hpp>
#include <godot_cpp/classes/gradient_texture2d.hpp>
#include <godot_cpp/classes/tile_data.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/tile_data.hpp>
#include <godot_cpp/classes/engine.hpp>
#include "preview_texture/mesh_preview_texture.h"
#include "preview_texture/scene_preview_texture.h"

#include "mesh_utility/merged_mesh.h"
#include "merged_image.h"

namespace godot {// namespace godot
    const char* MESH_NAME_LAYER = "MeshName";
    const char* ROTATION_X_LAYER = "Rotation_X";
    const char* ROTATION_Y_LAYER = "Rotation_Y";
    const char* ROTATION_Z_LAYER = "Rotation_Z";
    const char* SCENE_NAME_LAYER = "SceneName";

TileToGridData::TileToGridData() : Resource() {
    set_local_to_scene(true);
    collision_shape = (Ref<BoxShape3D>) memnew(BoxShape3D);
    collision_shapes = Array();
    collision_shapes.append(collision_shape);
    collision_shapes.append(Transform3D());
}

TileToGridData::~TileToGridData() {} 

void TileToGridData::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_data_id"), &TileToGridData::get_data_id);
    ClassDB::bind_method(D_METHOD("set_data_id", "data_id"), &TileToGridData::set_data_id);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "data_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_data_id", "get_data_id");   

    ClassDB::bind_method(D_METHOD("get_terrain_set"), &TileToGridData::get_terrain_set);
    ClassDB::bind_method(D_METHOD("set_terrain_set", "terrain_set"), &TileToGridData::set_terrain_set);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "terrain_set", PROPERTY_HINT_ENUM , "", PROPERTY_USAGE_DEFAULT),"set_terrain_set", "get_terrain_set");
    
    
    ClassDB::bind_method(D_METHOD("get_name"), &TileToGridData::get_name);
    ClassDB::bind_method(D_METHOD("set_name", "name"), &TileToGridData::set_name);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "name", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_name", "get_name");
    

    ClassDB::bind_method(D_METHOD("get_tile_set_source"), &TileToGridData::get_tile_set_source);
    ClassDB::bind_method(D_METHOD("set_tile_set_source", "tile_set_source"), &TileToGridData::set_tile_set_source);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "tile_set_source", PROPERTY_HINT_RESOURCE_TYPE, "TileSetAtlasSource", PROPERTY_USAGE_NONE), "set_tile_set_source", "get_tile_set_source");
    
    ClassDB::bind_method(D_METHOD("get_grid_size"), &TileToGridData::get_grid_size);
    ClassDB::bind_method(D_METHOD("set_grid_size", "grid_size"), &TileToGridData::set_grid_size);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "grid_size", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_grid_size", "get_grid_size");

    ClassDB::bind_method(D_METHOD("get_tile_size"), &TileToGridData::get_tile_size);
    ClassDB::bind_method(D_METHOD("set_tile_size", "tile_size"), &TileToGridData::set_tile_size);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "tile_size", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_tile_size", "get_tile_size");

    ClassDB::bind_method(D_METHOD("get_terrain_set_hint_string"), &TileToGridData::get_terrain_set_hint_string);
    ClassDB::bind_method(D_METHOD("set_terrain_set_hint_string", "terrain_set_hint_string"), &TileToGridData::set_terrain_set_hint_string);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "terrain_set_hint_string", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_terrain_set_hint_string", "get_terrain_set_hint_string");

    ClassDB::bind_method(D_METHOD("get_terrain_set_to_hint_string"), &TileToGridData::get_terrain_set_to_hint_string);
    ClassDB::bind_method(D_METHOD("set_terrain_set_to_hint_string", "terrain_set_to_hint_string"), &TileToGridData::set_terrain_set_to_hint_string);
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "terrain_set_to_hint_string", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_terrain_set_to_hint_string", "get_terrain_set_to_hint_string");

    ClassDB::bind_method(D_METHOD("get_mesh_library"), &TileToGridData::get_mesh_library);
    ClassDB::bind_method(D_METHOD("set_mesh_library", "mesh_library"), &TileToGridData::set_mesh_library);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh_library", PROPERTY_HINT_RESOURCE_TYPE, "MeshLibrary", PROPERTY_USAGE_NONE), "set_mesh_library", "get_mesh_library");
    

    ClassDB::bind_method(D_METHOD("get_terrain_bit_to_icon"), &TileToGridData::get_terrain_bit_to_icon);
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "_terrain_bit_to_icon", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_READ_ONLY), "", "get_terrain_bit_to_icon");

    ClassDB::bind_method(D_METHOD("set_name_to_row_in_source", "name_to_row_in_source"), &TileToGridData::set_name_to_row_in_source);
    ClassDB::bind_method(D_METHOD("get_name_to_row_in_source"), &TileToGridData::get_name_to_row_in_source);
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "name_to_row_in_source", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_name_to_row_in_source", "get_name_to_row_in_source");
}




Ref<MeshLibrary> TileToGridData::get_mesh_library(){
    return mesh_library;
}
void TileToGridData::set_mesh_library(Ref<MeshLibrary> p_mesh_library){
    mesh_library = p_mesh_library;
    request_generation();
}

Vector3 TileToGridData::get_grid_size() const {
    return grid_size;
}
void TileToGridData::set_grid_size(Vector3 p_grid_size) {
    if (p_grid_size == grid_size) {
        return; // No change
    }
    grid_size = p_grid_size;
    collision_shape->set_size(grid_size);
    collision_shapes[1] = Transform3D(Basis(), Vector3(0,grid_size.y * 0.5,0)); // Update the transform to match the new grid size
    request_generation();
}

Vector2i TileToGridData::get_tile_size() const {
    return tile_size;
}
void TileToGridData::set_tile_size(Vector2i p_tile_size) {
    if (p_tile_size == tile_size) {
        return; // No change
    }
    tile_size = p_tile_size;
    request_generation();
}

Vector<TypedArray<Texture2D>> TileToGridData::get_terrain_set_to_terrain_bit_to_icon() const {
    return terrain_set_to_terrain_bit_to_icon;
}
void TileToGridData::set_terrain_set_to_terrain_bit_to_icon(Vector<TypedArray<Texture2D>> p_terrain_set_to_terrain_bit_to_icon) {
    if (p_terrain_set_to_terrain_bit_to_icon == terrain_set_to_terrain_bit_to_icon) {
        return; // No change
    }
    terrain_set_to_terrain_bit_to_icon = p_terrain_set_to_terrain_bit_to_icon;
    notify_property_list_changed();
}

TypedArray<String> TileToGridData::get_terrain_set_to_hint_string() const {
    return terrain_set_to_hint_string;
}
void TileToGridData::set_terrain_set_to_hint_string(TypedArray<String> p_terrain_set_to_hint_string) {
    if (p_terrain_set_to_hint_string == terrain_set_to_hint_string) {
        return; // No change
    }
    terrain_set_to_hint_string = p_terrain_set_to_hint_string;
    notify_property_list_changed();
}

String TileToGridData::get_terrain_set_hint_string() const {
    return terrain_set_hint_string;
}
void TileToGridData::set_terrain_set_hint_string(String p_terrain_set_hint_string) {
    if (p_terrain_set_hint_string == terrain_set_hint_string) {
        return; // No change
    }
    terrain_set_hint_string = p_terrain_set_hint_string;
    notify_property_list_changed();
}

Ref<TileSetSource> TileToGridData::get_tile_set_source(){
    return tile_set_source;
}

void TileToGridData::set_tile_set_source(Ref<TileSetSource> p_tile_set_source){
    if (!p_tile_set_source.is_valid()) {
        ERR_FAIL_MSG("TileSetSource is not valid.");
    }
    tile_set_source = p_tile_set_source;
    tile_set_source->set_name(name);
    request_generation();
}

String TileToGridData::get_name(){
    return name;
}
void TileToGridData::set_name(String p_name){
    if (mesh_library.is_valid()){
        for (int i : mesh_library->get_item_list()) {
            if (mesh_library->get_item_name(i).begins_with(name + "_" + itos(data_id))) {
                // If the item name starts with the name of this TileToGridData, it is a generated tile
                mesh_library->set_item_name(i, p_name + mesh_library->get_item_name(i).substr(name.length()));
            }
        }
    }
    if(tile_set_source.is_valid()){
        tile_set_source->set_name(p_name);
        for (int index = 0; index < tile_set_source->get_tiles_count(); index++) {
            Vector2i atlas_coords = tile_set_source->get_tile_id(index); 
            TileData *data = tile_set_source->get_tile_data(atlas_coords, 0);
            data->set_custom_data(MESH_NAME_LAYER, p_name + ((String)data->get_custom_data(MESH_NAME_LAYER)).substr(name.length()));
        }
    }
    name = p_name;
    request_generation();
}

int TileToGridData::get_data_id() const {
    return data_id;
}
void TileToGridData::set_data_id(int p_data_id) {
    if (p_data_id == data_id) {
        return; // No change
    }
    data_id = p_data_id;
}
void TileToGridData::set_name_to_row_in_source(TypedDictionary<String, int> p_name_to_row_in_source) {
    if (!tile_set_source.is_valid()) {
        name_to_row_in_source = p_name_to_row_in_source;
        return; // No tile set source, nothing to do
    }
    for (int row : name_to_row_in_source.values()) {
        tile_set_source->remove_tile(Vector2i(0, row));
    }   
    name_to_row_in_source = p_name_to_row_in_source;
}

TypedDictionary<String, int> TileToGridData::get_name_to_row_in_source() const {
    return name_to_row_in_source;
}



void TileToGridData::generate_full_tiles(){
    generation_requested = false;
    print_line("TileToGridData::generate_full_tiles for source id : ", data_id);

    if (!tile_set_source.is_valid() || !mesh_library.is_valid() || !get_local_scene()){
        print_line("stopping early because it is not initialised yet");
        print_line("tile_set_source : ", tile_set_source);
        print_line("mesh_library : ", mesh_library);
        print_line(" get_local_scene() : ", get_local_scene() );
        return;
    }

    TypedDictionary<String, TileItemData> tiles_data = _generate_tiles_data();
    // delete item not in use anymore
    if (tiles_data.size() == 0) {
        print_line("TileToGridData::_generate_full_tiles: No tiles data generated, stopping early.");
        return;
    }

    for (int i : mesh_library->get_item_list()) {
        if (mesh_library->get_item_name(i).begins_with(name + "_" + itos(data_id)) && !tiles_data.has(mesh_library->get_item_name(i).trim_prefix(name + "_" + itos(data_id)))) {
            // If the item name starts with the name of this TileToGridData, it is a generated tile
            print_line("TileToGridData::_generate_full_tiles: Removing item ", mesh_library->get_item_name(i), " from mesh library because it is not in use anymore.");
            mesh_library->remove_item(i);
        }
    }
    for(String item_name : name_to_row_in_source.keys()){
        if(!tiles_data.has(item_name)){
            tile_set_source->remove_tile(Vector2i(0,name_to_row_in_source[item_name]));
            name_to_row_in_source.erase(item_name);
        }
    }

    for (Ref<TileItemData> item_data : tiles_data.values()) {

        Ref<MeshTileItemData> mesh_item_data = item_data;
        Ref<SceneTileItemData> scene_item_data = item_data;
        if (mesh_item_data.is_valid()) {
            String full_item_name =  name + "_" + itos(data_id) + mesh_item_data->item_name;
            int id  = mesh_library->find_item_by_name(full_item_name);
            if (id == -1) {
                // If the item does not exist, create it
                id = mesh_library->get_last_unused_item_id();
                mesh_library->create_item(id);
                print_line("TileToGridData::_generate_full_tiles: Creating item ", full_item_name, " with id ", id);
            } 

            mesh_library->set_item_mesh(id, mesh_item_data->mesh);
            mesh_library->set_item_name(id, full_item_name);
            mesh_library->set_item_shapes(id, collision_shapes);
            print_line("TileToGridData::_generate_full_tiles: Generated mesh for item ", full_item_name, " with shapes ", collision_shapes);

            Ref<MeshPreviewTexture> preview = memnew(MeshPreviewTexture);
            preview->set_width(tile_size.x);
            preview->set_mesh(mesh_item_data->mesh);
            mesh_item_data->preview = preview;
        } else if (scene_item_data.is_valid()) {
            Ref<ScenePreviewTexture> preview = memnew(ScenePreviewTexture);
            preview->set_width(tile_size.x);
            preview->set_packed_scene(scene_item_data->scene);
            scene_item_data->preview = preview;
        }
    }

    //wait for previews to be generated 
    RenderingServer::get_singleton()->connect("frame_post_draw", callable_mp(this, &TileToGridData::_generate_tile_set_source).bind(tiles_data), CONNECT_ONE_SHOT | CONNECT_DEFERRED);

}


TypedDictionary<String, TileItemData>  TileToGridData::_generate_tiles_data() {
    TypedDictionary<String, TileItemData> tiles_data;
    return tiles_data;
}


void TileToGridData::create_tile_at_row(int row, int alternative_tile_count) {
    if (!tile_set_source.is_valid()) {
        return; // No tile set source, nothing to do
    }
    Vector2i atlas_coords(0, row);
    if (!tile_set_source->has_tile(atlas_coords)) {
        ERR_FAIL_COND_MSG(tile_set_source->get_texture() == nullptr, "TileToGridData::_generate_tile_set_source: TileSetSource texture is null, please set a texture for the TileSetSource.");
        ERR_FAIL_COND_MSG(!tile_set_source->has_room_for_tile(atlas_coords, Vector2i(1, 1), 1, Vector2i(0, 0), 1), "TileToGridData::_generate_tile_set_source: TileSetSource does not have room for tile for item. Please check the tile set source." + Variant(tile_set_source->get_texture()->get_size()).stringify() + " and tile size " + Variant(tile_size).stringify() + ". TileSetSource has " + itos(tile_set_source->get_tiles_count()) + " tiles, but no room for tile at row " + itos(row) + ".");
        tile_set_source->create_tile(atlas_coords);
    }
    while (tile_set_source->get_alternative_tiles_count(atlas_coords) < alternative_tile_count) {
        tile_set_source->create_alternative_tile(atlas_coords);
    }
    while (tile_set_source->get_alternative_tiles_count(atlas_coords) > alternative_tile_count) {
        tile_set_source->remove_alternative_tile(atlas_coords, tile_set_source->get_alternative_tiles_count(atlas_coords) - 1);
    }
}

void TileToGridData::_generate_tile_set_source(TypedDictionary<String, TileItemData> tiles_data) {
    // Generate the tile set source from the tuples
    int lowest_unused_row = 0;
    TypedArray<Image> images;
    
    for (Ref<TileItemData> item_data : tiles_data.values()) {
        String item_name = item_data->item_name;
        if(!name_to_row_in_source.has(item_name)){
            while(name_to_row_in_source.values().has(lowest_unused_row)){
                lowest_unused_row++;
            }
            name_to_row_in_source[item_name] = lowest_unused_row;
        }
        int row = name_to_row_in_source[item_name];
        if(row >= images.size()){
            images.resize(row + 1);
        }
        Ref<MeshTileItemData> mesh_item_data = item_data;
        Ref<SceneTileItemData> scene_item_data = item_data;
        if (mesh_item_data.is_valid()) {
            Ref<MeshPreviewTexture> preview = mesh_item_data->preview;
            ERR_FAIL_COND_MSG(!preview->is_ready(), "TileToGridData::_generate_tile_set_source: Preview texture for item " + item_name + " is not ready, please wait for the texture to be generated.");
            images[row] = preview->get_image();
            mesh_library->set_item_preview(mesh_library->find_item_by_name(name + "_" + itos(data_id) + item_name), preview);
        } else if (scene_item_data.is_valid()) {
            Ref<ScenePreviewTexture> preview = scene_item_data->preview;
            ERR_FAIL_COND_MSG(!preview->is_ready(), "TileToGridData::_generate_tile_set_source: Preview texture for item " + item_name + " is not ready, please wait for the texture to be generated.");
            images[row] = preview->get_image();
        }
    }
    ERR_FAIL_COND_MSG(images.size() == 0, "TileToGridData::_generate_tile_set_source: No images found in tile set source, please check the meshes and the tile set source. There was " + itos(tiles_data.size()) + " tuples, but no images were generated.");
    Ref<ImageTexture> texture = tile_set_source->get_texture();
    if(!texture.is_valid()){
        texture = (Ref<ImageTexture>) memnew(ImageTexture);
        tile_set_source->set_texture(texture);
    }
    Ref<MergedImage> image = memnew(MergedImage);
    image->set_images(images);
    texture->set_image(image);
    Vector2i texture_region_size = Vector2i(
        tile_size.x,
        image->get_max_height()
    );
    tile_set_source->set_texture_region_size(texture_region_size);
    Vector2i image_size = image->get_size();
    ERR_FAIL_COND_MSG(image_size != texture->get_size(), "TileToGridData::_generate_tile_set_source: Image size does not match texture size, image size was (" + itos(image_size.x) + "," + itos(image_size.y) + "), texture size was (" + itos(texture->get_width()) + "," + itos(texture->get_height()) + ").");
    ERR_FAIL_COND_MSG(image_size.x < texture_region_size.x || image_size.y < texture_region_size.y, "TileToGridData::_generate_tile_set_source: Image size is zero, please check the images in the tile set source, image size was (" + itos(image_size.x) + "," + itos(image_size.y) + "), . \n first image size was (" + itos(image->get_max_width()) + "," + itos(image->get_max_height()) + ").");

    for (Ref<TileItemData> item_data : tiles_data.values()) {
        String item_name = item_data->item_name;
        int row = name_to_row_in_source[item_name];
        UtilityFunctions::printt(row, " is the row for item ", item_name, " in tile set source with id ", data_id, " and name ", name);
        Vector2i atlas_coords(0, row);
        UtilityFunctions::print(atlas_coords);

        
        
        Ref<MeshTileItemData> mesh_item_data = item_data;
        Ref<SceneTileItemData> scene_item_data = item_data;
        if (mesh_item_data.is_valid()) {
            if (!tile_set_source->has_tile(atlas_coords)) {
                create_tile_at_row(row);
            }
            for(int i = 0; i<4; i++){
                TileData *tile_data = tile_set_source->get_tile_data(atlas_coords,i);
                ERR_FAIL_COND_MSG(tile_data == nullptr, "TileToGridData::_generate_tile_set_source: TileData is null for item " + item_name + " at row " + itos(row) + " and index " + itos(i) + " and image size was (" + itos(((Ref<Image>)images[atlas_coords.x])->get_width()) + "," + itos(((Ref<Image>)images[atlas_coords.x])->get_height()) + ").");
                tile_data->set_terrain_set(terrain_set);
                tile_data->set_terrain(item_data->center_bit);
                for (KeyValue<TileSet::CellNeighbor, int> neighbor_and_value : item_data->peering_bits) {
                    TileSet::CellNeighbor neighbor = TileSet::CellNeighbor((neighbor_and_value.key + 12 * i) % 16);
                    int value = neighbor_and_value.value;
                    tile_data->set_terrain_peering_bit(neighbor, value);
                }
                switch(i){
                    case 1 :
                    tile_data->set_transpose(true);
                    tile_data->set_flip_v(true);
                    break;
                    case 2:
                    tile_data->set_flip_h(true);
                    tile_data->set_flip_v(true);
                    break;
                    case 3:
                    tile_data->set_transpose(true);
                    tile_data->set_flip_h(true);
                    break;
                    default:
                    break;
                }
                tile_data->set_custom_data(MESH_NAME_LAYER, name + "_" + itos(data_id) + item_name);
                tile_data->set_custom_data(ROTATION_Y_LAYER, 90 * i);
            }
        } else if (scene_item_data.is_valid()) {
            if (!tile_set_source->has_tile(atlas_coords)) {
                create_tile_at_row(row, 1);
            }
            
            TileData *tile_data = tile_set_source->get_tile_data(atlas_coords, 0);
            
            tile_data->set_custom_data(SCENE_NAME_LAYER, scene_item_data->scene);
        }
        print_line("TileToGridData::_generate_tile_set_source: Creating tile for item " + item_name + " at row " + itos(row) + " and index " + itos(atlas_coords.x) + ". Generated alternative tiles: " + itos(tile_set_source->get_alternative_tiles_count(atlas_coords)) );
    }
}


TypedArray<Texture2D> TileToGridData::get_terrain_bit_to_icon() const {
    for (TypedArray<Texture2D> terrain_bit_to_icon : terrain_set_to_terrain_bit_to_icon) {
        print_line("terrain_bit_to_icon : ", terrain_bit_to_icon);
    }
    if (terrain_set < terrain_set_to_terrain_bit_to_icon.size() && terrain_set >= 0) {
        return terrain_set_to_terrain_bit_to_icon[terrain_set];
    }
    TypedArray<Texture2D> result;
    result.resize(1);
    return result;
}

void TileToGridData::set_terrain_set(const int &p_terrain_set) {
    terrain_set = p_terrain_set;
    notify_property_list_changed();
}

int TileToGridData::get_terrain_set() const {
    return terrain_set;
}


void TileToGridData::request_generation() {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return;
    }
    print_line("TileToGridData::request_generation -- generation_requested: ", generation_requested);
    if (!generation_requested) {
        generation_requested = true;
        callable_mp(this, &TileToGridData::generate_full_tiles).call_deferred();
    }
}


} // namespace godot
