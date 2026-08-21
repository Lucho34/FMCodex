"""Focused deterministic tests for the Shared Portrait derivative pipeline."""

from __future__ import annotations

import hashlib
import io
import json
from pathlib import Path
import sys
import unittest

sys.dont_write_bytecode = True

from PIL import Image

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from GenerateSharedPortraitRuntimeDerivatives import (  # noqa: E402
    encode_runtime_derivative,
)
from SharedPortraitImportCatalog import (  # noqa: E402
    MASTER_SIZE,
    RUNTIME_SIZE,
    asset_path,
    load_catalog,
    master_path,
    provenance_path,
    runtime_derivative_path,
    validate_source_png,
)


FIXTURE_KEYS = (
    "Prototype.Arsenal.GabrielMagalhaes",
    "Prototype.ManchesterCity.ErlingHaaland",
)
GABRIEL_V3_CANDIDATE_STATUS = (
    "V3 REFINEMENT CANDIDATE IMPORTED — PENDING FINAL MANUAL ARTWORK GATE"
)
HAALAND_V2_CANDIDATE_STATUS = "V2 CANDIDATE IMPORTED — PENDING MANUAL PIE GATE"
V1_VISUAL_FAIL_STATUS = "VISUAL CONFORMANCE FAIL — REQUIRES V2 ART MASTER"
GABRIEL_V2_SUPERSEDED_STATUS = (
    "TECHNICALLY CONFORMING — SUPERSEDED BY V3 COMPOSITION REFINEMENT"
)


class SharedPortraitRuntimeDerivativePipelineTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.project_root = SCRIPT_DIR.parent
        cls.catalog = load_catalog(cls.project_root)
        cls.by_key = {entry["playerKey"]: entry for entry in cls.catalog}
        provenance_document = json.loads(
            provenance_path(cls.project_root).read_text(encoding="utf-8")
        )
        cls.provenance_by_key = {
            entry["playerKey"]: entry
            for entry in provenance_document["entries"]
        }

    def test_fixture_identity_candidate_status_and_history_are_explicit(self) -> None:
        gabriel = self.by_key[FIXTURE_KEYS[0]]
        haaland = self.by_key[FIXTURE_KEYS[1]]
        self.assertEqual(gabriel["visualStatus"], GABRIEL_V3_CANDIDATE_STATUS)
        self.assertEqual(len(gabriel["replacementHistory"]), 2)
        self.assertEqual(
            gabriel["replacementHistory"][1]["visualStatus"],
            GABRIEL_V2_SUPERSEDED_STATUS,
        )
        self.assertEqual(haaland["visualStatus"], HAALAND_V2_CANDIDATE_STATUS)
        self.assertEqual(len(haaland["replacementHistory"]), 1)
        for player_key in FIXTURE_KEYS:
            entry = self.by_key[player_key]
            self.assertEqual(
                entry["replacementHistory"][0]["visualStatus"],
                V1_VISUAL_FAIL_STATUS,
            )
            self.assertTrue(player_key.startswith("Prototype."))
            self.assertNotIn("DisplaySerial", player_key)
            self.assertNotRegex(entry["assetName"], r"_[0-9]{3}_")

    def test_master_contract_and_runtime_separation(self) -> None:
        for player_key in FIXTURE_KEYS:
            entry = self.by_key[player_key]
            master = master_path(self.project_root, entry)
            validate_source_png(master, MASTER_SIZE)
            self.assertIn("ArtSource", master.parts)
            self.assertNotIn("Content", master.parts)
            self.assertEqual(MASTER_SIZE[0] * 3, MASTER_SIZE[1] * 2)

    def test_derivative_is_deterministic_rgb_and_exact_size(self) -> None:
        for player_key in FIXTURE_KEYS:
            entry = self.by_key[player_key]
            master = master_path(self.project_root, entry)
            first = encode_runtime_derivative(master)
            second = encode_runtime_derivative(master)
            self.assertEqual(first, second)
            with Image.open(io.BytesIO(first)) as derivative:
                derivative.load()
                self.assertEqual(derivative.size, RUNTIME_SIZE)
                self.assertEqual(derivative.mode, "RGB")
            self.assertEqual(RUNTIME_SIZE[0] * 3, RUNTIME_SIZE[1] * 2)

    def test_generated_derivative_and_provenance_hashes_match(self) -> None:
        for player_key in FIXTURE_KEYS:
            entry = self.by_key[player_key]
            master = master_path(self.project_root, entry)
            derivative = runtime_derivative_path(self.project_root, entry)
            validate_source_png(derivative, RUNTIME_SIZE)
            record = self.provenance_by_key[player_key]
            self.assertEqual(
                record["masterSha256"],
                hashlib.sha256(master.read_bytes()).hexdigest().upper(),
            )
            self.assertEqual(
                record["runtimeDerivativeSha256"],
                hashlib.sha256(derivative.read_bytes()).hexdigest().upper(),
            )
            self.assertEqual(record["masterDimensions"], list(MASTER_SIZE))
            self.assertEqual(
                record["runtimeDerivativeDimensions"], list(RUNTIME_SIZE)
            )
            expected_status = (
                GABRIEL_V3_CANDIDATE_STATUS
                if player_key == FIXTURE_KEYS[0]
                else HAALAND_V2_CANDIDATE_STATUS
            )
            self.assertEqual(record["visualStatus"], expected_status)
            self.assertEqual(
                record["replacementHistory"][0]["visualStatus"],
                V1_VISUAL_FAIL_STATUS,
            )
            self.assertNotEqual(
                record["masterSha256"],
                record["replacementHistory"][0]["masterSha256"],
            )
            self.assertNotEqual(
                record["runtimeDerivativeSha256"],
                record["replacementHistory"][0]["runtimeDerivativeSha256"],
            )
        gabriel_record = self.provenance_by_key[FIXTURE_KEYS[0]]
        self.assertEqual(len(gabriel_record["replacementHistory"]), 2)
        self.assertEqual(
            gabriel_record["replacementHistory"][1]["masterSha256"],
            "DEFACB5F836E6B304956675244A014C8C9968F2A50E5717343F80F6B3F577427",
        )
        self.assertEqual(
            gabriel_record["replacementHistory"][1]["runtimeDerivativeSha256"],
            "7079A5E8AF6E3A1C0B806D9C1BE2EB85D09D0DF7B4161A4A4993BBAFAE1CA55C",
        )

    def test_runtime_paths_are_stable_and_generated_sources_do_not_ship(self) -> None:
        for player_key in FIXTURE_KEYS:
            entry = self.by_key[player_key]
            derivative = runtime_derivative_path(self.project_root, entry)
            self.assertIn("ContentSource", derivative.parts)
            self.assertNotIn("Content", derivative.parts)
            expected_asset = asset_path(entry)
            self.assertEqual(
                self.provenance_by_key[player_key]["runtimeAssetPath"],
                f"{expected_asset}.{entry['assetName']}",
            )
            self.assertTrue(expected_asset.startswith("/Game/UI/Portraits/"))


if __name__ == "__main__":
    unittest.main(verbosity=2)
