"""Production v3 framing; fixed v1 anchors/background with explicit v2 rendering."""
from dataclasses import dataclass
import itertools
import math
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageChops, ImageDraw

from PlayerPortraitForeground import FOREGROUND_PROFILE, source_pixel_hash, digest
from GenerateSharedPortraitRuntimeDerivatives import hand_background, pitch_stadium_background

HAND_PROFILE = 'BalancedBust_v3'
PITCH_PROFILE = 'QuietPitchBust_v3'
ANCHOR_PROFILE = 'FaceEyeCrownAnchors_v1'


@dataclass(frozen=True)
class FaceAnchors:
    eye_x: float
    eye_y: float
    crown_y: float
    hair_top_y: float
    chin_y: float
    shoulder_y: float
    face_width: float
    evidence: dict

    @property
    def head_height(self):
        return self.chin_y - self.crown_y


def _validate_basis(source, basis):
    if basis.profile != FOREGROUND_PROFILE or basis.source_pixel_sha256 != source_pixel_hash(source):
        raise ValueError('Composition requires the matching fixed SourceSpaceForeground_v1 basis')
    if basis.alpha.mode != 'L' or basis.alpha.size != (512, 768):
        raise ValueError('Expected fixed 512x768 foreground alpha')


def measure_face_anchors(source, basis):
    """Read source landmarks; never change alpha or fit the whole body bounding box."""
    _validate_basis(source, basis)
    cv2.setNumThreads(1)
    x, y, w, h = basis.evidence['faceBox']
    pixels = np.asarray(source.resize((512, 768), Image.Resampling.LANCZOS))
    gray = cv2.cvtColor(pixels, cv2.COLOR_RGB2GRAY)
    path = Path(cv2.data.haarcascades) / 'haarcascade_eye_tree_eyeglasses.xml'
    detector = cv2.CascadeClassifier(str(path))
    boxes = [tuple(map(int, b)) for b in detector.detectMultiScale(
        gray[y:y+int(.62*h), x:x+w], scaleFactor=1.03, minNeighbors=4,
        minSize=(14, 14), maxSize=(int(.4*w), int(.35*h)))]
    pairs = []
    for first, second in itertools.combinations(sorted(boxes), 2):
        l = (first[0]+first[2]/2, first[1]+first[3]/2)
        r = (second[0]+second[2]/2, second[1]+second[3]/2)
        if l[0] > r[0]:
            l, r, first, second = r, l, second, first
        if .24*w <= r[0]-l[0] <= .7*w and abs(l[1]-r[1]) < .14*h:
            score = first[2]*first[3]+second[2]*second[3]-8*abs(l[1]-r[1])-2*abs((l[0]+r[0])/2-.5*w)
            pairs.append((score, l, r))
    if pairs:
        _, l, r = max(pairs)
        ex, ey = x+(l[0]+r[0])/2, y+(l[1]+r[1])/2
        method = 'eyePair'
    else:
        ex, ey = x+w/2, y+.38*h
        method = 'faceBoxFallback'
    mask = np.asarray(basis.alpha) > 127
    left, right = max(0, int(x-.15*w)), min(512, int(x+1.15*w))
    top, bottom = max(0, int(y-.55*h)), int(y+.35*h)
    counts = mask[top:bottom, left:right].sum(axis=1)
    occupied = np.flatnonzero(counts)
    if not len(occupied):
        raise RuntimeError('Missing crown evidence; no candidate output')
    crown = next((top+i for i in range(len(counts)-2) if min(counts[i:i+3]) >= .20*w), None)
    if crown is None:
        raise RuntimeError('No stable head crown')
    hair_top = top+int(occupied[0])
    # A disclosed face-box-based jaw estimate, not a claim of anatomical landmarks.
    chin = y+1.04*h
    shoulder = None
    for yy in range(int(chin), min(768, int(chin+1.2*h))):
        row = np.flatnonzero(mask[yy])
        if len(row) and row[-1]-row[0]+1 >= 1.65*w:
            shoulder = yy
            break
    if shoulder is None:
        raise RuntimeError('No shoulder transition evidence; review source before publishing')
    evidence = {'profile': ANCHOR_PROFILE, 'faceBox': [x, y, w, h], 'eyeMethod': method,
                'eyeDetectorSha256': digest(path.read_bytes()), 'crown': crown, 'hairTop': hair_top,
                'chinEstimate': chin, 'shoulderFlare': shoulder,
                'shoulderMeaning': 'alpha width first reaches 1.65 face widths; torso proxy, not neckline',
                'sourcePixelSha256': basis.source_pixel_sha256, 'foregroundAlphaSha256': basis.alpha_sha256}
    return FaceAnchors(ex, ey, crown, hair_top, chin, shoulder, w, evidence)


