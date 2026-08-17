"""Editor-only import for the Stage 6.13.1.3.11.5 Full Card artwork pilot."""

from pathlib import Path

import unreal


IMPORTS = (
    ("Arsenal", "BukayoSaka"),
    ("Arsenal", "DavidRaya"),
    ("ManchesterCity", "Rodri"),
    ("ManchesterCity", "GianluigiDonnarumma"),
)
ASSET_SUFFIX = "FullCardPilot_02"
EXPECTED_SIZE = (1024, 1536)


def texture_size(texture: unreal.Texture2D) -> tuple[int, int]:
    return texture.blueprint_get_size_x(), texture.blueprint_get_size_y()


project_root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
tasks = []
expected_by_task = {}

for team, player in IMPORTS:
    asset_name = f"T_Prototype_{team}_{player}_{ASSET_SUFFIX}"
    source_path = (
        project_root
        / "ArtSource"
        / "UI"
        / "PrototypeTeams"
        / team
        / "Portraits"
        / f"{asset_name}.png"
    )
    destination = f"/Game/UI/Portraits/PrototypeTeams/{team}"
    expected_asset_path = f"{destination}/{asset_name}"
    if not source_path.is_file():
        raise RuntimeError(f"Full Card pilot source image is missing: {source_path}")

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
    width, height = texture_size(asset)
    if (width, height) != EXPECTED_SIZE:
        raise RuntimeError(
            f"Unexpected dimensions {width}x{height}, expected "
            f"{EXPECTED_SIZE[0]}x{EXPECTED_SIZE[1]}: {expected_asset_path}"
        )
    asset.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    asset.set_editor_property("srgb", True)
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Failed to save package: {expected_asset_path}")

    unreal.log(
        "FMCODEX_FULL_CARD_PILOT_IMPORT "
        f"source={task.get_editor_property('filename')} "
        f"asset={expected_asset_path} class={asset.get_class().get_name()} "
        f"dimensions={width}x{height} lod_group=TEXTUREGROUP_UI "
        "srgb=true saved=true"
    )

unreal.log("FMCODEX_FULL_CARD_PILOT_IMPORT=PASS")
