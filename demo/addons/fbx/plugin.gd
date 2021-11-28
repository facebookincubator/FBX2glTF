@tool
extends EditorPlugin

var import_plugin

func _enter_tree():
	import_plugin = preload("res://addons/fbx/import_fbx.gd").new()
	add_scene_format_importer_plugin(import_plugin)


func _exit_tree():
	add_scene_format_importer_plugin(import_plugin)
	import_plugin = null
