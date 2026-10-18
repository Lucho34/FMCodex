"""Import the authored Cross environment plate only; no match-shell reimports."""
from pathlib import Path
import unreal

root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
name = "T_CrossBroadcast_Stadium"
task = unreal.AssetImportTask()
task.filename = str(root / "ArtSource/UI/TacticalScene" / (name + ".png"))
task.destination_path = "/Game/UI/TacticalScene"
task.automated = True
task.replace_existing = True
task.save = False
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
texture = unreal.load_asset(task.destination_path + "/" + name)
if not isinstance(texture, unreal.Texture2D):
    raise RuntimeError("Cross broadcast environment import failed")
texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
texture.set_editor_property("filter", unreal.TextureFilter.TF_BILINEAR)
texture.set_editor_property("srgb", True)
texture.set_editor_property("never_stream", True)
texture.set_editor_property("compression_no_alpha", True)
unreal.EditorAssetLibrary.save_loaded_asset(texture)
unreal.log("CROSS_BROADCAST_ASSET=PASS " + name)
