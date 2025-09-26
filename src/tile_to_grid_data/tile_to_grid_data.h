#ifndef TILE_TO_GRID_DATA_H
#define TILE_TO_GRID_DATA_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/type_info.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/tile_set_atlas_source.hpp>
#include <godot_cpp/classes/mesh_library.hpp>
#include <godot_cpp/classes/box_shape3d.hpp>
#include <godot_cpp/classes/tile_set.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/typed_dictionary.hpp>
#include <godot_cpp/templates/hash_map.hpp>


#include "mesh_utility/transformed_mesh.h"



namespace godot {
    extern const char* MESH_NAME_LAYER;
    extern const char* ROTATION_X_LAYER;
    extern const char* ROTATION_Y_LAYER;
    extern const char* ROTATION_Z_LAYER;
    extern const char* SCENE_NAME_LAYER;

    class TileItemData : public RefCounted {
        GDCLASS(TileItemData, RefCounted);
        
        public:
        String item_name;
        Ref<ImageTexture> preview;
        int center_bit = 0;
        HashMap<TileSet::CellNeighbor, int> peering_bits;
        
        protected:
        static void _bind_methods() {};
    };
    
    class SceneTileItemData : public TileItemData {
        GDCLASS(SceneTileItemData, TileItemData);
        
        public:
        Ref<PackedScene> scene;
        
        protected:
        static void _bind_methods() {};
    };
    
    
    class MeshTileItemData : public TileItemData {
        GDCLASS(MeshTileItemData, TileItemData);
        
        public:
        Ref<Mesh> mesh;
        
        protected:
        static void _bind_methods() {};
    };
    
    class TileToGridData : public Resource {
        GDCLASS(TileToGridData, Resource);
        
        public:
        TileToGridData();
        ~TileToGridData();
        
        String get_name();
        void set_name(String p_name) ;
        void set_data_id(int p_data_id);
        int get_data_id() const;
        
        void set_grid_size(Vector3 p_grid_size);
        Vector3 get_grid_size() const;
        void set_tile_size(Vector2i p_tile_size);
        Vector2i get_tile_size() const;
        void set_terrain_set_to_terrain_bit_to_icon(Vector<TypedArray<Texture2D>> p_terrain_set_to_terrain_bit_to_icon);
        Vector<TypedArray<Texture2D>> get_terrain_set_to_terrain_bit_to_icon() const ;
        void set_terrain_set_to_hint_string(TypedArray<String> p_terrain_set_to_hint_string);
        TypedArray<String> get_terrain_set_to_hint_string() const ;
        void set_terrain_set_hint_string(String p_terrain_set_hint_string);
        String get_terrain_set_hint_string() const ;
        Ref<MeshLibrary> get_mesh_library();
        void set_mesh_library(Ref<MeshLibrary> p_mesh_library);
        
        
        Ref<TileSetSource> get_tile_set_source();
        void set_tile_set_source(Ref<TileSetSource> p_tile_set_source);
        
        void set_name_to_row_in_source(TypedDictionary<String,int> p_name_to_row_in_source);
        TypedDictionary<String,int> get_name_to_row_in_source() const;
        void set_name_to_id(TypedDictionary<String,int> p_name_to_id);
        TypedDictionary<String,int> get_name_to_id() const;
        
        void request_generation();
        
        
        void set_terrain_set(const int &p_terrain_set);
        int get_terrain_set() const ;
        
        TypedArray<Texture2D> get_terrain_bit_to_icon() const;
        
        
        void generate_full_tiles();
        virtual TypedDictionary<String, TileItemData> _generate_tiles_data();
        void _generate_tile_set_source(TypedDictionary<String, TileItemData> tiles_data);
        
        
        
        protected:
        static void _bind_methods();
        void create_tile_at_row(int row, int alternative_tile_count = 4);
        int data_id =-1;
        
        
        Vector<TypedArray<Texture2D>> terrain_set_to_terrain_bit_to_icon;
        TypedArray<String> terrain_set_to_hint_string;
        String terrain_set_hint_string;
        Vector3 grid_size = Vector3(0,0,0);
        Vector2i tile_size = Vector2i(16,16);
        Ref<MeshLibrary> mesh_library;
        
        int terrain_set = 0;
        Ref<TileSetAtlasSource> tile_set_source;
        
        
        private :
        Ref<BoxShape3D> collision_shape = memnew(BoxShape3D);
        Array collision_shapes;
        
        String name;
        TypedDictionary<String,int> name_to_row_in_source;
        bool generation_requested = false;
        
        
    };
    
} // namespace godot
#endif // TILE_TO_GRID_DATA_H
