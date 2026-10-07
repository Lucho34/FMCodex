#!/usr/bin/env python3
"""One focused source-validation test with malformed-input subcases."""
import copy
import json
import sys
import unittest

sys.dont_write_bytecode = True
import ImportGuidedMatchContent as content


class GuidedContentSourceTest(unittest.TestCase):
    def test_source_validation(self):
        data = content.generate()
        self.assertEqual(json.dumps(data, ensure_ascii=False, indent=2) + "\n", content.OUTPUT.read_text(encoding="utf-8"))
        self.assertEqual(data, content.generate())
        self.assertEqual({r["StepId"] for r in data["steps"]}, set(content.STEPS))
        cases = {
            "duplicate": lambda d: d["steps"].append(copy.deepcopy(d["steps"][0])),
            "surface": lambda d: d["steps"][0].update(SurfaceType="ModalTypo"),
            "target": lambda d: d["steps"][0].update(FocusTargetId="Screen.X100"),
            "placeholder": lambda d: d["steps"][0].update(BodyCN="{Unknown.Field}"),
            "missing step": lambda d: d["steps"].pop(),
            "overlap": lambda d: d["emphasis"].append(copy.deepcopy(d["emphasis"][0])),
            "missing span": lambda d: d["emphasis"][0].update(MatchText="找不到的短语"),
            "negative delay": lambda d: d["timings"][0].update(Value=-1),
            "infinite delay": lambda d: d["timings"][0].update(Value=float("inf")),
            "missing lesson": lambda d: d["steps"][0].update(LessonId="Unknown"),
        }
        for name, mutate in cases.items():
            with self.subTest(name=name):
                bad = copy.deepcopy(data)
                mutate(bad)
                with self.assertRaises(content.ValidationFailure):
                    content.validate(bad)


if __name__ == "__main__":
    unittest.main()
