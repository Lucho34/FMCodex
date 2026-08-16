"""Fresh-process validation for Stage 6.13.1.3.10 portrait packages."""

import unreal


ASSETS = (
    ("Arsenal", "T_Prototype_Arsenal_DavidRaya_HandMicro_06"),
    ("Arsenal", "T_Prototype_Arsenal_BukayoSaka_HandMicro_06"),
    ("Arsenal", "T_Prototype_Arsenal_DeclanRice_HandMicro_06"),
    ("Arsenal", "T_Prototype_Arsenal_MartinOdegaard_HandMicro_06"),
    ("Arsenal", "T_Prototype_Arsenal_WilliamSaliba_HandMicro_06"),
    ("Arsenal", "T_Prototype_Arsenal_GabrielMartinelli_HandMicro_Validation_05"),
    ("Arsenal", "T_Prototype_Arsenal_GabrielMagalhaes_HandMicro_Validation_05"),
    ("Arsenal", "T_Prototype_Arsenal_MikelMerino_HandMicro_Validation_05"),
    ("ManchesterCity", "T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_06"),
    ("ManchesterCity", "T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_06"),
    ("ManchesterCity", "T_Prototype_ManchesterCity_PhilFoden_HandMicro_06"),
    ("ManchesterCity", "T_Prototype_ManchesterCity_Rodri_HandMicro_06"),
    ("ManchesterCity", "T_Prototype_ManchesterCity_RubenDias_HandMicro_06"),
    ("ManchesterCity", "T_Prototype_ManchesterCity_JoskoGvardiol_HandMicro_Validation_05"),
    ("ManchesterCity", "T_Prototype_ManchesterCity_BernardoSilva_HandMicro_Validation_05"),
    ("ManchesterCity", "T_Prototype_ManchesterCity_JeremyDoku_HandMicro_Validation_05"),
)
EXPECTED_SIZE = (1536, 1024)


for team, asset_name in ASSETS:
    asset_path = f"/Game/UI/Portraits/PrototypeTeams/{team}/HandMicro/{asset_name}"
    asset = unreal.load_asset(asset_path)
    if not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(f"Fresh-process load failed: {asset_path}")
    size = (asset.blueprint_get_size_x(), asset.blueprint_get_size_y())
    if size != EXPECTED_SIZE:
        raise RuntimeError(f"Unexpected dimensions {size}: {asset_path}")
    if asset.get_editor_property("lod_group") != unreal.TextureGroup.TEXTUREGROUP_UI:
        raise RuntimeError(f"Texture group is not UI: {asset_path}")
    if not asset.get_editor_property("srgb"):
        raise RuntimeError(f"sRGB is disabled: {asset_path}")
    compression = asset.get_editor_property("compression_settings")
    mip_generation = asset.get_editor_property("mip_gen_settings")
    texture_filter = asset.get_editor_property("filter")
    never_stream = asset.get_editor_property("never_stream")
    lod_bias = asset.get_editor_property("lod_bias")
    if compression != unreal.TextureCompressionSettings.TC_BC7:
        raise RuntimeError(f"Compression is not BC7: {asset_path}")
    if mip_generation != unreal.TextureMipGenSettings.TMGS_SHARPEN1:
        raise RuntimeError(f"Mip generation is not Sharpen1: {asset_path}")
    if texture_filter != unreal.TextureFilter.TF_TRILINEAR:
        raise RuntimeError(f"Filter is not trilinear: {asset_path}")
    if not never_stream:
        raise RuntimeError(f"NeverStream is disabled: {asset_path}")
    if lod_bias != 0:
        raise RuntimeError(f"Unexpected LOD bias {lod_bias}: {asset_path}")
    unreal.log(
        "FMCODEX_HAND_MICRO_DRAFT_VALIDATION "
        f"asset={asset_path} dimensions={size[0]}x{size[1]} "
        "lod_group=TEXTUREGROUP_UI srgb=true "
        f"compression={compression} mip_generation={mip_generation} "
        f"filter={texture_filter} never_stream={never_stream} "
        f"lod_bias={lod_bias}"
    )

unreal.log(f"FMCODEX_HAND_MICRO_DRAFT_VALIDATION_COUNT={len(ASSETS)}")
unreal.log("FMCODEX_HAND_MICRO_DRAFT_VALIDATION=PASS")
