"""Objective contracts for the isolated source-space foreground prototype."""
import unittest
from pathlib import Path
from unittest.mock import patch
import numpy as np
from PIL import Image

import PlayerPortraitForeground as foreground
from PlayerPortraitForeground import (FOREGROUND_PROFILE,ForegroundBasis,extract_source_foreground,
    compose_with_foreground,png_bytes,_keep_anchored_components,_bounded_integrity)

ROOT=Path(__file__).resolve().parent.parent

class PlayerPortraitForegroundTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Image.open(ROOT/'ArtSource/UI/PlayerMaster/Prototype.ManchesterCity.TijjaniReijnders/Master.png') as image:
            cls.source=image.copy()
        cls.basis=extract_source_foreground(cls.source)

    def test_purpose_and_crop_only_change_framing(self):
        initial=self.basis.alpha_sha256
        with patch.object(foreground,'extract_source_foreground',side_effect=AssertionError('Framing must not extract')):
            hand=compose_with_foreground(self.source,(192,128),[.02,.065,.96,.46],'BalancedBust_v2',self.basis)
            alternate=compose_with_foreground(self.source,(192,128),[0,.055,1,.5],'BalancedBust_v2',self.basis)
            pitch=compose_with_foreground(self.source,(512,768),[0,.055,1,.55],'QuietPitchBust_v2',self.basis)
        self.assertNotEqual(png_bytes(hand),png_bytes(alternate))
        self.assertEqual((hand.size,pitch.size),((192,128),(512,768)))
        self.assertEqual(self.basis.alpha_sha256,initial)
        self.assertFalse(self.basis.evidence['framingInputsUsed'])

    def test_source_basis_is_deterministic_and_versioned(self):
        again=extract_source_foreground(self.source)
        self.assertEqual(again.alpha_sha256,self.basis.alpha_sha256)
        self.assertEqual(again.evidence,self.basis.evidence)
        self.assertEqual(again.profile,FOREGROUND_PROFILE)
        self.assertEqual(len(again.evidence['faceLocatorSha256']),64)
        with self.assertRaises(ValueError):extract_source_foreground(self.source,'unknown')

    def test_mismatched_source_and_full_are_rejected(self):
        other=self.source.copy();other.putpixel((0,0),tuple(255-v for v in other.getpixel((0,0))))
        with self.assertRaises(ValueError):
            compose_with_foreground(other,(192,128),[0,.055,1,.5],'BalancedBust_v2',self.basis)
        with self.assertRaises(ValueError):
            compose_with_foreground(self.source,(768,1152),[0,0,1,1],'CropOnly_v1',self.basis)

    def test_multiple_anchored_components_survive_noise_does_not(self):
        binary=np.zeros((768,512),np.uint8)
        binary[100:140,220:270]=255;binary[300:370,180:330]=255
        binary[20:30,20:30]=255
        face=np.zeros_like(binary,dtype=bool);face[110:120,230:240]=True
        torso=np.zeros_like(binary,dtype=bool);torso[320:340,210:230]=True
        kept,records=_keep_anchored_components(binary,[face,torso])
        self.assertEqual(len(records),2)
        self.assertTrue(kept[115,235] and kept[330,220])
        self.assertEqual(kept[25,25],0)

    def test_integrity_pass_fills_small_holes_only(self):
        binary=np.zeros((768,512),np.uint8);binary[100:300,180:330]=255
        binary[140:144,210:214]=0;binary[190:215,250:275]=0
        result=_bounded_integrity(binary)
        self.assertEqual(result[141,211],255)
        self.assertEqual(result[202,262],0)
        self.assertEqual(result[95,250],0)
        self.assertEqual(result[150,175],0)

if __name__=='__main__':unittest.main()
