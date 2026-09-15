"""Hash-bound, whole-selection visual preflight for production family art.

Numerical checks never grant visual acceptance. A reviewer must inspect both
source sides and actual-size compositions before recording this offline receipt.
"""
import hashlib
import json

IMPLEMENTATION_HASH_PROFILE = 'SourceTextLF_SHA256_v1'

IMPLEMENTATION_FILES = (
    'Scripts/GenerateSharedPortraitRuntimeDerivatives.py',
    'Scripts/SharedPortraitImportCatalog.py',
    'Scripts/PlayerPortraitForeground.py',
    'Scripts/PlayerPortraitForegroundV2.py',
    'Scripts/PlayerPortraitComposition.py',
    'Scripts/PlayerPortraitPreflight.py',
)
REGIONS = ('hair', 'temple', 'ear', 'jaw', 'neck', 'collar', 'shoulder', 'shirtSleeveEdge')


def canonical_code_sha256(data):
    """Code bytes only: ignore CRLF/CR representation; every other byte matters."""
    return hashlib.sha256(data.replace(b'\r\n', b'\n').replace(b'\r', b'\n')).hexdigest().upper()


def implementation_file_hash(path):
    return canonical_code_sha256(path.read_bytes())


def implementation_hashes(root):
    return {name: implementation_file_hash(root/name) for name in IMPLEMENTATION_FILES}


def role_visual_status(previous, output_hash):
    return (previous.get('visualStatus', 'PENDING USER PIE')
            if previous.get('runtimeDerivativeSha256') == output_hash else 'PENDING USER PIE')


def validate_composition_metrics(role, evidence):
    metrics = evidence['after']
    head, eye, clearance, shoulder, center, residual = (
        ((34, 40), (20.5, 24.5), 2, 59.5, 48, 1.8) if role == 'Hand'
        else ((56, 66), (35, 43), 4, 106, 65, 2.6))
    if not (head[0] <= metrics['headHeight'] <= head[1]
            and eye[0] <= metrics['eyeY'] <= eye[1]
            and metrics['hairTopY'] >= clearance
            and metrics['shoulderY'] <= shoulder
            and abs(metrics['centerX']-center) <= residual):
        raise RuntimeError('Composition outside family bounds; inspect before publishing')


def preflight_binding(record, roles):
    return {'playerKey': record['playerKey'], 'masterSha256': record['masterSha256'],
            'cropMetadataSha256': record['cropMetadataSha256'],
            'roles': {role: record['roles'][role]['runtimeDerivativeSha256'] for role in roles}}


def validate_preflight(path, root, records, roles):
    if not path:
        raise RuntimeError('Reviewed whole-selection preflight receipt required before production writes')
    document = json.loads(path.read_text(encoding='utf-8'))
    expected = [preflight_binding(record, roles) for record in records]
    if (document.get('schemaVersion') != 1 or document.get('status') != 'PASS'
            or document.get('implementationHashProfile') != IMPLEMENTATION_HASH_PROFILE
            or document.get('implementationSha256') != implementation_hashes(root)
            or document.get('bindings') != expected):
        raise RuntimeError('Preflight selection/source/recipe/output/code mismatch; review candidates again')
    inspections = document.get('inspections', {})
    if set(inspections) != {r['playerKey'] for r in records}:
        raise RuntimeError('Preflight must inspect the entire selection before publishing')
    for inspection in inspections.values():
        for side in ('left', 'right'):
            if set(inspection.get(side, {})) != set(REGIONS) or set(inspection[side].values()) != {'PASS'}:
                raise RuntimeError('Both source sides require explicit foreground integrity review')
        for role in roles:
            if role in ('Hand', 'Shared') and inspection.get(role) != 'PASS':
                raise RuntimeError('Actual-size family composition review is incomplete')
    return document
