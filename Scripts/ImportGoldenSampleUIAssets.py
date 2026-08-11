"""Editor-only, repeatable import for the Stage 6.12 Golden Sample textures."""

from pathlib import Path

import unreal


IMPORTS = (
    (
        "ArtSource/UI/GoldenSample/Cards/T_Golden_CardFrame_01.png",
        "/Game/UI/Cards/GoldenSample",
        "/Game/UI/Cards/GoldenSample/T_Golden_CardFrame_01",
        (1024, 1536),
    ),
    (
        "ArtSource/UI/GoldenSample/Portraits/T_Golden_PlayerPortrait_01.png",
        "/Game/UI/Portraits/GoldenSample",
        "/Game/UI/Portraits/GoldenSample/T_Golden_PlayerPortrait_01",
        (1024, 1536),
    ),
    (
        "ArtSource/UI/GoldenSample/Icons/T_Golden_Role_Forward_01.png",
        "/Game/UI/Icons/GoldenSample",
        "/Game/UI/Icons/GoldenSample/T_Golden_Role_Forward_01",
        (1254, 1254),
    ),
    (
        "ArtSource/UI/GoldenSample/Icons/T_Golden_Skill_LongShot_01.png",
        "/Game/UI/Icons/GoldenSample",
        "/Game/UI/Icons/GoldenSample/T_Golden_Skill_LongShot_01",
        (1254, 1254),
    ),
)


def texture_size(texture: unreal.Texture2D) -> tuple[int, int]:
    return texture.blueprint_get_size_x(), texture.blueprint_get_size_y()


project_root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
tasks = []
expected_by_task = {}

for relative_source, destination_path, expected_asset_path, expected_size in IMPORTS:
    source_path = project_root / relative_source
    if not source_path.is_file():
        raise RuntimeError(f"Golden Sample source image is missing: {source_path}")

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source_path))
    task.set_editor_property("destination_path", destination_path)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", False)
    task.set_editor_property("save", True)
    tasks.append(task)
    expected_by_task[id(task)] = (expected_asset_path, expected_size)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

for task in tasks:
    expected_asset_path, expected_size = expected_by_task[id(task)]
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
    if (width, height) != expected_size:
        raise RuntimeError(
            f"Unexpected dimensions {width}x{height}, expected {expected_size}: "
            f"{expected_asset_path}"
        )
    asset.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Failed to save package: {expected_asset_path}")

    unreal.log(
        "FMCODEX_GOLDEN_IMPORT "
        f"source={task.get_editor_property('filename')} "
        f"asset={expected_asset_path} class={asset.get_class().get_name()} "
        f"dimensions={width}x{height} lod_group=TEXTUREGROUP_UI saved=true"
    )

unreal.log("FMCODEX_GOLDEN_IMPORT=PASS")
