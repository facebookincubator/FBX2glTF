@tool
extends EditorSceneFormatImporter

var settings = ProjectSettings
	
const settings_fbx2gltf_path = "filesystem/import/fbx/fbx2gltf_path"

var fbx2gltf_path : String

func _get_extensions():
	return ["fbx"]


func _get_import_flags():
	return EditorSceneFormatImporter.IMPORT_SCENE


func _import_scene(path: String, flags: int, bake_fps: int):
	if not ProjectSettings.has_setting(settings_fbx2gltf_path):
		ProjectSettings.set_initial_value(settings_fbx2gltf_path, "FBX2glTF")
		ProjectSettings.set_setting(settings_fbx2gltf_path, "FBX2glTF")
	else:
		fbx2gltf_path = ProjectSettings.get_setting(settings_fbx2gltf_path)
	if fbx2gltf_path.is_empty():
		return null
	var property_info = {
		"name": settings_fbx2gltf_path,
		"type": TYPE_STRING,
		"hint": PROPERTY_HINT_GLOBAL_FILE,
		"hint_string": ""
	}
	ProjectSettings.add_property_info(property_info)
	var user_path_base = OS.get_user_data_dir()
	var path_global : String = ProjectSettings.globalize_path(path)
	var output_path : String = "res://.godot/imported/" + path.get_file().get_basename() + "-" + path.md5_text() + ".gltf" 
	var output_path_global = ProjectSettings.globalize_path(output_path)
	var stdout = [].duplicate()
	var temp_dir_global =  ProjectSettings.globalize_path("res://.godot/imported/")
	var fbx2gltf_path_global = ProjectSettings.globalize_path(fbx2gltf_path)
	var ret = OS.execute(fbx2gltf_path_global, [
		"--fbx-temp-dir", temp_dir_global,
		"-i", path_global,
		"-o", output_path_global], stdout, true)

	for line in stdout:
		print(line)
	if ret != 0:
		print("FBX2glTF returned " + str(ret))
		return null

	var gstate : GLTFState = GLTFState.new()
	var gltf : GLTFDocument = GLTFDocument.new()
	var root_node : Node = gltf.import_scene(output_path, 0, 1000.0, gstate)
	root_node.name = path.get_basename().get_file()

	return root_node
