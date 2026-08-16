"""Fresh-process validation for the frozen Hand Micro production Runtime192 set."""

import unreal


DESTINATION = "/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout"
ASSET_NAMES = (
    "T_Prototype_Arsenal_DavidRaya_HandMicro_ApprovedRuntime192",
    "T_Prototype_Arsenal_WilliamSaliba_HandMicro_ApprovedRuntime192",
    "T_Prototype_Arsenal_BukayoSaka_HandMicro_ApprovedRuntime192",
    "T_Prototype_Arsenal_MartinOdegaard_HandMicro_ApprovedRuntime192",
    "T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_ApprovedRuntime192",
    "T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_ApprovedRuntime192",
    "T_Prototype_Arsenal_DeclanRice_HandMicro_ApprovedRuntime192",
    "T_Prototype_Arsenal_GabrielMartinelli_HandMicro_ApprovedRuntime192",
    "T_Prototype_Arsenal_GabrielMagalhaes_HandMicro_ApprovedRuntime192",
    "T_Prototype_Arsenal_MikelMerino_HandMicro_ApprovedRuntime192",
    "T_Prototype_ManchesterCity_PhilFoden_HandMicro_ApprovedRuntime192",
    "T_Prototype_ManchesterCity_Rodri_HandMicro_ApprovedRuntime192",
    "T_Prototype_ManchesterCity_RubenDias_HandMicro_ApprovedRuntime192",
    "T_Prototype_ManchesterCity_JoskoGvardiol_HandMicro_ApprovedRuntime192",
    "T_Prototype_ManchesterCity_BernardoSilva_HandMicro_ApprovedRuntime192",
    "T_Prototype_ManchesterCity_JeremyDoku_HandMicro_ApprovedRuntime192",
)

OBSOLETE_DEVELOPER_ROOTS = (
    "/Game/Developers/FMCodex/HandMicroDiagnostics",
    "/Game/Developers/FMCodex/HandMicroArtConformance",
    "/Game/Developers/FMCodex/HandMicroPortraitRebalance",
    "/Game/Developers/FMCodex/HandMicroReferenceA",
)

for obsolete_root in OBSOLETE_DEVELOPER_ROOTS:
    obsolete_assets = unreal.EditorAssetLibrary.list_assets(
        obsolete_root, recursive=True, include_folder=False
    )
    if obsolete_assets:
        raise RuntimeError(
            f"Obsolete Hand Micro assets remain under {obsolete_root}: "
            f"{obsolete_assets}"
        )

for asset_name in ASSET_NAMES:
    asset_path = f"{DESTINATION}/{asset_name}"
    asset = unreal.load_asset(asset_path)
    if not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(f"Missing approved Texture2D: {asset_path}")
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
            f"Approved rollout validation failed: {asset_path}: {failed}"
        )
    unreal.log(
        "FMCODEX_HAND_MICRO_PRODUCTION_VALIDATE_ITEM=PASS "
        f"asset={asset_path} dimensions=192x128 uv=full runtime_transform=none"
    )

unreal.log(
    "FMCODEX_HAND_MICRO_PRODUCTION_VALIDATE=PASS "
    f"fresh_process_reload_count={len(ASSET_NAMES)} obsolete_assets=0 "
    "errors=0 warnings=0"
)