def current_frame(crop, role):
    """Frozen v2 placement expressed in actual visible-window pixels / analysis pixel."""
    x, y, w, h = crop
    if role == 'Hand':
        scale = 64/(h*768)
        width = round(128*w*1024/(h*1536))/2
        return scale, (96-width)/2-x*512*scale, -y*768*scale
    if role != 'Shared':
        raise ValueError('Full is frozen')
    scale = 112/(h*768)
    return scale, 65-256*scale, -y*768*scale


def frame_metrics(anchors, frame, height):
    scale, tx, ty = frame
    return {'headHeight': anchors.head_height*scale, 'eyeY': anchors.eye_y*scale+ty,
            'centerX': anchors.eye_x*scale+tx, 'crownY': anchors.crown_y*scale+ty,
            'hairTopY': anchors.hair_top_y*scale+ty, 'chinY': anchors.chin_y*scale+ty,
            'shoulderY': anchors.shoulder_y*scale+ty,
            'belowChin': height-(anchors.chin_y*scale+ty),
            'belowShoulderFlare': height-(anchors.shoulder_y*scale+ty)}


def _band(value, low, high, unit=1):
    return (max(low-value, 0, value-high)/unit)**2


def fit_frame(anchors, original, profile):
    if profile == HAND_PROFILE:
        width, height = 96, 64
        hs, es = np.arange(34, 40.001, .125), np.arange(20.5, 24.501, .125)
        head_band, eye_band, shoulder_band, chin_band, crown_band = (36.5, 38.5), (22, 23.5), (51, 58.5), (40, 45), (3, 8)
        hair_min, shoulder_max = 2, 59.5
        center_limit = 1.8
        shoulder_weight = 8
    elif profile == PITCH_PROFILE:
        width, height = 130, 112
        hs, es = np.arange(56, 66.001, .25), np.arange(35, 43.001, .25)
        head_band, eye_band, shoulder_band, chin_band, crown_band = (59, 63), (37.5, 40.5), (87, 101), (70, 80), (7, 15)
        hair_min, shoulder_max = 4, 106
        center_limit = 2.6
        shoulder_weight = 2
    else:
        raise ValueError('Unknown candidate composition profile')
    before = frame_metrics(anchors, original, height)
    target_x = width/2+center_limit*math.tanh((before['centerX']-width/2)/(width/24))
    best = None
    for head_height in hs:
        scale = float(head_height/anchors.head_height)
        for eye_y in es:
            frame = (scale, target_x-anchors.eye_x*scale, float(eye_y-anchors.eye_y*scale))
            m = frame_metrics(anchors, frame, height)
            if m['hairTopY'] < hair_min or m['shoulderY'] > shoulder_max:
                continue
            score = (1.5*_band(m['headHeight'], *head_band, 2)
                     + 2*_band(m['eyeY'], *eye_band)
                     + shoulder_weight*_band(m['shoulderY'], *shoulder_band, 3)
                     + .8*_band(m['chinY'], *chin_band, 2)
                     + .5*_band(m['crownY'], *crown_band, 2)
                     + .08*((head_height-sum(head_band)/2)/2)**2
                     + .08*((eye_y-sum(eye_band)/2)/2)**2
                     + .025*((head_height-before['headHeight'])/4)**2)
            item = (float(score), float(head_height), float(eye_y), frame, m)
            if best is None or item[:3] < best[:3]:
                best = item
    if best is None:
        raise RuntimeError('No bounded family fit; no per-player override is allowed')
    return best[3], {'profile': profile, 'visibleSize': [width, height], 'score': best[0],
                     'before': before, 'after': best[4], 'transform': list(best[3]),
                     'normalization': 'bounded head/eye/chin/shoulder fit; isotropic scale and translation only',
                     'cropRole': 'weak preference and bounded horizontal pose residual; not extraction input'}


