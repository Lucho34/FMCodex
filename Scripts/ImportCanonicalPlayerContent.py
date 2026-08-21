#!/usr/bin/env python3
"""Validate the approved 40-player workbook and generate runtime JSON.

The packaged game never reads XLSX. This development-time importer reads the
minimal OOXML surface directly with the Python standard library, validates the
canonical content contract independently of workbook helper formulas, and only
replaces the output after the entire dataset passes.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import re
import sys
import tempfile
import zipfile
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any
from xml.etree import ElementTree as ET


TEAMS = ("Arsenal", "Manchester City")
POSITIONS = {"A", "M", "D", "A/M", "M/D", "GK"}
SKILLS = {"LongShot", "CutInsideShot", "PassControl", "Cross", "ThroughBall"}
OUTFIELD_ATTRIBUTES = ("SHO", "DRI", "PAS", "OFF", "MRK", "TKL", "SPD", "STR", "STA", "LS")
GOALKEEPER_ATTRIBUTES = ("HAN", "POS_GK", "REF", "AER", "ANT", "1V1")
SOURCE_HEADERS = (
    "RosterSlot", "Team", "PlayerId", "中文名", "EnglishName", "Position",
    *OUTFIELD_ATTRIBUTES,
    *GOALKEEPER_ATTRIBUTES,
    "Skill1", "S1_MinTP", "S1_MaxTP",
    "Skill2", "S2_MinTP", "S2_MaxTP",
    "Skill3", "S3_MinTP", "S3_MaxTP", "Notes",
)
CONFIG_SCHEMA_VERSION = 2
RUNTIME_SCHEMA_VERSION = 2
PLAYER_KEY_PATTERN = re.compile(r"^[A-Za-z0-9.]+$")


class ValidationFailure(Exception):
    pass


def column_index(cell_reference: str) -> int:
    match = re.match(r"^([A-Z]+)", cell_reference)
    if match is None:
        raise ValidationFailure(f"Invalid XLSX cell reference: {cell_reference}")
    result = 0
    for character in match.group(1):
        result = result * 26 + ord(character) - ord("A") + 1
    return result - 1


def read_shared_strings(archive: zipfile.ZipFile) -> list[str]:
    path = "xl/sharedStrings.xml"
    if path not in archive.namelist():
        return []
    root = ET.fromstring(archive.read(path))
    values: list[str] = []
    for item in root.findall("{*}si"):
        values.append("".join(node.text or "" for node in item.findall(".//{*}t")))
    return values


def resolve_sheet_path(archive: zipfile.ZipFile, sheet_name: str) -> str:
    workbook = ET.fromstring(archive.read("xl/workbook.xml"))
    relationship_id = None
    for sheet in workbook.findall(".//{*}sheet"):
        if sheet.attrib.get("name") == sheet_name:
            relationship_id = sheet.attrib.get(
                "{http://schemas.openxmlformats.org/officeDocument/2006/relationships}id"
            )
            break
    if relationship_id is None:
        raise ValidationFailure(f"Workbook sheet not found: {sheet_name}")

    relationships = ET.fromstring(archive.read("xl/_rels/workbook.xml.rels"))
    for relationship in relationships.findall("{*}Relationship"):
        if relationship.attrib.get("Id") == relationship_id:
            target = relationship.attrib.get("Target", "")
            if target.startswith("/"):
                return target.lstrip("/")
            return str(Path("xl") / target).replace("\\", "/")
    raise ValidationFailure(f"Workbook relationship not found for sheet: {sheet_name}")


def parse_cell(cell: ET.Element, shared_strings: list[str]) -> Any:
    cell_type = cell.attrib.get("t")
    if cell_type == "inlineStr":
        return "".join(node.text or "" for node in cell.findall(".//{*}t"))
    value = cell.find("{*}v")
    if value is None or value.text is None:
        return None
    raw = value.text
    if cell_type == "s":
        index = int(raw)
        if index < 0 or index >= len(shared_strings):
            raise ValidationFailure(f"Shared string index out of range: {index}")
        return shared_strings[index]
    if cell_type in {"str", "e"}:
        return raw
    try:
        numeric = float(raw)
    except ValueError:
        return raw
    return int(numeric) if numeric.is_integer() else numeric


def read_sheet_rows(workbook_path: Path, sheet_name: str) -> list[list[Any]]:
    try:
        with zipfile.ZipFile(workbook_path) as archive:
            shared_strings = read_shared_strings(archive)
            sheet_path = resolve_sheet_path(archive, sheet_name)
            root = ET.fromstring(archive.read(sheet_path))
    except (KeyError, OSError, zipfile.BadZipFile, ET.ParseError) as error:
        raise ValidationFailure(f"Unable to read XLSX: {error}") from error

    rows: list[list[Any]] = []
    for row in root.findall(".//{*}sheetData/{*}row"):
        values: dict[int, Any] = {}
        for cell in row.findall("{*}c"):
            reference = cell.attrib.get("r", "")
            values[column_index(reference)] = parse_cell(cell, shared_strings)
        if values:
            width = max(values) + 1
            rows.append([values.get(index) for index in range(width)])
        else:
            rows.append([])
    return rows


def text_value(value: Any, label: str, errors: list[str]) -> str:
    if value is None:
        errors.append(f"{label}: value is required")
        return ""
    result = str(value).strip()
    if not result:
        errors.append(f"{label}: value is required")
    return result


def integer_value(value: Any, label: str, errors: list[str]) -> int:
    if isinstance(value, bool) or value is None:
        errors.append(f"{label}: integer is required")
        return 0
    if isinstance(value, int):
        return value
    if isinstance(value, float) and math.isfinite(value) and value.is_integer():
        return int(value)
    if isinstance(value, str) and re.fullmatch(r"[+-]?\d+", value.strip()):
        return int(value.strip())
    errors.append(f"{label}: expected integer, found {value!r}")
    return 0


def optional_text(value: Any) -> str:
    return "" if value is None else str(value).strip()


def validate_config(config: dict[str, Any], errors: list[str]) -> dict[tuple[str, str], dict[str, Any]]:
    if config.get("schemaVersion") != CONFIG_SCHEMA_VERSION:
        errors.append(
            f"Import config schemaVersion must be {CONFIG_SCHEMA_VERSION}"
        )
    version = optional_text(config.get("balanceContentVersion"))
    if not version:
        errors.append("Import config balanceContentVersion is required")
    default_rarity = config.get("defaultRarity")
    if default_rarity not in {"Common", "Regional", "National", "Continental", "WorldClass"}:
        errors.append(f"Import config defaultRarity is invalid: {default_rarity!r}")

    entries = config.get("players")
    if not isinstance(entries, list) or len(entries) != 40:
        errors.append("Import config must contain exactly 40 player mappings")
        return {}

    mapping: dict[tuple[str, str], dict[str, Any]] = {}
    player_keys: set[str] = set()
    for index, entry in enumerate(entries, start=1):
        if not isinstance(entry, dict):
            errors.append(f"Import config player {index}: object is required")
            continue
        team = optional_text(entry.get("team"))
        english_name = optional_text(entry.get("englishName"))
        player_key = optional_text(entry.get("playerKey"))
        display_name = optional_text(entry.get("displayName"))
        join_key = (team, english_name)
        if team not in TEAMS:
            errors.append(f"Import config player {index}: invalid team {team!r}")
        if not english_name:
            errors.append(f"Import config player {index}: englishName is required")
        if join_key in mapping:
            errors.append(f"Import config duplicate mapping: {team} / {english_name}")
        if not PLAYER_KEY_PATTERN.fullmatch(player_key):
            errors.append(f"Import config player {index}: invalid PlayerKey {player_key!r}")
        if not display_name:
            errors.append(f"Import config player {index}: displayName is required")
        if player_key in player_keys:
            errors.append(f"Import config duplicate PlayerKey: {player_key}")
        player_keys.add(player_key)
        mapping[join_key] = entry
    return mapping


def parse_players(rows: list[list[Any]], config: dict[str, Any], workbook_hash: str) -> tuple[dict[str, Any], dict[str, Any]]:
    errors: list[str] = []
    if not rows:
        raise ValidationFailure("Source sheet is empty")
    header = rows[0]
    actual_headers = tuple(header[index] if index < len(header) else None for index in range(len(SOURCE_HEADERS)))
    if actual_headers != SOURCE_HEADERS:
        errors.append(
            "Source columns A:AF do not match the required contract. "
            f"Expected {SOURCE_HEADERS!r}; found {actual_headers!r}"
        )

    config_mapping = validate_config(config, errors)
    header_map = {name: index for index, name in enumerate(SOURCE_HEADERS)}
    players: list[dict[str, Any]] = []
    seen_join_keys: set[tuple[str, str]] = set()
    serials: set[int] = set()
    roster_slots: dict[str, set[int]] = defaultdict(set)

    for sheet_row, raw_row in enumerate(rows[1:], start=2):
        row = list(raw_row) + [None] * max(0, len(SOURCE_HEADERS) - len(raw_row))
        if all(row[index] is None for index in range(len(SOURCE_HEADERS))):
            continue
        row_errors: list[str] = []
        label = f"row {sheet_row}"
        roster_slot = integer_value(row[header_map["RosterSlot"]], f"{label} RosterSlot", row_errors)
        team = text_value(row[header_map["Team"]], f"{label} Team", row_errors)
        display_serial = integer_value(row[header_map["PlayerId"]], f"{label} PlayerId/DisplaySerial", row_errors)
        chinese_name = text_value(row[header_map["中文名"]], f"{label} 中文名", row_errors)
        english_name = text_value(row[header_map["EnglishName"]], f"{label} EnglishName", row_errors)
        position = text_value(row[header_map["Position"]], f"{label} Position", row_errors)

        if team not in TEAMS:
            row_errors.append(f"{label} Team: unsupported value {team!r}")
        if position not in POSITIONS:
            row_errors.append(f"{label} Position: unsupported value {position!r}")
        if not 1 <= roster_slot <= 20:
            row_errors.append(f"{label} RosterSlot: must be 1-20")
        if roster_slot in roster_slots[team]:
            row_errors.append(f"{label} RosterSlot: duplicate {team} slot {roster_slot}")
        roster_slots[team].add(roster_slot)
        if display_serial <= 0:
            row_errors.append(f"{label} DisplaySerial: must be positive")
        if display_serial in serials:
            row_errors.append(f"{label} DisplaySerial: duplicate {display_serial}")
        serials.add(display_serial)

        join_key = (team, english_name)
        identity = config_mapping.get(join_key)
        if identity is None:
            row_errors.append(f"{label}: no stable PlayerKey mapping for {team} / {english_name}")
            identity = {"playerKey": ""}
        else:
            seen_join_keys.add(join_key)

        outfield: dict[str, int] | None = None
        goalkeeper: dict[str, int] | None = None
        if position == "GK":
            goalkeeper = {}
            for attribute in GOALKEEPER_ATTRIBUTES:
                value = integer_value(row[header_map[attribute]], f"{label} {attribute}", row_errors)
                if not 1 <= value <= 6:
                    row_errors.append(f"{label} {attribute}: must be 1-6")
                goalkeeper[attribute] = value
            for attribute in OUTFIELD_ATTRIBUTES:
                if row[header_map[attribute]] is not None:
                    row_errors.append(f"{label} {attribute}: GK field must be blank")
        else:
            outfield = {}
            for attribute in OUTFIELD_ATTRIBUTES:
                value = integer_value(row[header_map[attribute]], f"{label} {attribute}", row_errors)
                if not 1 <= value <= 6:
                    row_errors.append(f"{label} {attribute}: must be 1-6")
                outfield[attribute] = value
            for attribute in GOALKEEPER_ATTRIBUTES:
                if row[header_map[attribute]] is not None:
                    row_errors.append(f"{label} {attribute}: outfield field must be blank")

        skill_assignments: list[dict[str, Any]] = []
        skill_ids: set[str] = set()
        for skill_index in range(1, 4):
            skill_id = optional_text(row[header_map[f"Skill{skill_index}"]])
            min_raw = row[header_map[f"S{skill_index}_MinTP"]]
            max_raw = row[header_map[f"S{skill_index}_MaxTP"]]
            if not skill_id:
                if min_raw is not None or max_raw is not None:
                    row_errors.append(f"{label} Skill{skill_index}: TP range exists without SkillId")
                continue
            if skill_id not in SKILLS:
                row_errors.append(f"{label} Skill{skill_index}: unknown SkillId {skill_id!r}")
            min_tp = integer_value(min_raw, f"{label} Skill{skill_index} MinTP", row_errors)
            max_tp = integer_value(max_raw, f"{label} Skill{skill_index} MaxTP", row_errors)
            if not 2 <= min_tp <= max_tp <= 8:
                row_errors.append(
                    f"{label} Skill{skill_index}: expected 2 <= MinTP <= MaxTP <= 8, "
                    f"found {min_tp}-{max_tp}"
                )
            if skill_id in skill_ids:
                row_errors.append(f"{label}: duplicate SkillId {skill_id}")
            skill_ids.add(skill_id)
            skill_assignments.append({"skillId": skill_id, "minTP": min_tp, "maxTP": max_tp})

        if len(skill_assignments) > 3:
            row_errors.append(f"{label}: more than three skills")
        for tactical_point in range(2, 9):
            eligible = [
                skill["skillId"]
                for skill in skill_assignments
                if skill["minTP"] <= tactical_point <= skill["maxTP"]
            ]
            if len(eligible) > 2:
                row_errors.append(
                    f"{label} TP {tactical_point}: overlap violation for {', '.join(eligible)}"
                )

        presentation = identity.get("presentation", {}) if isinstance(identity, dict) else {}
        if not isinstance(presentation, dict):
            row_errors.append(f"{label}: presentation override must be an object")
            presentation = {}
        rarity = presentation.get("rarity", config.get("defaultRarity"))
        if rarity not in {"Common", "Regional", "National", "Continental", "WorldClass"}:
            row_errors.append(f"{label}: invalid rarity {rarity!r}")

        errors.extend(row_errors)
        players.append({
            "playerKey": identity.get("playerKey", ""),
            "displayName": optional_text(identity.get("displayName")),
            "team": team,
            "rosterSlot": roster_slot,
            "displaySerial": display_serial,
            "chineseName": chinese_name,
            "englishName": english_name,
            "position": position,
            "outfieldAttributes": outfield,
            "goalkeeperAttributes": goalkeeper,
            "skills": skill_assignments,
            "notes": optional_text(row[header_map["Notes"]]),
            "presentation": {
                "nationality": optional_text(presentation.get("nationality")),
                "birthDate": optional_text(presentation.get("birthDate")),
                "heightCm": int(presentation.get("heightCm", 0)),
                "weightKg": int(presentation.get("weightKg", 0)),
                "rarity": rarity,
            },
        })

    if len(players) != 40:
        errors.append(f"Expected exactly 40 player rows, found {len(players)}")
    counts = Counter(player["team"] for player in players)
    for team in TEAMS:
        if counts[team] != 20:
            errors.append(f"Expected exactly 20 {team} players, found {counts[team]}")
        if roster_slots[team] != set(range(1, 21)):
            errors.append(f"{team} RosterSlot values must be exactly 1-20")
    missing_config = set(config_mapping) - seen_join_keys
    if missing_config:
        errors.append(
            "Import config entries absent from workbook: "
            + ", ".join(f"{team} / {name}" for team, name in sorted(missing_config))
        )
    player_keys = [player["playerKey"] for player in players]
    if len(set(player_keys)) != len(player_keys):
        errors.append("PlayerKey values are not unique after workbook join")

    if errors:
        raise ValidationFailure("\n".join(errors))

    players.sort(key=lambda player: (TEAMS.index(player["team"]), player["rosterSlot"]))
    skill_distribution = Counter(len(player["skills"]) for player in players)
    runtime = {
        "schemaVersion": RUNTIME_SCHEMA_VERSION,
        "balanceContentVersion": config["balanceContentVersion"],
        "sourceWorkbook": "FMCodex_40_Player_Attribute_Skill_PointRules.xlsx",
        "sourceWorkbookSha256": workbook_hash,
        "sourceSheet": config["sourceSheet"],
        "players": players,
    }
    summary = {
        "players": len(players),
        "teams": dict(sorted(counts.items())),
        "outfield": sum(player["position"] != "GK" for player in players),
        "goalkeepers": sum(player["position"] == "GK" for player in players),
        "skillCountDistribution": {str(key): skill_distribution[key] for key in range(4)},
        "skillAssignments": sum(len(player["skills"]) for player in players),
        "tpOverlapChecks": len(players) * 7,
        "tpOverlapViolations": 0,
        "balanceContentVersion": config["balanceContentVersion"],
    }
    return runtime, summary


def main() -> int:
    repository_root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, type=Path, help="Approved XLSX authoring workbook")
    parser.add_argument(
        "--config",
        type=Path,
        default=repository_root / "ContentSource/PlayerContent/CanonicalPlayerImportConfig.json",
        help="Stable PlayerKey and presentation-sidecar configuration",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=repository_root / "Content/Data/CanonicalPlayerContent.json",
        help="Generated runtime JSON",
    )
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true", help="Atomically replace output after full validation")
    mode.add_argument("--check", action="store_true", help="Validate workbook and require exact output equality")
    args = parser.parse_args()

    try:
        workbook_bytes = args.input.read_bytes()
        config = json.loads(args.config.read_text(encoding="utf-8-sig"))
        sheet_name = config.get("sourceSheet", "")
        rows = read_sheet_rows(args.input, sheet_name)
        runtime, summary = parse_players(
            rows,
            config,
            hashlib.sha256(workbook_bytes).hexdigest(),
        )
        canonical_text = json.dumps(runtime, ensure_ascii=False, indent=2) + "\n"
        if args.check:
            if not args.output.is_file():
                raise ValidationFailure(f"Canonical runtime output is missing: {args.output}")
            existing_text = args.output.read_text(encoding="utf-8-sig")
            if existing_text != canonical_text:
                raise ValidationFailure(
                    "Source workbook and canonical runtime content differ. "
                    "Run the importer with --write after reviewing the workbook change."
                )
            summary["sourceToCanonicalExactMatch"] = "40/40"
            summary["mode"] = "check"
        else:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            with tempfile.NamedTemporaryFile(
                mode="w",
                encoding="utf-8",
                newline="\n",
                dir=args.output.parent,
                prefix=args.output.name + ".",
                suffix=".tmp",
                delete=False,
            ) as temporary:
                temporary.write(canonical_text)
                temporary_path = Path(temporary.name)
            temporary_path.replace(args.output)
            summary["sourceToCanonicalExactMatch"] = "40/40"
            summary["mode"] = "write"
        print(json.dumps(summary, ensure_ascii=False, sort_keys=True))
        return 0
    except (OSError, json.JSONDecodeError, ValidationFailure, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
