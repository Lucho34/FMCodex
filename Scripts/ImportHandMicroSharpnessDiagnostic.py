"""Import/configure the bounded non-shipping 192x128 portrait validation set."""

from pathlib import Path

import unreal


DESTINATION = "/Game/Developers/FMCodex/HandMicroDiagnostics"
EXPECTED_SIZE = (192, 128)
ASSET_NAMES = (
    "T_Prototype_Arsenal_DavidRaya_HandMicro_Runtime192",
    "T_Prototype_Arsenal_WilliamSaliba_HandMicro_Runtime192",
    "T_Prototype_Arsenal_BukayoSaka_HandMicro_Runtime192",
    "T_Prototype_Arsenal_MartinOdegaard_HandMicro_Runtime192",
    "T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_Runtime192",
    "T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_Runtime192",
)

project_root = Path(
    unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
)
source_root = (
    project_root / "ArtSource" / "UI" / "Diagnostics" / "HandMicroSharpness"
)

tasks = []
for asset_name in ASSET_NAMES:
    source_path = source_root / f"{asset_name}.png"
    if not source_path.is_file():
        raise RuntimeError(f"Diagnostic source image is missing: {source_path}")
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source_path))
    task.set_editor_property("destination_path", DESTINATION)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", False)
    task.set_editor_property("save", True)
    tasks.append(task)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

for asset_name in ASSET_NAMES:
    asset_path = f"{DESTINATION}/{asset_name}"
    asset = unreal.load_asset(asset_path)
    if not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(f"Imported object is not Texture2D: {asset_path}")
    width = asset.blueprint_get_size_x()
    height = asset.blueprint_get_size_y()
    if (width, height) != EXPECTED_SIZE:
        raise RuntimeError(
            f"Unexpected dimensions {width}x{height}, expected 192x128: {asset_path}"
        )

    asset.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    asset.set_editor_property(
        "compression_settings", unreal.TextureCompressionSettings.TC_BC7
    )
    asset.set_editor_property(
        "mip_gen_settings", unreal.TextureMipGenSettings.TMGS_SHARPEN1
    )
    asset.set_editor_property("filter", unreal.TextureFilter.TF_TRILINEAR)
    asset.set_editor_property("never_stream", True)
    asset.set_editor_property("srgb", True)
    asset.set_editor_property("lod_bias", 0)
    if not unreal.EditorAssetLibrary.save_loaded_asset(
        asset, only_if_is_dirty=False
    ):
        raise RuntimeError(f"Failed to save package: {asset_path}")
    unreal.log(
        "FMCODEX_HAND_MICRO_SHARPNESS_IMPORT_ITEM=PASS "
        f"asset={asset_path} dimensions={width}x{height} "
        "lod_group=TEXTUREGROUP_UI compression=TC_BC7 "
        "mip_generation=TMGS_SHARPEN1 filter=TF_TRILINEAR "
        "never_stream=true srgb=true lod_bias=0"
    )

unreal.log(
    "FMCODEX_HAND_MICRO_SHARPNESS_IMPORT=PASS "
    f"source_count={len(ASSET_NAMES)} import_count={len(ASSET_NAMES)} errors=0"
)
