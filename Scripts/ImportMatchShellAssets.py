"""Import only the two independent Stage 8 match-shell textures."""
from pathlib import Path
import unreal

root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
for name in ("T_MatchShell_Stadium", "T_MatchShell_Turf"):
    task = unreal.AssetImportTask()
    task.filename = str(root / "ArtSource/UI/MatchShell" / (name + ".png"))
    task.destination_path = "/Game/UI/MatchShell"
    task.automated = True
    task.replace_existing = True
    task.save = False
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    texture = unreal.load_asset(task.destination_path + "/" + name)
    if not isinstance(texture, unreal.Texture2D):
        raise RuntimeError("Missing shell texture: " + name)
    texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)
    texture.set_editor_property("srgb", True)
    texture.set_editor_property("never_stream", True)
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    unreal.log(f"MATCH_SHELL_ASSET {name} {texture.blueprint_get_size_x()}x{texture.blueprint_get_size_y()}")
unreal.log("MATCH_SHELL_IMPORT=PASS")
