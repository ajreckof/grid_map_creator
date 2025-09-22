#ifndef MERGING_TILE_TO_GRID_DATA_H
#define MERGING_TILE_TO_GRID_DATA_H

#include "tile_to_grid_data.h"

#include "preview_texture/mesh_preview_texture.h"
namespace godot {

class MergingTileToGridData : public TileToGridData {
    GDCLASS(MergingTileToGridData, TileToGridData);

public:
    MergingTileToGridData();
    ~MergingTileToGridData();

    virtual void _setup_local_to_scene() override;

    void set_list_count(int count);
    int get_list_count() const;

    void set_center_bit(int p_bit);
    int get_center_bit() const ;

    void init_tile_2D_preview();
    void update_transformed_mesh(int index);

    void notify_changed_id(int id);
    bool is_id_changed(int id) const;

    void _validate_property(PropertyInfo &p_property) const;

    String tuple_to_name(PackedInt32Array tuple);

    virtual TypedDictionary<String, TileItemData> _generate_tiles_data() override;
    bool check_tuple_validity(PackedInt32Array tuple);

protected:
    static void _bind_methods();
    void _get_property_list(List<PropertyInfo> *p_list) const;

    bool _get(const StringName &p_name, Variant &r_ret) const;
    bool _set(const StringName &p_name, const Variant &p_value);

    Ref<ArrayMesh> get_mesh(int index) const;
    int get_bottom_bits(int index) const;
    int get_right_bits(int index) const;
    int get_bottom_right_bits(int index) const;
    int get_ids(int index) const;


private:
    // Additional properties specific to merging functionality can be added here
    
    int center_bit = 0;

    bool initialised = false;
    static Transform3D symmetry_tranform;

    PackedInt32Array changed_ids;
    // list of parameter for meshes : mesh, transform, type
    // list of variable to show as read_only : name, tile_2D_preview
    const StringName base_name = "mesh_{0}/";

    // Per-tile data structures indexed by tile ID
    TypedDictionary<int, ArrayMesh> _meshes;
    const StringName mesh_property_name = "mesh";
    
    TypedDictionary<int, TransformedMesh> transformed_symmetric_meshes;
    TypedDictionary<int, TransformedMesh> transformed_meshes;

    TypedDictionary<int, Transform3D> _transforms;
    const StringName rotation_property_name = "rotation";
    
    TypedDictionary<int, bool> _generate_symmetric;
    const StringName generate_symmetric_property_name = "generate_symmetric";

    TypedDictionary<int, MeshPreviewTexture> _tile_2D_previews;
    const StringName tile_2D_preview_property_name = "tile_2D_preview";

    TypedDictionary<int, MeshPreviewTexture> _tile_2D_previews_symmetric;
    const StringName tile_2D_preview_symmetric_property_name = "tile_2D_preview_symmetric";

    TypedDictionary<int, int> bottom_bits;
    const StringName bottom_bits_property_name = "bottom_bit";

    TypedDictionary<int, int> right_bits;
    const StringName right_bits_property_name = "right_bit";

    TypedDictionary<int, int> bottom_right_bits;
    const StringName bottom_right_bits_property_name = "bottom_right_bit";

    // Array of all tile IDs (ordered list for iteration)
    TypedArray<int> ids;
    const StringName ids_property_name = "id";

    int _list_count = 0;
};

} // namespace godot
#endif // MERGING_TILE_TO_GRID_DATA_H
