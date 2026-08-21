"""Editor-only, repeatable import for production Shared Portraits."""

from pathlib import Path
import sys

import unreal

sys.dont_write_bytecode = True

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from SharedPortraitImportCatalog import (  # noqa: E402
    RUNTIME_SIZE,
    asset_path,
    destination_path,
    load_catalog,
    runtime_derivative_path,
    select_entries,
    validate_source_png,
)


def texture_size(texture: unreal.Texture2D) -> tuple[int, int]:
    return texture.blueprint_get_size_x(), texture.blueprint_get_size_y()


project_root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
entries = select_entries(load_catalog(project_root))
tasks = []
expected_by_task = {}

for entry in entries:
    asset_name = entry["assetName"]
    source = runtime_derivative_path(project_root, entry)
    destination = destination_path(entry)
    expected_asset_path = asset_path(entry)
    if not source.is_file():
        raise RuntimeError(f"Shared Portrait runtime derivative is missing: {source}")
    validate_source_png(source, RUNTIME_SIZE)
    unreal.log(
        "FMCODEX_SHARED_PORTRAIT_RUNTIME_SOURCE=PASS "
        f"player_key={entry['playerKey']} source={source} "
        f"dimensions={RUNTIME_SIZE[0]}x{RUNTIME_SIZE[1]} rgb=true opaque=true"
    )

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source))
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
    if (width, height) != RUNTIME_SIZE:
        raise RuntimeError(
            f"Unexpected dimensions {width}x{height}, expected "
            f"{RUNTIME_SIZE[0]}x{RUNTIME_SIZE[1]}: {expected_asset_path}"
        )
    asset.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    asset.set_editor_property(
        "compression_settings", unreal.TextureCompressionSettings.TC_BC7
    )
    asset.set_editor_property(
        "mip_gen_settings", unreal.TextureMipGenSettings.TMGS_SHARPEN1
    )
    asset.set_editor_property("filter", unreal.TextureFilter.TF_TRILINEAR)
    # UE5.3 forces non-power-of-two sources to NeverStream while caching
    # platform data. The required 512x768 derivative is therefore intentionally
    # non-streaming; padding it to 512x1024 would alter the art contract.
    asset.set_editor_property("never_stream", True)
    asset.set_editor_property("srgb", True)
    asset.set_editor_property("lod_bias", 0)
    asset.modify()
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Failed to save package: {expected_asset_path}")

    unreal.log(
        "FMCODEX_PROTOTYPE_TEAM_IMPORT "
        f"source={task.get_editor_property('filename')} "
        f"asset={expected_asset_path} class={asset.get_class().get_name()} "
        f"dimensions={width}x{height} lod_group=TEXTUREGROUP_UI "
        "compression=TC_BC7 srgb=true mip_generation=TMGS_SHARPEN1 "
        "filter=TF_TRILINEAR lod_bias=0 never_stream=true "
        "streaming_equivalent=UE53_NPOT_UI saved=true"
    )

unreal.log(f"FMCODEX_PROTOTYPE_TEAM_IMPORT=PASS selected={len(entries)}")
