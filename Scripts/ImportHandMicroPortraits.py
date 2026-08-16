"""Import the frozen Hand Micro production Runtime192 set."""

from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.project_dir())
SOURCE_ROOT = (
    ROOT / "ArtSource" / "UI" / "PrototypeTeams"
    / "HandMicroApprovedRollout" / "Runtime192"
)
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

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
for asset_name in ASSET_NAMES:
    source = SOURCE_ROOT / f"{asset_name}.png"
    if not source.is_file():
        raise RuntimeError(f"Missing approved Runtime192 source: {source}")
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source))
    task.set_editor_property("destination_path", DESTINATION)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", False)
    asset_tools.import_asset_tasks([task])
    if not task.get_editor_property("imported_object_paths"):
        raise RuntimeError(f"UE import failed: {source}")
    asset_path = f"{DESTINATION}/{asset_name}"
    asset = unreal.load_asset(asset_path)
    if not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(f"Imported object is not Texture2D: {asset_path}")
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
    asset.modify()
    if not unreal.EditorAssetLibrary.save_loaded_asset(
        asset, only_if_is_dirty=False
    ):
        raise RuntimeError(f"Failed to save approved Texture2D: {asset_path}")
    unreal.log(
        "FMCODEX_HAND_MICRO_PRODUCTION_IMPORT_ITEM=PASS "
        f"asset={asset_path} dimensions={asset.blueprint_get_size_x()}x"
        f"{asset.blueprint_get_size_y()} lod_group=TEXTUREGROUP_UI "
        "compression=TC_BC7 mip_generation=TMGS_SHARPEN1 "
        "filter=TF_TRILINEAR never_stream=true srgb=true lod_bias=0"
    )

unreal.log(
    "FMCODEX_HAND_MICRO_PRODUCTION_IMPORT=PASS "
    f"source_count={len(ASSET_NAMES)} import_count={len(ASSET_NAMES)} errors=0"
)

