"""Import only the two original LongShot scene textures; preserve match-shell assets."""
from pathlib import Path
import struct
import unreal

root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
for name, size in (("T_TacticalScene_Turf", 1024), ("T_TacticalScene_Ball", 256)):
    source = root / "ArtSource/UI/TacticalScene" / (name + ".png")
    header = source.read_bytes()[:24]
    if header[:8] != b"\x89PNG\r\n\x1a\n" or struct.unpack(">II", header[16:24]) != (1254, 1254):
        raise RuntimeError("Scene UV crop requires a 1254-square PNG: " + name)
    task = unreal.AssetImportTask()
    task.filename = str(source)
    task.destination_path = "/Game/UI/TacticalScene"
    task.automated = True
    task.replace_existing = True
    task.save = False
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    texture = unreal.load_asset(task.destination_path + "/" + name)
    if not isinstance(texture, unreal.Texture2D):
        raise RuntimeError("Missing tactical scene texture: " + name)
    texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_SIMPLE_AVERAGE)
    # Generated sources are 1254 square. UE 5.3 needs power-of-two padding before
    # it can build a mip chain or honor MaxTextureSize; runtime UVs crop the pad.
    texture.set_editor_property("power_of_two_mode", unreal.TexturePowerOfTwoSetting.PAD_TO_POWER_OF_TWO)
    texture.set_editor_property("filter", unreal.TextureFilter.TF_TRILINEAR)
    texture.set_editor_property("max_texture_size", size)
    texture.set_editor_property("srgb", True)
    texture.set_editor_property("never_stream", True)
    texture.set_editor_property("compression_no_alpha", name.endswith("Turf"))
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    unreal.log(f"TACTICAL_SCENE_ASSET=PASS {name} max={size}")
