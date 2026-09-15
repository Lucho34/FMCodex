"""Production SourceSpaceForeground_v2; deterministic recovery over the v1 prior.

Only source pixels enter extraction. Local head/neck appearance, contour-supported
seeds and bounded side-component adjacency recover false negatives before framing.
The convex envelope constrains seeds only; it is never used as the output alpha.
"""
import cv2
import numpy as np
from PIL import Image, ImageDraw, ImageFilter

from PlayerPortraitForeground import (
    ANALYSIS_SIZE, ForegroundBasis, _dependencies, extract_source_foreground as extract_v1,
)

FOREGROUND_PROFILE = 'SourceSpaceForeground_v2'
LOCAL_ITERATIONS = 6
ADJACENCY_PIXELS = 8
EDGE_SIGMA_PIXELS = .5
EDGE_SUPPORT_PIXELS = 2
SEED_CORE_RADIUS_PIXELS = 3
BACKGROUND_GRADIENT_LIMIT = 7
BACKGROUND_MINIMUM_FACE_FRACTION = .4


def _contour_supported_seeds(foreground, shapes, face, lower, baseline):
    x, y, w, h = face
    # Seed confidence only: never erode the final alpha or its long-hair tips.
    kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (7, 7))
    core = cv2.erode(foreground.astype("uint8"), kernel, iterations=1) > 0
    head = core.copy()
    head[:max(0, int(y-.5*h))] = False
    head[min(ANALYSIS_SIZE[1], int(y+lower*h)):] = False
    head[:, :max(0, int(x))] = False
    head[:, min(ANALYSIS_SIZE[0], int(x+w)):] = False
    contours, _ = cv2.findContours(head.astype('uint8'), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not contours:
        raise RuntimeError('No supported head contour; candidate extraction rejected')
    hull = np.zeros(ANALYSIS_SIZE[::-1], np.uint8)
    cv2.fillConvexPoly(hull, cv2.convexHull(np.concatenate(contours)), 255)
    baseline_core = cv2.erode(baseline.astype("uint8"), kernel, iterations=1) > 0
    return shapes & ((hull > 0) | baseline_core)


def _classify(pixels, mask):
    mask = mask.copy()
    mask[0, :] = mask[-1, :] = cv2.GC_BGD
    mask[:, 0] = mask[:, -1] = cv2.GC_BGD
    cv2.setRNGSeed(0)
    cv2.grabCut(pixels.copy(), mask, None, np.zeros((1, 65)), np.zeros((1, 65)),
                LOCAL_ITERATIONS, cv2.GC_INIT_WITH_MASK)
    return ((mask == cv2.GC_FGD) | (mask == cv2.GC_PR_FGD)).astype('uint8')


def _select_components(binary, seeds, face=None, origin=None):
    count, labels, stats, _ = cv2.connectedComponentsWithStats(binary, 8)
    retained = np.zeros_like(binary)
    records = []
    for label in range(1, count):
        region = labels == label
        overlap = int(np.count_nonzero(region & seeds))
        if overlap >= 16:
            retained[region] = 1
            records.append({'area': int(stats[label, cv2.CC_STAT_AREA]),
                            'evidence': 'seed', 'seedOverlap': overlap})
    if not np.any(retained):
        raise RuntimeError('No seed-supported local foreground')
    if face is not None:
        # One adjacency pass against anchored components only: no island chaining.
        x, y, w, h = face
        left, top = origin
        distance = cv2.distanceTransform((retained == 0).astype('uint8'), cv2.DIST_L2, 5)
        yy, xx = np.indices(binary.shape)
        side = ((yy+top >= y+.25*h) & (yy+top <= y+.95*h)
                & (xx+left >= x-.10*w) & (xx+left <= x+1.10*w))
        for label in range(1, count):
            region = labels == label
            area = int(stats[label, cv2.CC_STAT_AREA])
            if np.any(retained[region]):
                continue
            nearby = int(np.count_nonzero(region & (distance <= ADJACENCY_PIXELS)))
            if area >= 64 and np.count_nonzero(region & side) >= .85*area and nearby >= 16:
                retained[region] = 1
                records.append({'area': area, 'evidence': 'bounded-side-adjacency',
                                'nearbyPixels': nearby})
    return retained > 0, records


def _merge_recovery(prior, recovered):
    old = np.asarray(prior)
    base = old > 127
    added = recovered & ~base
    if not np.any(added):
        return prior.copy(), 0
    # Preserve old membership and alpha exactly outside a 2px new-edge neighborhood.
    # No dilation/closing/hole-fill is applied to the repaired silhouette.
    feathered = np.asarray(Image.fromarray((base | recovered).astype('uint8')*255)
                           .filter(ImageFilter.GaussianBlur(EDGE_SIGMA_PIXELS)))
    distance = cv2.distanceTransform((~added).astype('uint8'), cv2.DIST_L2, 5)
    output = np.where(distance <= EDGE_SUPPORT_PIXELS, np.maximum(old, feathered), old)
    return Image.fromarray(output.astype('uint8')), int(np.count_nonzero(added))


def extract_source_foreground(source, profile=FOREGROUND_PROFILE):
    return extract_source_bases(source, profile)[1]


def extract_source_bases(source, profile=FOREGROUND_PROFILE):
    """Return the frozen measurement/background prior and the rendered v2 basis.

    Both come from one source-only extraction. Keeping the prior explicit preserves
    the accepted v3 framing/background while v2 repairs subject membership.
    """
    if profile != FOREGROUND_PROFILE:
        raise ValueError('Unknown foreground profile: '+str(profile))
    prior = extract_v1(source)
    _dependencies()
    x, y, w, h = face = prior.evidence['faceBox']
    left, top = max(0, int(x-.42*w)), max(0, int(y-.6*h))
    right, bottom = min(512, int(x+1.42*w)), min(768, int(y+1.65*h))
    window = np.s_[top:bottom, left:right]
    pixels = np.asarray(source.resize(ANALYSIS_SIZE, Image.Resampling.LANCZOS))
    base = np.asarray(prior.alpha) > 127
    # Only smooth source regions connected to the local exterior can veto new
    # foreground. Existing alpha is protected; crown/hair above the lower face
    # is excluded because low contrast alone cannot distinguish dark hair.
    local_rgb = pixels[window].astype('float32')
    gx = cv2.Sobel(local_rgb, cv2.CV_32F, 1, 0, ksize=3)/8
    gy = cv2.Sobel(local_rgb, cv2.CV_32F, 0, 1, ksize=3)/8
    gradient = np.sqrt(np.sum(gx*gx+gy*gy, axis=2))
    smooth_exterior = ((gradient <= BACKGROUND_GRADIENT_LIMIT) & ~base[window]).astype('uint8')
    count, labels, _, _ = cv2.connectedComponentsWithStats(smooth_exterior, 8)
    exterior = np.zeros_like(smooth_exterior, dtype=bool)
    for label in range(1, count):
        region = labels == label
        if np.any(region[:, 0]) or np.any(region[:, -1]) or np.any(region[0]):
            exterior[region] = True
    background = np.zeros_like(base)
    background[window] = exterior
    background[:int(y+BACKGROUND_MINIMUM_FACE_FRACTION*h)] = False
    envelope = Image.new('L', ANALYSIS_SIZE)
    draw = ImageDraw.Draw(envelope)
    draw.ellipse((x-.18*w, y-.4*h, x+1.18*w, y+1.06*h), fill=255)
    draw.rectangle((x-.18*w, y+.25*h, x+1.18*w, y+.95*h), fill=255)
    draw.polygon([(x+.10*w, y+.6*h), (x+.90*w, y+.6*h),
                  (x+.91*w, y+1.65*h), (x+.09*w, y+1.65*h)], fill=255)
    allowed = np.asarray(envelope) > 0
    shapes = Image.new('L', ANALYSIS_SIZE)
    draw = ImageDraw.Draw(shapes)
    draw.ellipse((x+.035*w, y+.005*h, x+.965*w, y+.99*h), fill=255)
    draw.rectangle((x+.06*w, y+.28*h, x+.94*w, y+.66*h), fill=255)
    draw.ellipse((x+.12*w, y-.08*h, x+.88*w, y+.32*h), fill=255)
    draw.polygon([(x+.30*w, y+.76*h), (x+.70*w, y+.76*h),
                  (x+.70*w, y+1.5*h), (x+.30*w, y+1.5*h)], fill=255)
    shapes = np.asarray(shapes) > 0
    seeds = _contour_supported_seeds(base, shapes, face, .78, base)
    mask = np.full(ANALYSIS_SIZE[::-1], cv2.GC_BGD, np.uint8)
    mask[allowed] = cv2.GC_PR_BGD
    mask[base & allowed] = cv2.GC_PR_FGD
    seeds &= ~background
    mask[seeds] = cv2.GC_FGD
    mask[background] = cv2.GC_BGD
    first = _classify(pixels[window], mask[window])
    local, first_records = _select_components(first, seeds[window], face, (left, top))
    recovered = np.zeros_like(base)
    recovered[window] = local
    # Recovered ears become source-contour evidence in one fixed second pass.
    # Restrict the seed hull to the face box; hanging braids and cheek/neck gaps
    # outside it cannot become forced foreground simply through a broad hull.
    refined_seeds = _contour_supported_seeds(recovered, shapes, face, .95, base)
    refined_seeds &= ~background
    refinement = mask[window].copy()
    refinement[local] = cv2.GC_PR_FGD
    refinement[refined_seeds[window]] = cv2.GC_FGD
    refinement[background[window]] = cv2.GC_BGD
    second = _classify(pixels[window], refinement)
    final, final_records = _select_components(second, refined_seeds[window])
    recovered[window] = final
    alpha, added = _merge_recovery(prior.alpha, recovered)
    evidence = {
        'analysisSize': list(ANALYSIS_SIZE), 'faceBox': face,
        'priorProfile': prior.profile, 'priorAlphaSha256': prior.alpha_sha256,
        'priorEvidence': prior.evidence, 'localBox': [left, top, right, bottom],
        'localGrabCutPasses': 2, 'localGrabCutIterationsPerPass': LOCAL_ITERATIONS,
        'seedHullLowerFaceFractions': [.78, .95], 'hullIsFinalMask': False,
        'seedCoreErosionRadiusPixels': SEED_CORE_RADIUS_PIXELS,
        'seedCoreErosionIterations': 1, 'seedCoreAffectsFinalAlpha': False,
        'backgroundGradientLimit': BACKGROUND_GRADIENT_LIMIT,
        'backgroundMinimumFaceFraction': BACKGROUND_MINIMUM_FACE_FRACTION,
        'backgroundRule': '3x3 RGB Sobel / 8; smooth regions connected to local exterior; new pixels only',
        'backgroundVetoPixels': int(np.count_nonzero(background)),
        'adjacencyPixels': ADJACENCY_PIXELS, 'adjacencyMinimumArea': 64,
        'adjacencyMinimumSupport': 16, 'adjacencyMinimumSideFraction': .85,
        'adjacencyPasses': 1, 'initialLocalComponents': first_records,
        'finalLocalComponents': final_records, 'addedBinaryPixels': added,
        'newDilationIterations': 0, 'newClosingIterations': 0, 'newHoleFillPixels': 0,
        'recoveryEdgeSigmaPixels': EDGE_SIGMA_PIXELS,
        'recoveryEdgeSupportPixels': EDGE_SUPPORT_PIXELS,
        'priorAlphaNeverReduced': True, 'randomSeed': 0, 'threads': 1,
        'framingInputsUsed': False,
    }
    return prior, ForegroundBasis(profile, prior.source_pixel_sha256, alpha, evidence)
