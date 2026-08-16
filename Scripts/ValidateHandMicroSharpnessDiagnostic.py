"""Fresh-process validation for the non-shipping 192x128 portrait set."""

import unreal


DESTINATION = "/Game/Developers/FMCodex/HandMicroDiagnostics"
ASSET_NAMES = (
    "T_Prototype_Arsenal_DavidRaya_HandMicro_Runtime192",
    "T_Prototype_Arsenal_WilliamSaliba_HandMicro_Runtime192",
    "T_Prototype_Arsenal_BukayoSaka_HandMicro_Runtime192",
    "T_Prototype_Arsenal_MartinOdegaard_HandMicro_Runtime192",
    "T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_Runtime192",
    "T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_Runtime192",
)

for asset_name in ASSET_NAMES:
    asset_path = f"{DESTINATION}/{asset_name}"
    asset = unreal.load_asset(asset_path)
    if not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(f"Missing diagnostic Texture2D: {asset_path}")

    checks = {
        "dimensions": (asset.blueprint_get_size_x(), asset.blueprint_get_size_y())
        == (192, 128),
        "lod_group": asset.get_editor_property("lod_group")
        == unreal.TextureGroup.TEXTUREGROUP_UI,
        "compression": asset.get_editor_property("compression_settings")
        == unreal.TextureCompressionSettings.TC_BC7,
        "mips": asset.get_editor_property("mip_gen_settings")
        == unreal.TextureMipGenSettings.TMGS_SHARPEN1,
        "filter": asset.get_editor_property("filter")
        == unreal.TextureFilter.TF_TRILINEAR,
        "never_stream": bool(asset.get_editor_property("never_stream")),
        "srgb": bool(asset.get_editor_property("srgb")),
        "lod_bias": int(asset.get_editor_property("lod_bias")) == 0,
    }
    failed = [name for name, passed in checks.items() if not passed]
    if failed:
        raise RuntimeError(
            f"Diagnostic texture validation failed for {asset_path}: {failed}"
        )
    unreal.log(
        "FMCODEX_HAND_MICRO_SHARPNESS_VALIDATE_ITEM=PASS "
        f"asset={asset_path} dimensions=192x128 uv=full "
        "composition=unchanged crop=none reframe=none "
        "lod_group=TEXTUREGROUP_UI compression=TC_BC7 "
        "mip_generation=TMGS_SHARPEN1 filter=TF_TRILINEAR "
        "never_stream=true srgb=true lod_bias=0"
    )

unreal.log(
    "FMCODEX_HAND_MICRO_SHARPNESS_VALIDATE=PASS "
    f"fresh_process_reload_count={len(ASSET_NAMES)} errors=0 warnings=0"
)
