"""Focused production v3 contracts. Canonical Masters are source-bound fixtures.

All fixtures are canonical source assets.
Missing or altered fixtures fail explicitly; these tests guard the promoted extraction behavior.
Source-bound regions prove objective integrity, not USER PIE acceptance.
"""
import ast
import hashlib
import inspect
import json
from pathlib import Path
import unittest
import numpy as np
from PIL import Image
import PlayerPortraitForeground as v1
import PlayerPortraitForegroundV2 as v2
import PlayerPortraitForegroundV3 as v3
import PlayerPortraitComposition as composition
from TestPlayerPortraitForegroundV2 import FIXTURES, coverage

ROOT = Path(__file__).resolve().parent.parent
COLLARS = [
    {'key': 'Prototype.ManchesterCity.JohnStones',
     'sourcePixelSha256': 'F7538162F810E7FB0A9D2CB7D59F0E358ED66D5CFFDB6B69C903A94DA2B5EC94',
     'recoverPoints': [(330, 300), (325, 315), (315, 330)],
     'backgroundPoints': [(348, 280), (370, 295)]},
    {'key': 'Prototype.ManchesterCity.MatheusNunes',
     'sourcePixelSha256': '0EBE925C908773740A4E436F35F944948A7DE76D0213D4575D3B6F12048C1C35',
     'recoverPoints': [(180, 305), (190, 318)],
     'backgroundPoints': [(165, 280), (152, 300)]},
    {'key': 'Prototype.ManchesterCity.AntoineSemenyo',
     'sourcePixelSha256': 'CC77EC9F902FF0B31CB3DD75F9E213C2CDEF8E08B7BF510283A3F6EBE059FEB4',
     'recoverPoints': [(330, 300), (230, 360)],
     'backgroundPoints': [(350, 270), (375, 290)]},
]
FROZEN_IMPLEMENTATION = {
    'GenerateSharedPortraitRuntimeDerivatives.py': 'ABC21D12E979EF86390BBC9AFFC445D7CA535C57EF9383EDD04A2C35A0EBF65A',
    'SharedPortraitImportCatalog.py': '5C9894FBAAE91613055097FC13538F723E0193CB18A9E2FCAC0AC6C72620D001',
    'PlayerPortraitForeground.py': '7FC09B50396CF095ADCEE174FBE2AAA57902AD7AD90969C0309AB043B2DA6949',
    'PlayerPortraitForegroundV2.py': 'B424E76037555DC2F989CFC5A050BD028EDFF10B49AF2DE2A8B6AD361D9506C3',
    'PlayerPortraitComposition.py': 'B61E5AC362C0D715022BF5FF9666F477828DF6E785E690E8310F11BF471249AA',
    'PlayerPortraitPreflight.py': 'ADCF8378D33498F9FE692343A71105DEF29B34DDDF61C999B460477B098E1008',
}


