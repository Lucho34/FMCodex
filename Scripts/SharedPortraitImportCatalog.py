"""Shared catalog and structural source checks for production portraits."""

from __future__ import annotations

import json
import os
import struct
from pathlib import Path


BATCH_ENVIRONMENT_VARIABLE = "FMCODEX_SHARED_PORTRAIT_PLAYER_KEYS"
MASTER_SIZE = (1024, 1536)
RUNTIME_SIZE = (512, 768)
PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def load_catalog(project_root: Path) -> list[dict[str, str]]:
    manifest_path = (
        project_root
        / "ArtSource"
        / "UI"
        / "PrototypeTeams"
        / "SharedPortraitImportManifest.json"
    )
    with manifest_path.open("r", encoding="utf-8") as stream:
        manifest = json.load(stream)
    if manifest.get("schemaVersion") != 1:
        raise RuntimeError(f"Unsupported Shared Portrait manifest: {manifest_path}")
    entries = manifest.get("entries")
    if not isinstance(entries, list) or not entries:
        raise RuntimeError(f"Shared Portrait manifest has no entries: {manifest_path}")

    player_keys: set[str] = set()
    asset_names: set[str] = set()
    for entry in entries:
        player_key = entry.get("playerKey", "")
        team = entry.get("team", "")
        asset_name = entry.get("assetName", "")
        if team not in ("Arsenal", "ManchesterCity"):
            raise RuntimeError(f"Invalid Shared Portrait team: {entry}")
        prefix = f"Prototype.{team}."
        if not player_key.startswith(prefix):
            raise RuntimeError(f"PlayerKey/team mismatch: {entry}")
        suffix = player_key.removeprefix(prefix)
        expected_name = f"T_Prototype_{team}_{suffix}_01"
        if asset_name != expected_name:
            raise RuntimeError(
                f"PlayerKey/asset mismatch: expected {expected_name}, got {asset_name}"
            )
        if player_key in player_keys or asset_name in asset_names:
            raise RuntimeError(f"Duplicate Shared Portrait entry: {entry}")
        player_keys.add(player_key)
        asset_names.add(asset_name)
    return entries


def select_entries(entries: list[dict[str, str]]) -> list[dict[str, str]]:
    raw_selection = os.environ.get(BATCH_ENVIRONMENT_VARIABLE, "").strip()
    if not raw_selection:
        return entries
    requested = [key.strip() for key in raw_selection.split(";") if key.strip()]
    if len(requested) != len(set(requested)):
        raise RuntimeError(f"Duplicate requested PlayerKey: {raw_selection}")
    by_player_key = {entry["playerKey"]: entry for entry in entries}
    unknown = [key for key in requested if key not in by_player_key]
    if unknown:
        raise RuntimeError(f"Unknown Shared Portrait PlayerKey(s): {unknown}")
    return [by_player_key[key] for key in requested]


def master_path(project_root: Path, entry: dict[str, str]) -> Path:
    return (
        project_root
        / "ArtSource"
        / "UI"
        / "PrototypeTeams"
        / entry["team"]
        / "Portraits"
        / f"{entry['assetName']}.png"
    )


def runtime_derivative_path(project_root: Path, entry: dict[str, str]) -> Path:
    return (
        project_root
        / "ContentSource"
        / "UI"
        / "SharedPortraitRuntime"
        / entry["team"]
        / f"{entry['assetName']}.png"
    )


def provenance_path(project_root: Path) -> Path:
    return (
        project_root
        / "ContentSource"
        / "UI"
        / "SharedPortraitRuntime"
        / "SharedPortraitRuntimeProvenance.json"
    )


def destination_path(entry: dict[str, str]) -> str:
    return f"/Game/UI/Portraits/PrototypeTeams/{entry['team']}"


def asset_path(entry: dict[str, str]) -> str:
    return f"{destination_path(entry)}/{entry['assetName']}"


def validate_source_png(path: Path, expected_size: tuple[int, int]) -> None:
    data = path.read_bytes()
    if path.suffix.lower() != ".png" or not data.startswith(PNG_SIGNATURE):
        raise RuntimeError(f"Shared Portrait source is not a PNG: {path}")
    if len(data) < 33 or data[12:16] != b"IHDR":
        raise RuntimeError(f"Shared Portrait PNG has no valid IHDR: {path}")
    width, height, bit_depth, color_type, compression, filtering, _ = struct.unpack(
        ">IIBBBBB", data[16:29]
    )
    if (width, height) != expected_size:
        raise RuntimeError(
            f"Unexpected source dimensions {width}x{height}, expected "
            f"{expected_size[0]}x{expected_size[1]}: {path}"
        )
    if bit_depth != 8 or color_type != 2:
        raise RuntimeError(
            f"Shared Portrait must be opaque 8-bit RGB PNG; "
            f"bit_depth={bit_depth} color_type={color_type}: {path}"
        )
    if compression != 0 or filtering != 0:
        raise RuntimeError(f"Unsupported Shared Portrait PNG encoding: {path}")

    chunk_types: list[bytes] = []
    offset = len(PNG_SIGNATURE)
    while offset + 12 <= len(data):
        chunk_length = struct.unpack(">I", data[offset : offset + 4])[0]
        chunk_type = data[offset + 4 : offset + 8]
        chunk_end = offset + 12 + chunk_length
        if chunk_end > len(data):
            raise RuntimeError(f"Shared Portrait PNG has a truncated chunk: {path}")
        chunk_types.append(chunk_type)
        offset = chunk_end
        if chunk_type == b"IEND":
            break
    if b"tRNS" in chunk_types:
        raise RuntimeError(f"Shared Portrait PNG contains transparency: {path}")
    if b"IDAT" not in chunk_types or not chunk_types or chunk_types[-1] != b"IEND":
        raise RuntimeError(f"Shared Portrait PNG is incomplete: {path}")
