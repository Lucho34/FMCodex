"""Stage 8.3B global rules; no aesthetic pseudo-tests or gameplay matrix."""
import copy,json,unittest
from pathlib import Path
from PIL import Image
from SharedPortraitImportCatalog import load_catalog,is_canonical,pitch_composition,resolved_crop,master_path,validate_generated_source,runtime_derivative_path,ACTIVE_PITCH_PROFILE
from GenerateSharedPortraitRuntimeDerivatives import encode_runtime_derivative
from ImportCanonicalPlayerContent import validate_config
ROOT=Path(__file__).resolve().parent.parent

class PlayerCardFamilyContractTest(unittest.TestCase):
    def test_all_eleven_shared_outputs_use_one_reproducible_recipe(self):
        players=[p for p in load_catalog(ROOT) if is_canonical(p)]
        self.assertEqual(len(players),11)
        for p in players:
            e=dict(p,runtimeRole='Shared')
            with self.subTest(key=p['playerKey']):
                self.assertEqual(pitch_composition(e),ACTIVE_PITCH_PROFILE)
                record=validate_generated_source(ROOT,e)
                self.assertEqual(record['visualStatus'],'USER PIE ACCEPTED')
                self.assertEqual(record['dimensions'],[512,768])
                self.assertEqual(encode_runtime_derivative(master_path(ROOT,e),(512,768),resolved_crop(e),ACTIVE_PITCH_PROFILE),runtime_derivative_path(ROOT,e).read_bytes())
    def test_future_canonical_defaults_and_old_profile_rejection(self):
        e=copy.deepcopy(next(p for p in load_catalog(ROOT) if is_canonical(p)))
        e.pop('pitchCompositionProfile');e.pop('cropOverrides',None);e['runtimeRole']='Shared'
        self.assertEqual(pitch_composition(e),'QuietPitchBust_v2')
        self.assertEqual(resolved_crop(e),[0,.055,1,.55])
        e['pitchCompositionProfile']='QuietPitchBust_v1'
        with self.assertRaisesRegex(RuntimeError,'must use global'):pitch_composition(e)
    def test_explicit_defaults_unique_configurable_and_validated(self):
        config=json.loads((ROOT/'ContentSource/PlayerContent/CanonicalPlayerImportConfig.json').read_text(encoding='utf-8'))
        runtime=json.loads((ROOT/'Content/Data/CanonicalPlayerContent.json').read_text(encoding='utf-8'))
        self.assertEqual(config['schemaVersion'],3);self.assertEqual(runtime['schemaVersion'],3)
        errors=[];validate_config(config,errors);self.assertEqual(errors,[])
        by_key={p['playerKey']:p for p in runtime['players']}
        for team in ('Arsenal','Manchester City'):
            members=[p for p in config['players'] if p['team']==team]
            nums=[p['presentation']['defaultShirtNumber'] for p in members]
            self.assertEqual(len(set(nums)),20);self.assertTrue(all(1<=n<=99 for n in nums))
            for p in members:self.assertEqual(by_key[p['playerKey']]['presentation']['defaultShirtNumber'],p['presentation']['defaultShirtNumber'])
        for key,n in {'Prototype.Arsenal.DavidRaya':1,'Prototype.Arsenal.BukayoSaka':7,'Prototype.ManchesterCity.Rodri':16,'Prototype.ManchesterCity.ErlingHaaland':9}.items():
            self.assertEqual(by_key[key]['presentation']['defaultShirtNumber'],n)
        for value in (100,-1,True,1.5):
            bad=copy.deepcopy(config);bad['players'][0]['presentation']['defaultShirtNumber']=value;errors=[];validate_config(bad,errors);self.assertTrue(errors)
        bad=copy.deepcopy(config);bad['players'][1]['presentation']['defaultShirtNumber']=bad['players'][0]['presentation']['defaultShirtNumber'];errors=[];validate_config(bad,errors);self.assertTrue(any('duplicate defaultShirtNumber' in x for x in errors))
    def test_acceptance_survives_unchanged_output_but_new_or_changed_output_is_pending(self):
        # Exercise the real status decision in memory; never generate or write art.
        from unittest.mock import patch
        import contextlib, io
        import GenerateSharedPortraitRuntimeDerivatives as generator
        entry=next(p for p in load_catalog(ROOT) if is_canonical(p))
        path=ROOT/'ContentSource/UI/PlayerPortraitRuntime/PlayerArtProvenance.json'
        record=next(r for r in json.loads(path.read_text(encoding='utf-8'))['entries'] if r['playerKey']==entry['playerKey'])
        self.assertEqual(record['roles']['Shared']['visualStatus'],'USER PIE ACCEPTED')
        previous=copy.deepcopy(record);previous['roles']={'Shared':previous['roles']['Shared']}
        data=runtime_derivative_path(ROOT,dict(entry,runtimeRole='Shared')).read_bytes()
        read_text=Path.read_text
        for case,prior,encoded,expected in (
            ('new',[],data,'PENDING USER PIE'),
            ('unchanged',[previous],data,'USER PIE ACCEPTED'),
            ('changed',[previous],data+b'changed-output-hash-fixture','PENDING USER PIE')):
            with self.subTest(case=case):
                def read_fixture(p,*args,**kwargs):
                    return json.dumps({'entries':prior}) if p==path else read_text(p,*args,**kwargs)
                with patch.object(Path,'read_text',read_fixture), \
                     patch.object(generator,'selected_runtime_roles',return_value=('Shared',)), \
                     patch.object(generator,'encode_runtime_derivative',return_value=encoded), \
                     patch.object(generator,'write_if_changed') as writes, \
                     contextlib.redirect_stdout(io.StringIO()):
                    result=generator.generate_canonical_selected(ROOT,[entry])
                self.assertEqual(len(result),1)
                self.assertEqual(result[0]['roles']['Shared']['visualStatus'],expected)
                self.assertEqual(result[0]['roleStatus']['Shared'],expected)
                self.assertEqual(writes.call_count,2)

    def test_bio_is_authored_data_without_synthesized_values(self):
        config=json.loads((ROOT/'ContentSource/PlayerContent/CanonicalPlayerImportConfig.json').read_text(encoding='utf-8'))
        runtime=json.loads((ROOT/'Content/Data/CanonicalPlayerContent.json').read_text(encoding='utf-8'))
        by_key={p['playerKey']:p for p in config['players']}
        for f in ('birthDate','heightCm','weightKg','nationality'):
            self.assertEqual(sum(bool(p['presentation'][f]) for p in runtime['players']),16)
            for p in runtime['players']:self.assertEqual(p['presentation'][f] or None,by_key[p['playerKey']].get('presentation',{}).get(f) or None)

if __name__=='__main__':unittest.main(verbosity=2)
