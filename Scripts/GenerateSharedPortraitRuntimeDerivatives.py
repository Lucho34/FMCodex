"""Generate deterministic 512x768 Shared Portrait runtime-source PNGs."""

from __future__ import annotations

import hashlib
import io
import json
import os
from pathlib import Path
import sys

sys.dont_write_bytecode = True

from PIL import Image, __version__ as PILLOW_VERSION

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from SharedPortraitImportCatalog import (  # noqa: E402
    MASTER_SIZE,
    RUNTIME_SIZE,
    asset_path,
    load_catalog,
    master_path,
    provenance_path,
    runtime_derivative_path,
    select_entries,
    validate_source_png,
)


GENERATOR_VERSION = 1
RESAMPLING_CONTRACT = "Pillow.Resampling.LANCZOS"
ENCODER_CONTRACT = "PNG RGB compress_level=9 optimize=false metadata=none"


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def sha256_file(path: Path) -> str:
    return sha256_bytes(path.read_bytes())


def encode_runtime_derivative(master: Path) -> bytes:
    validate_source_png(master, MASTER_SIZE)
    with Image.open(master) as source:
        source.load()
        if source.mode != "RGB" or source.size != MASTER_SIZE:
            raise RuntimeError(
                f"Shared Portrait Master must decode as opaque RGB "
                f"{MASTER_SIZE[0]}x{MASTER_SIZE[1]}: {master}"
            )
        derivative = source.resize(RUNTIME_SIZE, Image.Resampling.LANCZOS)
        if derivative.mode != "RGB" or derivative.size != RUNTIME_SIZE:
            raise RuntimeError(f"Invalid runtime derivative in memory: {master}")
        stream = io.BytesIO()
        derivative.save(
            stream,
            format="PNG",
            optimize=False,
            compress_level=9,
        )
        return stream.getvalue()


def write_if_changed(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.is_file() and path.read_bytes() == data:
        return
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_bytes(data)
    os.replace(temporary, path)


def load_existing_provenance(path: Path) -> dict[str, dict[str, object]]:
    if not path.is_file():
        return {}
    document = json.loads(path.read_text(encoding="utf-8"))
    if document.get("schemaVersion") != 1:
        raise RuntimeError(f"Unsupported Shared Portrait provenance: {path}")
    entries = document.get("entries", [])
    return {str(entry["playerKey"]): entry for entry in entries}


def generate_selected(project_root: Path) -> list[dict[str, object]]:
    catalog = load_catalog(project_root)
    selected = select_entries(catalog)
    records_by_key = load_existing_provenance(provenance_path(project_root))

    for entry in selected:
        master = master_path(project_root, entry)
        derivative = runtime_derivative_path(project_root, entry)
        if not master.is_file():
            raise RuntimeError(f"Shared Portrait Master is missing: {master}")

        first_encoding = encode_runtime_derivative(master)
        second_encoding = encode_runtime_derivative(master)
        if first_encoding != second_encoding:
            raise RuntimeError(
                f"Runtime derivative generation is not deterministic: {master}"
            )
        write_if_changed(derivative, first_encoding)
        validate_source_png(derivative, RUNTIME_SIZE)

        runtime_asset_path = asset_path(entry)
        record: dict[str, object] = {
            "playerKey": entry["playerKey"],
            "masterSourcePath": master.relative_to(project_root).as_posix(),
            "masterDimensions": list(MASTER_SIZE),
            "masterSha256": sha256_file(master),
            "runtimeDerivativePath": derivative.relative_to(project_root).as_posix(),
            "runtimeDerivativeDimensions": list(RUNTIME_SIZE),
            "runtimeDerivativeSha256": sha256_bytes(first_encoding),
            "runtimeAssetPath": (
                f"{runtime_asset_path}.{entry['assetName']}"
            ),
            "generatorVersion": GENERATOR_VERSION,
            "pillowVersion": PILLOW_VERSION,
            "resampling": RESAMPLING_CONTRACT,
            "encoder": ENCODER_CONTRACT,
            "visualStatus": entry.get("visualStatus", "UNREVIEWED"),
        }
        if entry.get("replacementHistory"):
            record["replacementHistory"] = entry["replacementHistory"]
        records_by_key[entry["playerKey"]] = record
        print(
            "FMCODEX_SHARED_PORTRAIT_DERIVATIVE "
            f"player_key={entry['playerKey']} "
            f"master_sha256={record['masterSha256']} "
            f"derivative_sha256={record['runtimeDerivativeSha256']} "
            f"dimensions={RUNTIME_SIZE[0]}x{RUNTIME_SIZE[1]} "
            "rgb=true opaque=true deterministic=true"
        )

    ordered_records = [
        records_by_key[entry["playerKey"]]
        for entry in catalog
        if entry["playerKey"] in records_by_key
    ]
    provenance_document = {
        "schemaVersion": 1,
        "description": (
            "Generated Shared Portrait runtime-source provenance; Art Masters "
            "remain the source of truth."
        ),
        "entries": ordered_records,
    }
    provenance_bytes = (
        json.dumps(
            provenance_document,
            ensure_ascii=False,
            indent=2,
        )
        + "\n"
    ).encode("utf-8")
    write_if_changed(provenance_path(project_root), provenance_bytes)
    return ordered_records


def main() -> None:
    project_root = SCRIPT_DIR.parent
    records = generate_selected(project_root)
    selected_count = len(select_entries(load_catalog(project_root)))
    print(
        "FMCODEX_SHARED_PORTRAIT_DERIVATIVE_GENERATION=PASS "
        f"selected={selected_count} provenance_records={len(records)}"
    )


if __name__ == "__main__":
    main()
