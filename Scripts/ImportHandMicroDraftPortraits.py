"""Editor-only import for the Stage 6.13.1.3.10 Hand Micro portrait set."""

from pathlib import Path

import unreal


IMPORTS = (
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


project_root = Path(
    unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
)
tasks = []
expected_by_task = {}

for team, asset_name in IMPORTS:
    source_path = (
        project_root
        / "ArtSource"
        / "UI"
        / "PrototypeTeams"
        / team
        / "HandMicroPortraits"
        / f"{asset_name}.png"
    )
    destination = f"/Game/UI/Portraits/PrototypeTeams/{team}/HandMicro"
    expected_asset_path = f"{destination}/{asset_name}"
    if not source_path.is_file():
        raise RuntimeError(f"Hand Micro source image is missing: {source_path}")

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source_path))
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", False)
    task.set_editor_property("save", True)
    tasks.append(task)
    expected_by_task[id(task)] = expected_asset_path

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

for task in tasks:
    expected_asset_path = expected_by_task[id(task)]
    asset_name = expected_asset_path.rsplit("/", 1)[-1]
    expected_object_path = f"{expected_asset_path}.{asset_name}"
    imported_paths = list(task.get_editor_property("imported_object_paths"))
    if expected_object_path not in imported_paths:
        raise RuntimeError(
            f"Expected {expected_object_path}, importer returned {imported_paths}"
        )

    asset = unreal.load_asset(expected_asset_path)
    if not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(f"Imported object is not Texture2D: {expected_asset_path}")
    width = asset.blueprint_get_size_x()
    height = asset.blueprint_get_size_y()
    if (width, height) != EXPECTED_SIZE:
        raise RuntimeError(
            f"Unexpected dimensions {width}x{height}, expected "
            f"{EXPECTED_SIZE[0]}x{EXPECTED_SIZE[1]}: {expected_asset_path}"
        )

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
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Failed to save package: {expected_asset_path}")

    unreal.log(
        "FMCODEX_HAND_MICRO_DRAFT_IMPORT "
        f"asset={expected_asset_path} dimensions={width}x{height} "
        "lod_group=TEXTUREGROUP_UI compression=TC_BC7 "
        "mip_generation=TMGS_SHARPEN1 filter=TF_TRILINEAR "
        "never_stream=true srgb=true saved=true"
    )

unreal.log(f"FMCODEX_HAND_MICRO_DRAFT_IMPORT_COUNT={len(IMPORTS)}")
unreal.log("FMCODEX_HAND_MICRO_DRAFT_IMPORT=PASS")
