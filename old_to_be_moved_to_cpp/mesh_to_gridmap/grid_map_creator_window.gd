@tool
extends HSplitContainer

@onready var mesh_list: VBoxContainer = $MeshList
@onready var mesh_list_tool_bar: HBoxContainer = $MeshList/MeshListToolBar

var editor_resource_picker := EditorResourcePicker.new()

func _ready() -> void:
	add_child(editor_resource_picker, true)
	
	editor_resource_picker.base_type = "Mesh"
	editor_resource_picker.resource_changed.connect(_on_array_selected)
	editor_resource_picker.hide()
	
	var delete_button := Button.new()
	delete_button.icon = get_theme_icon("Remove", "EditorIcons")
	delete_button.pressed.connect(_on_delete_button_pressed)
	mesh_list_tool_bar.add_child(delete_button)
	
	var add_button := Button.new()
	add_button.icon = get_theme_icon("Add", "EditorIcons")
	add_button.pressed.connect(_on_add_button_pressed)
	mesh_list_tool_bar.add_child(add_button)

@export_tool_button("add editor resource picker")
var add_editor_resource_picker : Callable = popup_quick_open_panel


func popup_quick_open_panel():

	editor_resource_picker.get_child(0).pressed.emit()
	var pop_up : PopupMenu = editor_resource_picker.get_child(2)
	var event_confirm := InputEventAction.new()
	event_confirm.action = "ui_accept"
	event_confirm.pressed = true
	var shortcut := Shortcut.new()
	shortcut.events = [event_confirm]
	pop_up.set_item_shortcut(pop_up.get_item_index(1), shortcut)
	pop_up.activate_item_by_event(event_confirm)


func _on_array_selected(resource : Resource, inspect := true) -> void:
	print(resource)


func _on_delete_button_pressed():
	pass

func _on_add_button_pressed():
	pass
