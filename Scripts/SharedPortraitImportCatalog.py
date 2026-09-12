"""Shared catalog and structural source checks for production portraits."""

from __future__ import annotations

import json
import hashlib
import math
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
    if manifest.get("schemaVersion") not in (1, 2):
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
        if entry.get("masterSourcePath"):
            expected_master = f"ArtSource/UI/PlayerMaster/{player_key}/Master.png"
            if entry["masterSourcePath"] != expected_master:
                raise RuntimeError(f"Noncanonical master path: {entry['masterSourcePath']}")
            if entry.get("compositionProfile") != "HeroBust_v1":
                raise RuntimeError(f"Unsupported composition profile: {player_key}")
            if not isinstance(entry.get("masterRevision"), int) or entry["masterRevision"] < 1:
                raise RuntimeError(f"Invalid master revision: {player_key}")
            digest = entry.get("masterSha256", "")
            if len(digest) != 64 or any(c not in "0123456789abcdefABCDEF" for c in digest):
                raise RuntimeError(f"Missing master hash: {player_key}")
            if set(entry.get("cropOverrides", {})) - {"handCropRect", "fullCropRect"}:
                raise RuntimeError(f"Unknown crop override: {player_key}")
            for role in ROLE_SIZES:
                resolved_crop(dict(entry, runtimeRole=role))
        player_keys.add(player_key)
        asset_names.add(asset_name)
    return entries


def select_entries(entries: list[dict[str, str]]) -> list[dict[str, str]]:
    raw_selection = os.environ.get(BATCH_ENVIRONMENT_VARIABLE, "").strip()
    if not raw_selection:
        if any(is_canonical(entry) for entry in entries):
            raise RuntimeError("Explicit PlayerKeys selection is required during canonical migration")
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
    if is_canonical(entry):
        return project_root / entry["masterSourcePath"]
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
    if is_canonical(entry):
        return project_root / "ContentSource/UI/PlayerPortraitRuntime" / entry["playerKey"] / (runtime_role(entry) + ".png")
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
    if is_canonical(entry):
        return "/Game/UI/Portraits/PrototypeTeams/Canonical/" + entry["playerKey"].replace(".", "_")
    return f"/Game/UI/Portraits/PrototypeTeams/{entry['team']}"


def asset_path(entry: dict[str, str]) -> str:
    return f"{destination_path(entry)}/{runtime_asset_name(entry)}"


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


ROLE_SIZES = {"Shared": (512, 768), "Hand": (192, 128), "Full": (768, 1152)}


def is_canonical(entry):
    return bool(entry.get("masterSourcePath"))


def runtime_role(entry):
    return entry.get("runtimeRole", "Shared")


def runtime_size(entry):
    return ROLE_SIZES[runtime_role(entry)] if is_canonical(entry) else RUNTIME_SIZE


def runtime_asset_name(entry):
    if is_canonical(entry):
        return "T_" + entry["playerKey"].replace(".", "_") + "_" + runtime_role(entry)
    return entry["assetName"]


def selected_runtime_roles():
    raw = os.environ.get("FMCODEX_PLAYER_ART_RUNTIME_ROLES", "").strip()
    if not raw:
        raise RuntimeError("Explicit RuntimeRoles selection is required for canonical generation/import")
    roles = tuple(part.strip() for part in raw.split(";") if part.strip())
    if not roles or len(set(roles)) != len(roles) or set(roles) - set(ROLE_SIZES):
        raise RuntimeError(f"Invalid explicit runtime roles: {raw}")
    return roles


def expand_runtime_entries(entries, roles=None):
    roles = (selected_runtime_roles() if any(is_canonical(e) for e in entries) else ("Shared",)) if roles is None else roles
    if not roles or len(set(roles)) != len(roles) or set(roles) - set(ROLE_SIZES):
        raise RuntimeError(f"Invalid explicit runtime roles: {roles}")
    return [dict(entry, runtimeRole=role) for entry in entries
            for role in (roles if is_canonical(entry) else ("Shared",))]


def hand_composition(entry):
    profile = entry.get("handCompositionProfile", "CropOnly_v1")
    if profile not in ("CropOnly_v1", "BalancedBust_v2"):
        raise RuntimeError(f"Unknown Hand composition: {profile}")
    return profile


