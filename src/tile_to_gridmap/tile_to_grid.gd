@tool
extends GridMap
class_name TileToGrid

var tile_map_layers: Array[TileMapLayer] = []
@export var grid_height: int
@export var hide_on_run: bool = true
@export var tile_to_grid_set : TileToGridSet :
	set(value):
		tile_to_grid_set = value
		tile_to_grid_set.mesh_library = mesh_library
		tile_to_grid_set.grid_size = cell_size

@export_tool_button("Build Gridmap") var BuildGridmap = copy_tiles

@export_tool_button("Clear Gridmap") var ClearGridmap = clear_tiles

func _init() -> void:
	child_entered_tree.connect(_on_child_entered_tree)
	child_exiting_tree.connect(_on_child_exiting_tree)


func _ready() -> void:
	add_to_group("tiletogridgroup", true)
	
	if not Engine.is_editor_hint():
		if hide_on_run:
			# Cacher tous les TileMapLayers enfants
			for tile_layer in tile_map_layers:
				tile_layer.visible = false


func _on_child_entered_tree(node: Node) -> void:
	if node is TileMapLayer:
		tile_map_layers.append(node)
		_setup_tile_layer(node)

func _on_child_exiting_tree(node: Node) -> void:
	if node is TileMapLayer:
		if tile_map_layers.has(node):
			tile_map_layers.erase(node)


func _setup_tile_layer(tile_layer: TileMapLayer) -> void:

	if Engine.is_editor_hint():
		tile_layer.tile_set = tile_to_grid_set.tile_set
	else : 
		tile_layer.visible = not hide_on_run



##Grid Rotation Notes (0: Down, 16: Right, 10: Up, 22: Left)
func copy_tiles():
	print("tile to gridmap : tiles copied")
	clear_tiles()
	for tile_layer in tile_map_layers:
		_copy_tiles_from_layer(tile_layer)

func _copy_tiles_from_layer(tile_layer: TileMapLayer):
	for tile_pos in tile_layer.get_used_cells():
		var tile_data = tile_layer.get_cell_tile_data(tile_pos)
		if not tile_data:
			continue
			
		var tile_scene : PackedScene = tile_data.get_custom_data("SceneName")
		if tile_scene :
			
			continue
		var mesh_name = tile_data.get_custom_data("MeshName")
		var mesh_int = mesh_library.find_item_by_name(mesh_name)
		var grid_basis : Basis
		grid_basis = grid_basis.rotated(Vector3.UP, tile_data.get_custom_data("Rotation_Y")/180. * PI)    # Rotate around the up axis (yaw).
		grid_basis = grid_basis.rotated(Vector3.RIGHT, tile_data.get_custom_data("Rotation_Z")/180. * PI) # Rotate around the right axis (pitch).
		grid_basis = grid_basis.rotated(Vector3.BACK, tile_data.get_custom_data("Rotation_X")/180. * PI)  # Rotate around the back axis (roll).
		var basis_index = get_orthogonal_index_from_basis(grid_basis)
		var grid_pos = tile_map_to_grid_map_position(tile_pos, grid_height)
		set_cell_item(grid_pos, mesh_int, basis_index)


func create_tile_from_scene(tile_pos : Vector2i, tile_scene : PackedScene, tile_layer : TileMapLayer = null) -> void:
	var scene : Node = tile_scene.instantiate()
	scene.global_position = grid_to_global(tile_pos, grid_height)
	if "cell_size" in scene:
		scene.cell_size = cell_size
	if "tile_position" in scene:
		scene.tile_position = tile_pos
	add_child(scene,true)
	scene.owner = owner if owner else self

static func tile_map_to_grid_map_position(tile_map_pos : Vector2i, heigth : int) -> Vector3i:
	return Vector3i(tile_map_pos.x, heigth, tile_map_pos.y)

static func grid_map_to_tile_map_position(grid_map : Vector3i) -> Vector2i :
	return Vector2i(grid_map.x, grid_map.z)

func global_to_grid(global_position : Vector3) -> Vector2i:
	var local_pos := to_local(global_position)
	var grid_position := local_to_map(local_pos)
	return grid_map_to_tile_map_position(grid_position)

func global_to_heigth(global_position : Vector3) -> float:
	var local_pos := to_local(global_position)
	var grid_position := local_to_map(local_pos)
	var local_pos_aligned := map_to_local(grid_position)
	return grid_position.y + (local_pos - local_pos_aligned).y / cell_size.y


func grid_to_global(tile_position : Vector2i, heigth_layer : float) -> Vector3 :
	var grid_position : Vector3i = tile_map_to_grid_map_position(tile_position, heigth_layer) 
	var local_pos := map_to_local(grid_position)
	local_pos.y += cell_size.y * (heigth_layer - int(heigth_layer))
	return to_global(local_pos)

func clear_tiles():
	print("tile to gridmap : tiles cleared")
	clear()
	for child in get_children():
		if not child is TileMapLayer:  # Ne pas supprimer les TileMapLayers
			child.queue_free()

# Méthodes utilitaires pour accéder aux TileMapLayers
func get_tile_map_layers() -> Array[TileMapLayer]:
	return tile_map_layers

func get_first_tile_map_layer() -> TileMapLayer:
	if tile_map_layers.size() > 0:
		return tile_map_layers[0]
	return null



func get_cell_tile_data(coords: Vector2i) -> TileData:
	var first_layer = get_first_tile_map_layer()
	if first_layer:
		return first_layer.get_cell_tile_data(coords)
	return null

func get_used_rect() -> Rect2i:
	var first_layer = get_first_tile_map_layer()
	if first_layer:
		return first_layer.get_used_rect()
	return Rect2i()


func _validate_property(property: Dictionary) -> void:
	if property.name == "mesh_library" :
		property.usage &= ~ PROPERTY_USAGE_EDITOR

func _notification(what: int) -> void:
	match what:
		NOTIFICATION_EDITOR_PRE_SAVE :
			tile_to_grid_set.update_ids()
