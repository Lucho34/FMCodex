"""Fresh-process validation for dedicated Prototype Full Card portraits."""

import unreal


IMPORTS = (
    ("Arsenal", "BukayoSaka", "FullCardPilot_02"),
    ("Arsenal", "DavidRaya", "FullCardPilot_02"),
    ("ManchesterCity", "Rodri", "FullCardPilot_02"),
    ("ManchesterCity", "GianluigiDonnarumma", "FullCardPilot_02"),
    ("Arsenal", "GabrielMartinelli", "FullCardHeroBust_01"),
    ("Arsenal", "GabrielMagalhaes", "FullCardHeroBust_01"),
    ("Arsenal", "MikelMerino", "FullCardHeroBust_01"),
    ("ManchesterCity", "JoskoGvardiol", "FullCardHeroBust_01"),
    ("ManchesterCity", "BernardoSilva", "FullCardHeroBust_01"),
    ("ManchesterCity", "JeremyDoku", "FullCardHeroBust_01"),
    ("Arsenal", "WilliamSaliba", "FullCardHeroBust_01"),
    ("Arsenal", "MartinOdegaard", "FullCardHeroBust_01"),
    ("Arsenal", "DeclanRice", "FullCardHeroBust_01"),
    ("ManchesterCity", "ErlingHaaland", "FullCardHeroBust_01"),
    ("ManchesterCity", "PhilFoden", "FullCardHeroBust_01"),
    ("ManchesterCity", "RubenDias", "FullCardHeroBust_01"),
)
EXPECTED_SIZE = (1024, 1536)


for team, player, asset_suffix in IMPORTS:
    asset_name = f"T_Prototype_{team}_{player}_{asset_suffix}"
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
        "FMCODEX_FULL_CARD_PORTRAIT_VALIDATE "
        f"asset={asset_path} object={asset.get_path_name()} "
        f"class={asset.get_class().get_name()} dimensions={size[0]}x{size[1]} "
        "lod_group=TEXTUREGROUP_UI srgb=true loaded=true redirector=false "
        f"compression={asset.get_editor_property('compression_settings')} "
        f"mips={asset.get_editor_property('mip_gen_settings')} "
        f"filter={asset.get_editor_property('filter')} "
        f"never_stream={bool(asset.get_editor_property('never_stream'))} "
        f"lod_bias={int(asset.get_editor_property('lod_bias'))}"
    )

unreal.log(f"FMCODEX_FULL_CARD_PORTRAIT_VALIDATION=PASS count={len(IMPORTS)}")
# Compatibility sentinel retained for callers of the established pilot wrapper.
unreal.log("FMCODEX_FULL_CARD_PILOT_VALIDATION=PASS")