def resolved_crop(entry):
    role = runtime_role(entry)
    profile = hand_composition(entry)
    reframe = role == "Hand" and profile == "BalancedBust_v2"
    default = ([0,.055,1,.5] if reframe else [0, 0.045, 1, 4/9]) if role == "Hand" else [0, 0, 1, 1]
    field = "handCropRect" if role == "Hand" else "fullCropRect"
    rect = entry.get("cropOverrides", {}).get(field, default) if role != "Shared" else default
    if not isinstance(rect, list) or len(rect) != 4 or any(
            isinstance(v, bool) or not isinstance(v, (int, float)) or not math.isfinite(v) for v in rect):
        raise RuntimeError(f"Invalid normalized crop: {rect}")
    x, y, w, h = rect
    if x < 0 or y < 0 or w <= 0 or h <= 0 or x+w > 1+1e-9 or y+h > 1+1e-9:
        raise RuntimeError(f"Out-of-bounds normalized crop: {rect}")
    ratio = 4/9 if role == "Hand" else 1
    if reframe and (w < .8 or h < .44 or h > .6):
        raise RuntimeError(f"Hand bust crop exceeds bounded profile: {rect}")
    if not reframe and not math.isclose(h, w*ratio, abs_tol=1e-8):
        raise RuntimeError(f"Crop would distort {role}: {rect}")
    return rect


def family_provenance_path(project_root):
    return project_root / "ContentSource/UI/PlayerPortraitRuntime/PlayerArtProvenance.json"


def crop_metadata_hash(entry):
    value = {"compositionProfile": entry["compositionProfile"], "cropOverrides": entry.get("cropOverrides", {})}
    if entry.get("handCompositionProfile"):
        value["handCompositionProfile"] = hand_composition(entry)
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(",", ":")).encode()).hexdigest().upper()


def validate_generated_source(project_root, entry):
    """Importer/validator gate: stale source or crop may never silently reimport."""
    if not is_canonical(entry):
        return None
    records = json.loads(family_provenance_path(project_root).read_text(encoding="utf-8"))["entries"]
    record = next((r for r in records if r["playerKey"] == entry["playerKey"]), None)
    actual_master = hashlib.sha256(master_path(project_root, entry).read_bytes()).hexdigest().upper()
    if not record or actual_master != entry["masterSha256"].upper() or record["masterSha256"] != actual_master:
        raise RuntimeError(f"Stale master provenance: {entry['playerKey']}")
    if (record["cropMetadataSha256"] != crop_metadata_hash(entry)
        or record["masterRevision"] != entry["masterRevision"]
        or record["masterSourcePath"] != entry["masterSourcePath"]):
        raise RuntimeError(f"Stale crop/revision provenance: {entry['playerKey']}")
    role = record.get("roles", {}).get(runtime_role(entry))
    if role is None:
        raise RuntimeError(f"No generated provenance for selected role: {runtime_role(entry)}")
    if role["cropRect"] != resolved_crop(entry):
        raise RuntimeError("Stale role crop provenance")
    if runtime_role(entry) == "Hand" and role.get("handCompositionProfile", "CropOnly_v1") != hand_composition(entry):
        raise RuntimeError("Stale Hand composition provenance")
    if runtime_role(entry) == "Hand" and hand_composition(entry) == "BalancedBust_v2":
        generator = project_root / "Scripts/GenerateSharedPortraitRuntimeDerivatives.py"
        if role.get("generatorSha256") != hashlib.sha256(generator.read_bytes()).hexdigest().upper():
            raise RuntimeError("Stale Hand generator provenance")
    path = runtime_derivative_path(project_root, entry)
    if role["runtimeDerivativePath"] != path.relative_to(project_root).as_posix():
        raise RuntimeError("Incorrect derivative path")
    if hashlib.sha256(path.read_bytes()).hexdigest().upper() != role["runtimeDerivativeSha256"]:
        raise RuntimeError("Derivative bytes differ from generated provenance")
    expected = asset_path(entry) + "." + runtime_asset_name(entry)
    if role["runtimeAssetPath"] != expected or role["dimensions"] != list(runtime_size(entry)):
        raise RuntimeError("Incorrect derived role binding/dimensions")
    return role
