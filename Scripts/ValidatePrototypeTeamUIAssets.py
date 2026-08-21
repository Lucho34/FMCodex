"""Fresh-process validation for production Shared Portrait packages."""

from pathlib import Path
import sys

import unreal

sys.dont_write_bytecode = True

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from SharedPortraitImportCatalog import (  # noqa: E402
    RUNTIME_SIZE,
    asset_path,
    load_catalog,
    select_entries,
)


project_root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
entries = select_entries(load_catalog(project_root))

for entry in entries:
    expected_asset_path = asset_path(entry)
    if not unreal.EditorAssetLibrary.does_asset_exist(expected_asset_path):
        raise RuntimeError(f"Asset package does not resolve: {expected_asset_path}")

    asset = unreal.load_asset(expected_asset_path)
    if not isinstance(asset, unreal.Texture2D):
        actual = "None" if asset is None else asset.get_class().get_name()
        raise RuntimeError(f"Expected Texture2D, got {actual}: {expected_asset_path}")
    expected_object_path = (
        f"{expected_asset_path}.{entry['assetName']}"
    )
    if asset.get_path_name() != expected_object_path:
        raise RuntimeError(
            f"Shared Portrait resolved through an alias or redirector: expected "
            f"{expected_object_path}, got {asset.get_path_name()}"
        )
    size = (asset.blueprint_get_size_x(), asset.blueprint_get_size_y())
    if size != RUNTIME_SIZE:
        raise RuntimeError(
            f"Unexpected dimensions {size}, expected {RUNTIME_SIZE}: {expected_asset_path}"
        )
    if asset.get_editor_property("lod_group") != unreal.TextureGroup.TEXTUREGROUP_UI:
        raise RuntimeError(f"Texture is not in TEXTUREGROUP_UI: {expected_asset_path}")
    if asset.get_editor_property("compression_settings") != unreal.TextureCompressionSettings.TC_BC7:
        raise RuntimeError(f"Texture is not TC_BC7: {expected_asset_path}")
    if asset.get_editor_property("mip_gen_settings") != unreal.TextureMipGenSettings.TMGS_SHARPEN1:
        raise RuntimeError(f"Texture is not TMGS_SHARPEN1: {expected_asset_path}")
    if asset.get_editor_property("filter") != unreal.TextureFilter.TF_TRILINEAR:
        raise RuntimeError(f"Texture is not TF_TRILINEAR: {expected_asset_path}")
    if not asset.get_editor_property("never_stream"):
        raise RuntimeError(
            f"Texture does not use the UE5.3 NPOT/UI NeverStream equivalent: "
            f"{expected_asset_path}"
        )
    if not asset.get_editor_property("srgb"):
        raise RuntimeError(f"Texture is not sRGB: {expected_asset_path}")
    if asset.get_editor_property("lod_bias") != 0:
        raise RuntimeError(f"Texture LODBias is not zero: {expected_asset_path}")

    unreal.log(
        "FMCODEX_PROTOTYPE_TEAM_VALIDATE "
        f"player_key={entry['playerKey']} asset={expected_asset_path} "
        f"object={asset.get_path_name()} "
        f"class={asset.get_class().get_name()} dimensions={size[0]}x{size[1]} "
        "lod_group=TEXTUREGROUP_UI compression=TC_BC7 srgb=true "
        "mip_generation=TMGS_SHARPEN1 filter=TF_TRILINEAR lod_bias=0 "
        "never_stream=true streaming_equivalent=UE53_NPOT_UI "
        "loaded=true redirector=false"
    )

unreal.log(f"FMCODEX_PROTOTYPE_TEAM_VALIDATION=PASS selected={len(entries)}")
