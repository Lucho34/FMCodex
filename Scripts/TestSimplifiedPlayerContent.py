"""Stage 8.20 foundation schema and source reconciliation (standard library only)."""
import copy
import hashlib
import json
import unittest
from pathlib import Path

import ImportCanonicalPlayerContent as importer

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "ContentSource/PlayerContent"


class SimplifiedContentTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.workbook = SOURCE / "FMCodex_Canonical_Player_Content.xlsx"
        cls.rows = importer.read_sheet_rows(cls.workbook, "球员配置")
        cls.config = json.loads((SOURCE / "CanonicalPlayerImportConfig.json").read_text(encoding="utf-8-sig"))
        cls.columns = {name: index for index, name in enumerate(importer.SOURCE_HEADERS)}

    def parse(self, rows=None):
        return importer.parse_players(
            self.rows if rows is None else rows, self.config,
            hashlib.sha256(self.workbook.read_bytes()).hexdigest(), self.workbook.name)[0]

    def invalid_cell(self, column, value, row=2):
        rows = copy.deepcopy(self.rows)
        rows[row][self.columns[column]] = value
        with self.assertRaises(importer.ValidationFailure):
            self.parse(rows)

    def test_exact_generated_json_and_counts(self):
        runtime = self.parse()
        saved = (ROOT / "Content/Data/CanonicalPlayerContent.json").read_text(encoding="utf-8-sig")
        self.assertEqual(saved, json.dumps(runtime, ensure_ascii=False, indent=2) + "\n")
        self.assertEqual(runtime["schemaVersion"], 4)
        self.assertEqual(len(runtime["players"]), 40)
        self.assertEqual(sum(p["position"] == "GK" for p in runtime["players"]), 2)

    def test_draft_values_and_passive_assignments_exact(self):
        draft = SOURCE / "FMCodex_Rules_Simplification_Draft.xlsx"
        attributes = {r[6]: r for r in importer.read_sheet_rows(draft, "AttributeMigrationDraft")[1:]}
        traits = {r[6]: r for r in importer.read_sheet_rows(draft, "TraitMigrationDraft")[1:]}
        for player in self.parse()["players"]:
            row = attributes[player["playerKey"]]
            self.assertEqual((player["displaySerial"], player["team"], player["chineseName"], player["position"], player["rosterSlot"], player["englishName"]), tuple(row[:6]))
            if player["position"] == "GK":
                self.assertIsNone(player["staminaTier"])
                self.assertIsNone(player["outfieldAttributes"])
                self.assertFalse(player["rankedTraits"] or player["binaryTraits"])
            else:
                self.assertEqual(set(player["outfieldAttributes"]), set(importer.OUTFIELD_ATTRIBUTES))
                for key, index in [("SHO", 14), ("CON", 15), ("DEF", 16)]:
                    self.assertEqual(player["outfieldAttributes"][key], row[index])
                self.assertEqual(player["staminaTier"], row[17])
            assigned = traits[player["playerKey"]][7:31]
            expected_ranked = [{"traitId": key, "rank": value} for key, value in zip(importer.TRAIT_HEADERS, assigned) if value and key != importer.BINARY_TRAIT]
            expected_binary = [key for key, value in zip(importer.TRAIT_HEADERS, assigned) if value and key == importer.BINARY_TRAIT]
            self.assertEqual(player["rankedTraits"], expected_ranked)
            self.assertEqual(player["binaryTraits"], expected_binary)

    def test_base_range_and_missing_value(self):
        for key in importer.OUTFIELD_ATTRIBUTES:
            for value in [None, 0, 7, 2.5, "bad"]:
                with self.subTest(key=key, value=value): self.invalid_cell(key, value)

    def test_tier_semantics(self):
        for value in [None, 5, 3, 1, "C", "s"]: self.invalid_cell("StaminaTier", value)

    def test_passive_rank_and_binary_validation(self):
        for value in [1, "有", "C"]: self.invalid_cell("Trait.LongShotCarrier", value)
        for value in [1, "S", "无"]: self.invalid_cell(importer.BINARY_TRAIT, value)

    def test_goalkeeper_exclusion(self):
        self.invalid_cell("SHO", 1, row=1)
        self.invalid_cell("StaminaTier", "S", row=1)
        self.invalid_cell("Trait.LongShotCarrier", "S", row=1)
        self.invalid_cell("HAN", 0, row=1)

    def test_identity_and_removed_tactic(self):
        self.invalid_cell("PlayerId", self.rows[1][2])
        self.invalid_cell("EnglishName", "Unknown")
        self.invalid_cell("Skill1", "PassControl")

    def test_old_or_unknown_header_rejected(self):
        rows = copy.deepcopy(self.rows)
        rows[0].append("LS")
        with self.assertRaises(importer.ValidationFailure): self.parse(rows)
        for column in ["CON", "DEF", "Trait.LongShotCarrier"]:
            rows = copy.deepcopy(self.rows)
            rows[0][self.columns[column]] = "UnknownLegacyField"
            with self.assertRaises(importer.ValidationFailure): self.parse(rows)


if __name__ == "__main__":
    unittest.main()
