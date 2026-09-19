"""Production garment continuity recovery over frozen v2.

Promoted unchanged from the USER PIE accepted Stage 8.5A candidate.
Only source pixels and source-derived foreground enter extraction. Central neck
and bottom-connected torso support bracket substantial missing garment regions;
a bounded source classification recovers their edges. Existing alpha is immutable.
"""
import cv2
import numpy as np
from PIL import Image

from PlayerPortraitForeground import (
    ANALYSIS_SIZE, ForegroundBasis, _dependencies, source_pixel_hash,
)
import PlayerPortraitForegroundV2 as v2

FOREGROUND_PROFILE = 'SourceSpaceForeground_v3'
BRIDGE_MINIMUM_PIXELS = 64
CORE_DISTANCE_PIXELS = 3
RAY_REACH_FACE_WIDTH = .25
BRIDGE_SPAN_FACE_WIDTH = .28
RECOVERY_DISTANCE_FACE_WIDTH = .10
MINIMUM_BRACKET_AXES = 2
BACKGROUND_APPEARANCE_DISTANCE = 18
BACKGROUND_SAMPLE_STRIDE = 8


def _shift(array, dx, dy):
    result = np.zeros_like(array)
    height, width = array.shape
    result[max(0, dy):min(height, height+dy), max(0, dx):min(width, width+dx)] = array[
        max(0, -dy):min(height, height-dy), max(0, -dx):min(width, width-dx)]
    return result


def _body_support(core, face):
    """Separate central neck runs from torso columns connected to a lower row.

    Lateral hair cannot independently serve as the neck endpoint. Narrow central
    runs are a conservative support proxy, not an anatomical segmentation claim.
    """
    x, y, w, h = face
    neck, torso = np.zeros_like(core), np.zeros_like(core)
    center = int(x+w/2)
    for row in range(max(0, int(y+.75*h)), min(768, int(y+2.2*h))):
        if not core[row, center]:
            continue
        left = right = center
        while left > 0 and core[row, left-1]:
            left -= 1
        while right < 511 and core[row, right+1]:
            right += 1
        if right-left+1 <= 1.15*w:
            neck[row, left:right+1] = True
    floor = min(767, int(y+2.1*h))
    for col in range(max(0, int(x-.2*w)), min(512, int(x+1.2*w))):
        row = floor
        while row >= max(0, int(y+.75*h)) and core[row, col]:
            torso[row, col] = True
            row -= 1
    return neck, torso


def _exterior_background(pixels, base):
    rgb = pixels.astype('float32')
    gx = cv2.Sobel(rgb, cv2.CV_32F, 1, 0, ksize=3)/8
    gy = cv2.Sobel(rgb, cv2.CV_32F, 0, 1, ksize=3)/8
    gradient = np.sqrt(np.sum(gx*gx+gy*gy, axis=2))
    smooth = (gradient <= v2.BACKGROUND_GRADIENT_LIMIT) & ~base
    _, labels, _, _ = cv2.connectedComponentsWithStats(smooth.astype('uint8'), 8)
    border = np.unique(np.concatenate([labels[0], labels[-1], labels[:, 0], labels[:, -1]]))
    return np.isin(labels, border) & smooth


def _bridge_seeds(neck, torso, allowed, base, width):
    brackets = np.zeros_like(base, np.uint8)
    for dx, dy in [(0, 1), (1, 1), (-1, 1)]:
        distances = []
        for support in (neck, torso):
            for sign in (-1, 1):
                distance = np.full(base.shape, 999, np.int16)
                for step in range(1, int(RAY_REACH_FACE_WIDTH*width)+1):
                    found = _shift(support, sign*dx*step, sign*dy*step)
                    distance = np.minimum(distance, np.where(found, step, 999))
                distances.append(distance)
        # Neck must be above the gap and torso below; never reverse the bridge.
        span = distances[1]+distances[2]
        brackets += (span <= BRIDGE_SPAN_FACE_WIDTH*width/np.hypot(dx, dy)).astype('uint8')
    proposed = allowed & ~base & (brackets >= MINIMUM_BRACKET_AXES)
    count, labels, stats, _ = cv2.connectedComponentsWithStats(proposed.astype('uint8'), 8)
    areas = [int(stats[i, cv2.CC_STAT_AREA]) for i in range(1, count)]
    retained = [i for i in range(1, count) if stats[i, cv2.CC_STAT_AREA] >= BRIDGE_MINIMUM_PIXELS]
    return np.isin(labels, retained), areas