class PlayerPortraitForegroundV3Test(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.samples = {}
        for fixture in COLLARS + FIXTURES:
            folder = 'ArtSource/UI/PlayerMaster'
            path = ROOT/folder/fixture['key']/'Master.png'
            if not path.is_file():
                raise RuntimeError('Candidate test requires original local fixture: '+str(path))
            source = Image.open(path)
            if v1.source_pixel_hash(source) != fixture['sourcePixelSha256']:
                raise AssertionError('Inspected source fixture changed: '+fixture['key'])
            prior, baseline = v2.extract_source_bases(source)
            candidate = v3._recover_from_v2(source, baseline)
            cls.samples[fixture['key']] = (source, prior, baseline, candidate)

    def test_explicit_production_identity(self):
        self.assertEqual(v3.FOREGROUND_PROFILE, 'SourceSpaceForeground_v3')
        self.assertEqual(v2.FOREGROUND_PROFILE, 'SourceSpaceForeground_v2')
        source = self.samples[COLLARS[0]['key']][0]
        for profile in ['SourceSpaceForeground_v3_candidate', 'SourceSpaceForeground_v2', 'unknown']:
            with self.assertRaises(ValueError):
                v3.extract_source_foreground(source, profile)

    def test_no_purpose_crop_or_identity_inputs(self):
        for function in (v3.extract_source_foreground, v3.extract_source_bases):
            self.assertEqual(list(inspect.signature(function).parameters), ['source', 'profile'])
        code = Path(v3.__file__).read_text(encoding='utf-8')
        names = {node.id.lower() for node in ast.walk(ast.parse(code)) if isinstance(node, ast.Name)}
        self.assertFalse(names & {'playerkey', 'team', 'kit', 'crop', 'purpose'})
        for fixture in COLLARS + FIXTURES:
            self.assertNotIn(fixture['key'], code)
        for sample in self.samples.values():
            self.assertFalse(sample[3].evidence['framingInputsUsed'])

    def test_deterministic_public_extraction(self):
        for key in [COLLARS[0]['key'], 'Prototype.Arsenal.EberechiEze']:
            source, prior, _, candidate = self.samples[key]
            again_prior, again = v3.extract_source_bases(source)
            self.assertEqual(again_prior.alpha_sha256, prior.alpha_sha256)
            self.assertEqual(again.alpha_sha256, candidate.alpha_sha256)
            self.assertEqual(again.evidence, candidate.evidence)

    def test_invalid_source_rejected(self):
        for source in [Image.new('RGB', (512, 768)), Image.new('RGBA', (1024, 1536))]:
            with self.assertRaises(ValueError):
                v3.extract_source_foreground(source)
        with self.assertRaises(RuntimeError):
            v3.extract_source_foreground(Image.new('RGB', (1024, 1536), '#18232d'))

    def test_source_and_baseline_binding(self):
        source, prior, baseline, candidate = self.samples[COLLARS[0]['key']]
        self.assertEqual(candidate.source_pixel_sha256, v1.source_pixel_hash(source))
        self.assertEqual(candidate.evidence['baselineAlphaSha256'], baseline.alpha_sha256)
        self.assertEqual(candidate.evidence['priorAlphaSha256'], prior.alpha_sha256)
        with self.assertRaises(ValueError):
            v3._recover_from_v2(source, prior)
        wrong_source = self.samples[COLLARS[1]['key']][0]
        with self.assertRaises(ValueError):
            v3._recover_from_v2(wrong_source, baseline)

    def test_all_three_source_supported_collars_recovered(self):
        for fixture in COLLARS:
            _, _, baseline, candidate = self.samples[fixture['key']]
            for point in fixture['recoverPoints']:
                with self.subTest(key=fixture['key'], point=point):
                    self.assertLess(coverage(baseline.alpha, point), 30)
                    self.assertGreater(coverage(candidate.alpha, point), 240)

    def test_collar_background_stays_excluded(self):
        for fixture in COLLARS:
            candidate = self.samples[fixture['key']][3]
            for point in fixture['backgroundPoints']:
                with self.subTest(key=fixture['key'], point=point):
                    self.assertLess(coverage(candidate.alpha, point), 30)

    def test_v2_historical_edges_and_background(self):
        for fixture in FIXTURES:
            candidate = self.samples[fixture['key']][3]
            for point in fixture['recoverPoints']:
                self.assertGreater(coverage(candidate.alpha, point), 180, (fixture['key'], point))
            for point in fixture['backgroundPoints']:
                self.assertLess(coverage(candidate.alpha, point), 30, (fixture['key'], point))

    def test_long_hair_braids_and_clean_control_exact(self):
        for fixture in FIXTURES:
            if fixture['protectionSample']:
                _, _, baseline, candidate = self.samples[fixture['key']]
                self.assertEqual(baseline.alpha_sha256, candidate.alpha_sha256, fixture['key'])

    def test_strictly_bounded_additive_recovery(self):
        for _, _, baseline, candidate in self.samples.values():
            old, new = np.asarray(baseline.alpha), np.asarray(candidate.alpha)
            self.assertTrue(np.all(new >= old))
            x, y, w, h = candidate.evidence['faceBox']
            yy, xx = np.indices(old.shape)
            protected = ((xx < x-.2*w-3) | (xx > x+1.2*w+3)
                         | (yy < y+.95*h-3) | (yy > y+1.95*h+3))
            self.assertTrue(np.array_equal(new[protected], old[protected]))
            self.assertEqual(candidate.evidence['newDilationIterations'], 0)
            self.assertEqual(candidate.evidence['newClosingIterations'], 0)
            self.assertEqual(candidate.evidence['recoveryEdgeSupportPixels'], 2)

    def test_bridge_requires_two_support_classes_and_substantial_region(self):
        neck = np.zeros((768, 512), bool)
        torso = np.zeros_like(neck)
        neck[240:300, 210:300] = True
        torso[310:380, 190:320] = True
        allowed = np.zeros_like(neck)
        allowed[300:310, 210:300] = True
        seeds, _ = v3._bridge_seeds(neck, torso, allowed, neck | torso, 180)
        self.assertGreater(seeds.sum(), 64)
        missing, _ = v3._bridge_seeds(np.zeros_like(neck), torso, allowed, neck | torso, 180)
        self.assertFalse(missing.any())
        tiny = np.zeros_like(neck)
        tiny[303:306, 250:253] = True
        rejected, _ = v3._bridge_seeds(neck, torso, tiny, neck | torso, 180)
        self.assertFalse(rejected.any())

    def test_background_rule_has_no_brightness_or_kit_class(self):
        base = np.zeros((768, 512), bool)
        for color in [(240, 240, 240), (15, 15, 15), (180, 25, 40), (80, 170, 240)]:
            rgb = np.full((768, 512, 3), color, np.uint8)
            self.assertTrue(v3._exterior_background(rgb, base).all())

    def test_background_appearance_veto_is_color_symmetric(self):
        seeds = np.zeros((768, 512), bool)
        seeds[310:320, 250:270] = True
        background = np.zeros_like(seeds)
        background[280:300, 100:120] = True
        for color in [(240, 240, 240), (15, 15, 15), (180, 25, 40), (80, 170, 240)]:
            rgb = np.full((768, 512, 3), color, np.uint8)
            self.assertFalse(v3._veto_background_appearance(rgb, seeds, background).any())
            rgb[seeds] = [255-value for value in color]
            self.assertTrue(v3._veto_background_appearance(rgb, seeds, background)[seeds].all())

    def test_impact_scan_hair_background_regressions(self):
        # These are required existing-28 impact cases, not extra primary samples.
        for key in ['Prototype.Arsenal.RiccardoCalafiori', 'Prototype.ManchesterCity.ErlingHaaland']:
            source = Image.open(ROOT/'ArtSource/UI/PlayerMaster'/key/'Master.png')
            _, baseline = v2.extract_source_bases(source)
            candidate = v3._recover_from_v2(source, baseline)
            self.assertEqual(candidate.alpha_sha256, baseline.alpha_sha256, key)

    def test_accepted_candidate_alpha_is_unchanged(self):
        expected = {'Prototype.Arsenal.BukayoSaka': 'B620207D3B0108211E59BF720ED8744D305D8D6EAAC9530C7AAC344F87690874', 'Prototype.Arsenal.DavidRaya': 'A108CC65879BD2332832F71E4F0E7B84B113E718DE21CB072303D81F222227AD', 'Prototype.Arsenal.DeclanRice': '100E8CA07B7BE5062288021DC3AD98AF03A404E809AA6CB71155B7E8205E734D', 'Prototype.Arsenal.EberechiEze': 'BA741C48EE5D0E1E3188D8B776F59DFB969E58F8AAA81E7F9D3BF536408E449A', 'Prototype.Arsenal.GabrielMagalhaes': 'EB9B89B7A5E4BB4E3446D7A28CF2BD1E8CED94AF7CE782346B8CA0489D5FA28B', 'Prototype.Arsenal.GabrielMartinelli': '755BBA2151885089207F5F8999BA64A0BC95276A494796AF9AF333FE34D85E32', 'Prototype.Arsenal.JurrienTimber': '816928F9CFFFA4493B0538BB3264D4319AE15121C10134880D23909261C1606A', 'Prototype.Arsenal.KaiHavertz': '0353104EB19F5A470ADAC15E15C306257A61C4D761E0A10C2B73EB72C16B2E5A', 'Prototype.Arsenal.MartinOdegaard': 'A917D42C5448BC28402F3A3EDFF09BC74EE30ACF62232D1616CED22006D28D92', 'Prototype.Arsenal.MartinZubimendi': '6CE5E9137AB1930066E381CF658978AD5A32531917F31B570453532CAACE0D2D', 'Prototype.Arsenal.MylesLewisSkelly': 'CA2A0B60353F11D8B99877E83E3C08EEA7C7F26FDE6B92A2371CC7147BA3DEA3', 'Prototype.Arsenal.RiccardoCalafiori': 'CAA5950CAF130F63E238DFBBE29DFD815E2F70C0B115E9E729D48A9589B4AA27', 'Prototype.Arsenal.ViktorGyokeres': '7C979CD1DBA8F35DD89A6B8F540D714A3A6BB3285AD61F7543C4C8BABEDF7491', 'Prototype.Arsenal.WilliamSaliba': 'C5D511E1B6D0084ABA78679EF976CD29F55EEC1B53905E7DF333846F26349AE9', 'Prototype.ManchesterCity.BernardoSilva': '03576B1F756658B2760E591EFD69CE8C821CCBB6AD24A4E1414903F85E580C30', 'Prototype.ManchesterCity.ErlingHaaland': 'BB32B9924F2A2D2B89641436FD409A763D85771D54B8CF4F75648CE167C7BD07', 'Prototype.ManchesterCity.GianluigiDonnarumma': 'E60C3D3647AB8A033B491415A6D2462B6BECE95F0EADAFE03B74DCD0C731BB15', 'Prototype.ManchesterCity.JeremyDoku': '3CF52159D090A992FB61D8C6BA0F28A60280306E49CDC1E21DE8A8B6CD040422', 'Prototype.ManchesterCity.JoskoGvardiol': 'A64B3DC73AEEFB11764C4472D14B6BB88791F7237BEB198DC3F57CDB66FC63D2', 'Prototype.ManchesterCity.MarcGuehi': 'BD77673A23EB171FED4C1EF081397D5BEB30E98A69DE35B425BF188E8EAC79C7', 'Prototype.ManchesterCity.NathanAke': 'F4D439A923C9E483E640CC1F4593EBE68F744B19ED8E73D18952AC0637B2E6C7', 'Prototype.ManchesterCity.OmarMarmoush': 'AF80E4A98677A778AE8461CC80259F24A069E1DA9A8910A3BB36EE78B7C74B1C', 'Prototype.ManchesterCity.PhilFoden': 'C8449958C3304EA97D0F4EC729F0469AFB5E2C5B6B067D28890266D6E5774026', 'Prototype.ManchesterCity.RayanAitNouri': '1919F7410028D1528B5CBC81BFF8DD6B9336A6C433491900DD3F7563B551AB9D', 'Prototype.ManchesterCity.RayanCherki': '16720D2D2F0918AC635EA8509D0A99A649AE8E4A472A3C91A3A96961F297F9DF', 'Prototype.ManchesterCity.Rodri': '01EA32C64BC42984AA849DEF990E6E5079DD8CAD0C922A59CE6A31D8698289E5', 'Prototype.ManchesterCity.RubenDias': '681A83C579B4EF1A9D8D032FCB5BCF70422850C53127A0251FC0EE0862242080', 'Prototype.ManchesterCity.TijjaniReijnders': '49A7701AAE963FF688EE09B761E1E20729C361540F76844C85555128A8B966DE', 'Prototype.Arsenal.BenWhite': 'B86F44C13D0AE43190BB14BDF0A4C89C96CE39F7276899F291B6AF28A1B951FD', 'Prototype.Arsenal.PieroHincapie': 'D693C28025304686C8CFBC2796F0F2CADAD8FF0CC0476925D320DF6A81B5053D', 'Prototype.Arsenal.MikelMerino': '9DB1DEB1C80C1FE305299D4D0889EA67E4CDC406BF9C57D834A7F02D3BEF818D', 'Prototype.Arsenal.ChristianNorgaard': '61576CCE505F60237EDA56FED20A49344018A5E3B907DC0D9A2898EBA0B97AE7', 'Prototype.Arsenal.LeandroTrossard': 'CBD44DADABE58257E7599D5785F1E65A38A4E2EB365BDB03365D12311A74D77C', 'Prototype.Arsenal.NoniMadueke': '0518BB36FB70F20D7DE3471FCF55444C7EE9D3D8DEF776DCC1DE8FF99F581614', 'Prototype.ManchesterCity.JohnStones': 'DC40AD006DA5307DCAB4C62197DD27CB84ACD123B25336863855F88217D950DA', 'Prototype.ManchesterCity.NicoGonzalez': '0AE666772CA6343AE159E11BDD9E37AE04E61C0D6345B799D6F964523B66A7A4', 'Prototype.ManchesterCity.MatheusNunes': '30760CB08689C5901519C5C273AFE818766B5BDD032D13ACA5A95F1D76BCF6E3', 'Prototype.ManchesterCity.MateoKovacic': '023B3E545DAE33F4C1EDA9BDA28C1738DE2C18A056025166724A8C1B3C29D097', 'Prototype.ManchesterCity.AntoineSemenyo': 'CDA04626BCF4B7F79A7DB50474544C202CD8309CD7ADA52C861972A700F0BAA3', 'Prototype.ManchesterCity.Savinho': '4F00297256D6456460A72B5C077E673B8465D99B0867B55C7B241E7A1ED0583A'}
        for key, (_, _, _, foreground) in self.samples.items():
            self.assertEqual(foreground.alpha_sha256, expected[key], key)
        for name in ('PlayerPortraitForeground.py','PlayerPortraitForegroundV2.py'):
            data=(ROOT/'Scripts'/name).read_bytes().replace(b'\r\n',b'\n').replace(b'\r',b'\n')
            self.assertEqual(hashlib.sha256(data).hexdigest().upper(), FROZEN_IMPLEMENTATION[name])

    def test_production_compositor_still_rejects_candidate(self):
        source, prior, _, candidate = self.samples[COLLARS[0]['key']]
        from dataclasses import replace
        candidate = replace(candidate, profile='SourceSpaceForeground_v3_candidate')
        anchors = composition.measure_face_anchors(source, prior)
        with self.assertRaises(ValueError):
            composition.compose_portrait(source, prior, anchors, [0, .055, 1, .5],
                                         composition.HAND_PROFILE, foreground=candidate)


if __name__ == '__main__':
    unittest.main(verbosity=2)
