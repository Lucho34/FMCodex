"""Fresh-process validation for Stage 6.12 Golden Sample Texture2D packages."""

import unreal


EXPECTED = (
    ("/Game/UI/Cards/GoldenSample/T_Golden_CardFrame_01", (1024, 1536), False),
    (
        "/Game/UI/Portraits/GoldenSample/T_Golden_PlayerPortrait_01",
        (1024, 1536),
        False,
    ),
    ("/Game/UI/Icons/GoldenSample/T_Golden_Role_Forward_01", (1254, 1254), True),
    (
        "/Game/UI/Icons/GoldenSample/T_Golden_Skill_LongShot_01",
        (1254, 1254),
        True,
    ),
)


for asset_path, expected_size, expects_alpha in EXPECTED:
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        raise RuntimeError(f"Asset package does not resolve: {asset_path}")

    asset = unreal.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"Fresh process could not load asset: {asset_path}")
    if not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(
            f"Expected Texture2D, got {asset.get_class().get_name()}: {asset_path}"
        )
    size = (asset.blueprint_get_size_x(), asset.blueprint_get_size_y())
    if size != expected_size:
        raise RuntimeError(
            f"Unexpected dimensions {size}, expected {expected_size}: {asset_path}"
        )
    compression_no_alpha = asset.get_editor_property("compression_no_alpha")
    if expects_alpha and compression_no_alpha:
        raise RuntimeError(
            "Alpha-bearing source was configured to discard alpha during compression: "
            f"{asset_path}"
        )

    unreal.log(
        "FMCODEX_GOLDEN_VALIDATE "
        f"asset={asset_path} object={asset.get_path_name()} "
        f"class={asset.get_class().get_name()} dimensions={size[0]}x{size[1]} "
        f"source_alpha={str(expects_alpha).lower()} "
        f"compression_no_alpha={str(compression_no_alpha).lower()} "
        f"srgb={asset.get_editor_property('srgb')} "
        "loaded=true redirector=false"
    )

unreal.log("FMCODEX_GOLDEN_VALIDATION=PASS")
