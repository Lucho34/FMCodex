"""Import/configure the bounded art-conformed Runtime192 candidate set."""

from pathlib import Path

import unreal


DESTINATION = "/Game/Developers/FMCodex/HandMicroArtConformance"
EXPECTED_SIZE = (192, 128)
ASSET_NAMES = (
    "T_Prototype_Arsenal_DavidRaya_HandMicro_ArtConformedRuntime192",
    "T_Prototype_Arsenal_WilliamSaliba_HandMicro_ArtConformedRuntime192",
    "T_Prototype_Arsenal_BukayoSaka_HandMicro_ArtConformedRuntime192",
    "T_Prototype_Arsenal_MartinOdegaard_HandMicro_ArtConformedRuntime192",
    "T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_ArtConformedRuntime192",
    "T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_ArtConformedRuntime192",
)

project_root = Path(
    unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
)
source_root = (
    project_root
    / "ArtSource"
    / "UI"
    / "Diagnostics"
    / "HandMicroArtConformance"
    / "Runtime192"
)

tasks = []
for asset_name in ASSET_NAMES:
    source_path = source_root / f"{asset_name}.png"
    if not source_path.is_file():
        raise RuntimeError(f"Art-conformance source image is missing: {source_path}")
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
    size = (asset.blueprint_get_size_x(), asset.blueprint_get_size_y())
    if size != EXPECTED_SIZE:
        raise RuntimeError(f"Unexpected dimensions {size}: {asset_path}")
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
        "FMCODEX_HAND_MICRO_ART_CONFORMANCE_IMPORT_ITEM=PASS "
        f"asset={asset_path} dimensions=192x128 "
        "lod_group=TEXTUREGROUP_UI compression=TC_BC7 "
        "mip_generation=TMGS_SHARPEN1 filter=TF_TRILINEAR "
        "never_stream=true srgb=true lod_bias=0"
    )

unreal.log(
    "FMCODEX_HAND_MICRO_ART_CONFORMANCE_IMPORT=PASS "
    f"source_count={len(ASSET_NAMES)} import_count={len(ASSET_NAMES)} errors=0"
)
