"""Import the original Stage 8.8F.2 athlete atlas; no other assets are touched."""
from pathlib import Path
import unreal

root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
name = "T_Theater_Athletes"
task = unreal.AssetImportTask()
task.filename = str(root / "ArtSource/UI/ResolutionTheater" / (name + ".png"))
task.destination_path = "/Game/UI/ResolutionTheater"
task.automated = True
task.replace_existing = True
task.save = False
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
texture = unreal.load_asset(task.destination_path + "/" + name)
if not isinstance(texture, unreal.Texture2D):
    raise RuntimeError("Theater athlete atlas failed to import")
texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
texture.set_editor_property("srgb", True)
texture.set_editor_property("never_stream", True)
texture.set_editor_property("compression_no_alpha", False)
unreal.EditorAssetLibrary.save_loaded_asset(texture)
unreal.log(f"THEATER_ATLAS_IMPORT=PASS {texture.blueprint_get_size_x()}x{texture.blueprint_get_size_y()}")
