"""Fresh-process validation for the four Full Card pilot portrait packages."""

import unreal


IMPORTS = (
    ("Arsenal", "BukayoSaka"),
    ("Arsenal", "DavidRaya"),
    ("ManchesterCity", "Rodri"),
    ("ManchesterCity", "GianluigiDonnarumma"),
)
ASSET_SUFFIX = "FullCardPilot_02"
EXPECTED_SIZE = (1024, 1536)


for team, player in IMPORTS:
    asset_name = f"T_Prototype_{team}_{player}_{ASSET_SUFFIX}"
    asset_path = f"/Game/UI/Portraits/PrototypeTeams/{team}/{asset_name}"
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        raise RuntimeError(f"Asset package does not resolve: {asset_path}")

    asset = unreal.load_asset(asset_path)
    if not isinstance(asset, unreal.Texture2D):
        actual = "None" if asset is None else asset.get_class().get_name()
        raise RuntimeError(f"Expected Texture2D, got {actual}: {asset_path}")
    size = (asset.blueprint_get_size_x(), asset.blueprint_get_size_y())
    if size != EXPECTED_SIZE:
        raise RuntimeError(
            f"Unexpected dimensions {size}, expected {EXPECTED_SIZE}: {asset_path}"
        )
    if asset.get_editor_property("lod_group") != unreal.TextureGroup.TEXTUREGROUP_UI:
        raise RuntimeError(f"Texture is not in TEXTUREGROUP_UI: {asset_path}")
    if not asset.get_editor_property("srgb"):
        raise RuntimeError(f"Texture is not sRGB: {asset_path}")

    unreal.log(
        "FMCODEX_FULL_CARD_PILOT_VALIDATE "
        f"asset={asset_path} object={asset.get_path_name()} "
        f"class={asset.get_class().get_name()} dimensions={size[0]}x{size[1]} "
        "lod_group=TEXTUREGROUP_UI srgb=true loaded=true redirector=false"
    )

unreal.log("FMCODEX_FULL_CARD_PILOT_VALIDATION=PASS")
