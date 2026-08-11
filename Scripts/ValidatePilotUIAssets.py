"""Fresh-process validation for the imported Stage 6.11 Texture2D packages."""

import unreal


EXPECTED = (
    "/Game/UI/Cards/T_Pilot_CardFrame_01",
    "/Game/UI/Portraits/T_Pilot_PlayerPortrait_01",
)


for asset_path in EXPECTED:
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        raise RuntimeError(f"Asset package does not resolve: {asset_path}")

    asset = unreal.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"Fresh process could not load asset: {asset_path}")
    if not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(
            f"Expected Texture2D, got {asset.get_class().get_name()}: {asset_path}"
        )
    width = asset.blueprint_get_size_x()
    height = asset.blueprint_get_size_y()
    if width <= 1 or height <= 1:
        raise RuntimeError(f"Invalid texture dimensions {width}x{height}: {asset_path}")

    unreal.log(
        "FMCODEX_PILOT_VALIDATE "
        f"asset={asset_path} object={asset.get_path_name()} "
        f"class={asset.get_class().get_name()} dimensions={width}x{height} "
        f"srgb={asset.get_editor_property('srgb')} loaded=true redirector=false"
    )

unreal.log("FMCODEX_PILOT_VALIDATION=PASS")