def _render_foreground(source, basis, size, scale, tx, ty):
    # Coordinates above are analysis pixels. Sample at 2x final PNG resolution,
    # then downsample; alpha membership stays exactly the same source basis.
    multiplier = 2
    samples = (size[0]*multiplier, size[1]*multiplier)
    def sample(image, input_factor):
        matrix = (input_factor/(multiplier*scale), 0, -tx*input_factor/scale,
                  0, input_factor/(multiplier*scale), -ty*input_factor/scale)
        return image.transform(samples, Image.Transform.AFFINE, matrix,
                               Image.Resampling.BICUBIC).resize(size, Image.Resampling.LANCZOS)
    return sample(source, 2), sample(basis.alpha, 1)


def compose_portrait(source, basis, anchors, crop, profile, *, foreground=None):
    """Compose without extraction; basis owns anchors/background, foreground pixels.

    Omitting foreground reproduces the historical v1 comparison. Production passes
    the source/hash-bound v3 basis explicitly; no scoped monkeypatch is involved.
    """
    _validate_basis(source, basis)
    if (anchors.evidence['foregroundAlphaSha256'] != basis.alpha_sha256
            or anchors.evidence['sourcePixelSha256'] != basis.source_pixel_sha256):
        raise ValueError('Anchors do not belong to this foreground basis')
    rendered = basis if foreground is None else foreground
    if foreground is not None:
        from PlayerPortraitForegroundV2 import FOREGROUND_PROFILE as V2_PROFILE
        from PlayerPortraitForegroundV3 import FOREGROUND_PROFILE as V3_PROFILE
        if (foreground.profile not in (V2_PROFILE, V3_PROFILE)
                or foreground.source_pixel_sha256 != basis.source_pixel_sha256
                or foreground.evidence.get('priorAlphaSha256') != basis.alpha_sha256
                or foreground.alpha.mode != 'L' or foreground.alpha.size != (512, 768)):
            raise ValueError('Rendered foreground does not belong to the fixed prior/source')
    role = 'Hand' if profile == HAND_PROFILE else 'Shared' if profile == PITCH_PROFILE else None
    if role is None:
        raise ValueError('Unsupported composition profile; Full is frozen')
    frame, evidence = fit_frame(anchors, current_frame(crop, role), profile)
    scale, tx, ty = frame
    if role == 'Hand':
        size = (192, 128)
        subject, alpha = _render_foreground(source, rendered, size, scale*2, tx*2, ty*2)
        background = hand_background(size)
    else:
        size = (512, 768)
        uv_width = 1/1.08
        uv_height = ((512/768)/(130/112))/1.08
        uv_left = .5-uv_width/2
        uv_top = .278-uv_height*.42
        factor = uv_width*512/130
        subject, alpha = _render_foreground(source, rendered, size, scale*factor,
                                           uv_left*512+tx*factor, uv_top*768+ty*factor)
        coverage = Image.new('L', size, 255)
        draw = ImageDraw.Draw(coverage)
        start, end = size[1]*(uv_top+uv_height+.025), size[1]*(uv_top+uv_height+.12)
        for yy in range(size[1]):
            amount = max(0.0, min(1.0, (yy-start)/(end-start)))
            if amount:
                draw.line((0, yy, size[0], yy), fill=round(255*(1-amount)))
        alpha = ImageChops.multiply(alpha, coverage)
        background = pitch_stadium_background(source, basis.alpha, size)
    background.paste(subject, (0, 0), alpha)
    return background, evidence