def _veto_background_appearance(pixels, seeds, background_samples):
    """Reject seed colors already observed in nearby confident source background.

    No color receives privileged membership. A dark, pale or bright region must
    first satisfy the structural bridge, and may then be conservatively rejected.
    """
    palette = pixels[background_samples][::BACKGROUND_SAMPLE_STRIDE].astype('float32')
    result = seeds.copy()
    if len(palette) and seeds.any():
        yy, xx = np.where(seeds)
        rgb = pixels[yy, xx].astype('float32')
        nearest = []
        for start in range(0, len(rgb), 128):
            squared = ((rgb[start:start+128, None, :]-palette[None, :, :])**2).sum(axis=2)
            nearest.extend(np.sqrt(squared.min(axis=1)))
        rejected = np.asarray(nearest) <= BACKGROUND_APPEARANCE_DISTANCE
        result[yy[rejected], xx[rejected]] = False
        count, labels, stats, _ = cv2.connectedComponentsWithStats(result.astype('uint8'), 8)
        retained = [i for i in range(1, count) if stats[i, cv2.CC_STAT_AREA] >= BRIDGE_MINIMUM_PIXELS]
        result = np.isin(labels, retained)
    return result


def _recover_from_v2(source, baseline):
    """Internal review seam; a source-bound v2 baseline is required."""
    source_hash = source_pixel_hash(source)
    if (baseline.profile != v2.FOREGROUND_PROFILE
            or baseline.source_pixel_sha256 != source_hash
            or baseline.alpha.mode != 'L' or baseline.alpha.size != ANALYSIS_SIZE):
        raise ValueError('Garment recovery requires this source and its v2 basis')
    _dependencies()
    pixels = np.asarray(source.resize(ANALYSIS_SIZE, Image.Resampling.LANCZOS))
    base = np.asarray(baseline.alpha) > 127
    x, y, w, h = face = baseline.evidence['faceBox']
    yy, xx = np.indices(base.shape)
    corridor = ((xx >= x-.20*w) & (xx <= x+1.20*w)
                & (yy >= y+.95*h) & (yy <= y+1.95*h))
    distance = cv2.distanceTransform((~base).astype('uint8'), cv2.DIST_L2, 5)
    core = cv2.distanceTransform(base.astype('uint8'), cv2.DIST_L2, 5) >= CORE_DISTANCE_PIXELS
    neck, torso = _body_support(core, face)
    exterior = _exterior_background(pixels, base)
    allowed = corridor & (distance <= RECOVERY_DISTANCE_FACE_WIDTH*w) & ~exterior
    seeds, seed_areas = _bridge_seeds(neck, torso, allowed, base, w)
    background_samples = (exterior & (distance >= .15*w)
                          & (yy >= y+.85*h) & (yy <= y+2*h)
                          & (xx >= x-.5*w) & (xx <= x+1.5*w))
    seeds = _veto_background_appearance(pixels, seeds, background_samples)
    left, right = max(0, int(x-.35*w)), min(512, int(x+1.35*w))
    top, bottom = max(0, int(y+.80*h)), min(768, int(y+2.1*h))
    window = np.s_[top:bottom, left:right]
    recovered = np.zeros_like(base)
    if np.any(seeds):
        labels = np.full(base.shape, cv2.GC_BGD, np.uint8)
        labels[allowed] = cv2.GC_PR_BGD
        labels[base] = cv2.GC_PR_FGD
        labels[core | seeds] = cv2.GC_FGD
        classified = v2._classify(pixels[window], labels[window]) > 0
        _, components, _, _ = cv2.connectedComponentsWithStats(
            (classified & ~base[window]).astype('uint8'), 8)
        supported = np.isin(components, np.unique(components[seeds[window]])) & (components > 0)
        recovered[window] = supported & allowed[window]
    alpha, added = v2._merge_recovery(baseline.alpha, recovered)
    evidence = {
        'candidateOnly': False, 'analysisSize': list(ANALYSIS_SIZE), 'faceBox': face,
        'priorProfile': baseline.evidence['priorProfile'],
        'priorAlphaSha256': baseline.evidence['priorAlphaSha256'],
        'baselineProfile': baseline.profile, 'baselineAlphaSha256': baseline.alpha_sha256,
        'baselineEvidence': baseline.evidence,
        'corridorFaceFractions': [-.20, .95, 1.20, 1.95],
        'coreDistancePixels': CORE_DISTANCE_PIXELS,
        'neckSupport': 'central contiguous core run, maximum width 1.15 face widths',
        'torsoSupport': 'unbroken core columns from y + 2.1 face heights',
        'supportRowFaceFractions': [.75, 2.2],
        'rayReachFaceWidth': RAY_REACH_FACE_WIDTH,
        'maximumBridgeSpanFaceWidth': BRIDGE_SPAN_FACE_WIDTH,
        'bridgeDirection': 'neck above, torso below; vertical and both diagonals',
        'minimumBracketAxes': MINIMUM_BRACKET_AXES,
        'minimumSeedComponentPixels': BRIDGE_MINIMUM_PIXELS,
        'proposedSeedComponentAreas': seed_areas, 'retainedSeedPixels': int(seeds.sum()),
        'backgroundRule': '3x3 RGB Sobel / 8; smooth non-foreground connected to source border',
        'backgroundGradientLimit': v2.BACKGROUND_GRADIENT_LIMIT,
        'backgroundAppearanceDistanceRGB': BACKGROUND_APPEARANCE_DISTANCE,
        'backgroundSampleStride': BACKGROUND_SAMPLE_STRIDE,
        'backgroundSampleMinimumDistanceFaceWidth': .15,
        'backgroundSampleCorridorFaceFractions': [-.5, .85, 1.5, 2.0],
        'backgroundAppearancePurpose': 'veto seed resemblance to local exterior; not foreground color selection',
        'maximumRecoveryDistanceFaceWidth': RECOVERY_DISTANCE_FACE_WIDTH,
        'localBox': [left, top, right, bottom],
        'localGrabCutPasses': int(np.any(seeds)),
        'localGrabCutIterationsPerPass': v2.LOCAL_ITERATIONS,
        'componentRule': 'new component must intersect a retained structural seed; no chaining',
        'addedBinaryPixels': added, 'baselineAlphaNeverReduced': True,
        'newDilationIterations': 0, 'newClosingIterations': 0, 'newHoleFillPixels': 0,
        'recoveryEdgeSigmaPixels': v2.EDGE_SIGMA_PIXELS,
        'recoveryEdgeSupportPixels': v2.EDGE_SUPPORT_PIXELS,
        'randomSeed': 0, 'threads': 1, 'framingInputsUsed': False,
    }
    return ForegroundBasis(FOREGROUND_PROFILE, source_hash, alpha, evidence)


def extract_source_bases(source, profile=FOREGROUND_PROFILE):
    if profile != FOREGROUND_PROFILE:
        raise ValueError('Unknown candidate foreground profile: '+str(profile))
    prior, baseline = v2.extract_source_bases(source)
    return prior, _recover_from_v2(source, baseline)


def extract_source_foreground(source, profile=FOREGROUND_PROFILE):
    return extract_source_bases(source, profile)[1]
