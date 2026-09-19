"""Current roster, purpose lifecycle and formal v1.3 production contracts."""
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parent.parent
FINAL12 = {
    'Prototype.Arsenal.' + name for name in
    ('BenWhite', 'PieroHincapie', 'MikelMerino', 'ChristianNorgaard', 'LeandroTrossard', 'NoniMadueke')
} | {
    'Prototype.ManchesterCity.' + name for name in
    ('JohnStones', 'NicoGonzalez', 'MatheusNunes', 'MateoKovacic', 'AntoineSemenyo', 'Savinho')
}
CHANGED5 = {'Prototype.ManchesterCity.' + name for name in
            ('BernardoSilva', 'JeremyDoku', 'JoskoGvardiol', 'RayanAitNouri', 'RubenDias')}


def expected_status(key, role):
    """Current roster after explicit final USER PIE; new/changed-art rules stay separate."""
    return 'USER PIE ACCEPTED'


class PlayerArtFinal40Test(unittest.TestCase):
    def test_roster_and_lifecycle_are_exact(self):
        from collections import Counter
        from SharedPortraitImportCatalog import load_catalog, is_canonical
        entries = load_catalog(ROOT)
        roster = json.loads((ROOT/'Content/Data/CanonicalPlayerContent.json').read_text(encoding='utf-8'))['players']
        self.assertEqual(len(entries), 40)
        self.assertTrue(all(is_canonical(e) for e in entries))
        self.assertEqual({e['playerKey'] for e in entries}, {p['playerKey'] for p in roster})
        self.assertEqual(Counter(e['team'] for e in entries), {'Arsenal':20, 'ManchesterCity':20})
        self.assertEqual({e['playerKey'] for e in entries if e.get('migrationStage') == '8.5'}, FINAL12)
        statuses = Counter()
        for entry in entries:
            expected = {role:expected_status(entry['playerKey'], role) for role in ('Hand','Shared','Full')}
            self.assertEqual(entry['roleStatus'], expected)
            statuses.update(expected.values())
        self.assertEqual(statuses, {'USER PIE ACCEPTED':120})
        self.assertNotIn('Test.FuturePlayer', {e['playerKey'] for e in entries})

    def test_candidate_profile_is_never_a_current_binding(self):
        from SharedPortraitImportCatalog import load_catalog, validate_generated_source
        for entry in load_catalog(ROOT):
            self.assertEqual(entry['familyRevision'], '1.3')
            self.assertEqual(entry['foregroundExtractionProfile'], 'SourceSpaceForeground_v3')
            for role in ('Hand','Shared','Full'):
                record = validate_generated_source(ROOT, dict(entry, runtimeRole=role))
                self.assertEqual(record['visualStatus'], expected_status(entry['playerKey'], role))
                self.assertNotIn('_candidate', json.dumps(record))


if __name__ == '__main__':
    unittest.main(verbosity=2)
