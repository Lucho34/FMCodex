"""Versioned, source-space foreground candidate; production v1.1 stays untouched.

No PlayerKey, team, purpose or crop enters extraction. OpenCV's bundled small
Haar face locator provides source anchors; segmentation remains offline GrabCut.
The candidate compositor reproduces the frozen framing math and reuses existing
background helpers. Legacy-mask parity is checked against current PNGs before
candidate output, keeping production generator/provenance valid during trials.
"""
from dataclasses import dataclass
import hashlib
import io
from importlib.metadata import version
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageChops, ImageDraw, ImageFilter, __version__ as PILLOW_VERSION

from GenerateSharedPortraitRuntimeDerivatives import hand_background, pitch_stadium_background

FOREGROUND_PROFILE = 'SourceSpaceForeground_v1'
LEGACY_FOREGROUND_PROFILE = 'CropSeededForeground_v1'
ANALYSIS_SIZE = (512, 768)


def digest(data):
    return hashlib.sha256(data).hexdigest().upper()


def source_pixel_hash(source):
    if source.mode != 'RGB' or source.size != (1024, 1536):
        raise ValueError('Expected unchanged RGB 1024x1536 Master')
    return digest(source.tobytes())


@dataclass(frozen=True)
class ForegroundBasis:
    profile: str
    source_pixel_sha256: str
    alpha: Image.Image
    evidence: dict

    @property
    def alpha_sha256(self):
        return digest(self.alpha.tobytes())


def _dependencies():
    if (version('opencv-python-headless') != '4.10.0.84'
            or np.__version__ != '1.24.1' or PILLOW_VERSION != '9.4.0'):
        raise RuntimeError('Use pinned Scripts/PlayerPortraitBuildRequirements.txt')
    cv2.setNumThreads(1)
    cv2.setRNGSeed(0)


def _keep_anchored_components(binary, anchors):
    """Union substantial components with face/neck/torso evidence, not one pixel."""
    count, labels, stats, _ = cv2.connectedComponentsWithStats(binary, 8)
    keep = np.zeros_like(binary)
    retained = []
    for label in range(1, count):
        region = labels == label
        overlap = [int(np.count_nonzero(region & anchor)) for anchor in anchors]
        if stats[label, cv2.CC_STAT_AREA] >= 64 and max(overlap, default=0) >= 16:
            keep[region] = 255
            retained.append({'area': int(stats[label, cv2.CC_STAT_AREA]), 'anchorOverlap': overlap})
    if not retained:
        raise RuntimeError('No coherent foreground component; do not publish a fake mask')
    return keep, retained


def _bounded_integrity(binary):
    # One 3x3 ellipse closing: radius 1 analysis pixel, 0.195% width / 0.130% height.
    # No blanket dilation. Only enclosed holes <=64 analysis pixels are filled.
    result = cv2.morphologyEx(binary, cv2.MORPH_CLOSE,
                              cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (3, 3)), iterations=1)
    count, labels, stats, _ = cv2.connectedComponentsWithStats(255-result, 8)
    for label in range(1, count):
        x, y, w, h, area = map(int, stats[label])
        if x > 0 and y > 0 and x+w < 512 and y+h < 768 and area <= 64:
            result[labels == label] = 255
    return result


def extract_source_foreground(source, profile=FOREGROUND_PROFILE):
    """Build one stable subject basis before either purpose chooses its framing."""
    if profile != FOREGROUND_PROFILE:
        raise ValueError('Unknown source-space foreground profile: '+str(profile))
    source_hash = source_pixel_hash(source)
    _dependencies()
    pixels = np.asarray(source.resize(ANALYSIS_SIZE, Image.Resampling.LANCZOS))
    cascade_path = Path(cv2.data.haarcascades)/'haarcascade_frontalface_default.xml'
    locator = cv2.CascadeClassifier(str(cascade_path))
    gray = cv2.cvtColor(pixels, cv2.COLOR_RGB2GRAY)
    faces = locator.detectMultiScale(gray, scaleFactor=1.05, minNeighbors=5,
                                      minSize=(65, 65), maxSize=(260, 260))
    candidates = [tuple(map(int, box)) for box in faces
                  if 128 <= box[0]+box[2]/2 <= 384 and box[1]+box[3]/2 < 384]
    if not candidates:
        raise RuntimeError('No reliable source face anchor; keep production assets unchanged')
    x, y, w, h = max(candidates, key=lambda b: (b[2]*b[3], -abs(b[0]+b[2]/2-256), -b[0], -b[1]))
    cx = x+w/2
    # Conservative source-space initialization; purpose crop never participates.
    # This coarse envelope is only a starting trimap. Its edge is reopened below.
    region = Image.new('L', ANALYSIS_SIZE, cv2.GC_BGD)
    draw = ImageDraw.Draw(region)
    draw.ellipse((92,26,430,458), fill=cv2.GC_PR_FGD)
    draw.polygon([(192,265),(337,265),(512,400),(512,768),(0,768),(0,420)],
                 fill=cv2.GC_PR_FGD)
    anchors = []
    face = Image.new('L', ANALYSIS_SIZE);fd=ImageDraw.Draw(face)
    fd.ellipse((x+.16*w, y+.05*h, x+.84*w, y+.94*h), fill=255)
    hair = Image.new('L', ANALYSIS_SIZE);hd=ImageDraw.Draw(hair)
    hd.ellipse((x+.35*w, y-.10*h, x+.65*w, y+.06*h), fill=255)
    neck = Image.new('L', ANALYSIS_SIZE);nd=ImageDraw.Draw(neck)
    nd.polygon([(cx-.12*w,y+.82*h),(cx+.12*w,y+.82*h),
                (256+.11*w,max(y+1.52*h,410)),(256-.11*w,max(y+1.52*h,410))], fill=255)
    torso = Image.new('L', ANALYSIS_SIZE);td=ImageDraw.Draw(torso)
    td.rectangle((205, max(461,round(y+1.6*h)),307,706), fill=255)
    mask = np.array(region)
    for seed in (face,hair,neck,torso):
        mask[np.asarray(seed)>0] = cv2.GC_FGD
    for seed in (face,neck,torso):anchors.append(np.asarray(seed)>0)
    cv2.grabCut(pixels,mask,None,np.zeros((1,65)),np.zeros((1,65)),6,cv2.GC_INIT_WITH_MASK)
    binary=np.where((mask==cv2.GC_FGD)|(mask==cv2.GC_PR_FGD),255,0).astype('uint8')
    initial, initial_components = _keep_anchored_components(binary, anchors)
    # Reopen a bounded 16-pixel source-space uncertainty band, NOT a final
    # dilation. GrabCut chooses the actual pixels; distant stadium lamps stay
    # background. The 3-pixel interior remains confident subject evidence.
    outside_distance = cv2.distanceTransform(255-initial, cv2.DIST_L2, 5)
    inside_distance = cv2.distanceTransform(initial, cv2.DIST_L2, 5)
    refined = np.full(initial.shape,cv2.GC_BGD,dtype='uint8')
    refined[outside_distance<=16] = cv2.GC_PR_BGD
    refined[initial>0] = cv2.GC_PR_FGD
    refined[inside_distance>3] = cv2.GC_FGD
    for anchor in anchors:refined[anchor] = cv2.GC_FGD
    cv2.grabCut(pixels,refined,None,np.zeros((1,65)),np.zeros((1,65)),6,cv2.GC_INIT_WITH_MASK)
    binary=np.where((refined==cv2.GC_FGD)|(refined==cv2.GC_PR_FGD),255,0).astype('uint8')
    kept, components = _keep_anchored_components(binary, anchors)
    repaired = _bounded_integrity(kept)
    alpha = Image.fromarray(repaired).filter(ImageFilter.GaussianBlur(.5))
    evidence = {'analysisSize':list(ANALYSIS_SIZE),'faceBox':[x,y,w,h],
                'faceLocator':'OpenCV bundled haarcascade_frontalface_default.xml',
                'faceLocatorSha256':digest(cascade_path.read_bytes()),
                'opencvDistribution':'opencv-python-headless 4.10.0.84','numpyVersion':np.__version__,
                'pillowVersion':PILLOW_VERSION,'randomSeed':0,'threads':1,'grabCutIterations':6,
                'initialComponents':initial_components,'components':components,
                'refinementBandPixels':16,'refinementInteriorPixels':3,'refinementGrabCutIterations':6,
                'closingKernel':[3,3],'closingIterations':1,
                'maximumEnclosedHolePixels':64,'edgeBlurSigmaPixels':.5,
                'framingInputsUsed':False}
    return ForegroundBasis(profile,source_hash,alpha,evidence)


def compose_with_foreground(source, size, crop, composition, basis):
    """Same v2 framing, explicit already-extracted basis. Never extracts here."""
    if basis.profile not in (FOREGROUND_PROFILE,LEGACY_FOREGROUND_PROFILE):
        raise ValueError('Unversioned foreground basis')
    if basis.source_pixel_sha256 != source_pixel_hash(source):
        raise ValueError('Foreground basis belongs to another source')
    if basis.alpha.mode != 'L' or basis.alpha.size != ANALYSIS_SIZE:
        raise ValueError('Invalid foreground alpha representation')
    x,y,w,h=crop;box=(x*1024,y*1536,(x+w)*1024,(y+h)*1536)
    mask=basis.alpha
    if composition == 'BalancedBust_v2' and size == (192,128):
        ow=round(size[1]*(box[2]-box[0])/(box[3]-box[1]))
        if ow > size[0]:raise ValueError('Hand framing exceeds canvas')
        result=hand_background(size)
        small=source.resize((ow,size[1]),Image.Resampling.LANCZOS,box=box)
        alpha=mask.resize((ow,size[1]),Image.Resampling.LANCZOS,box=tuple(c/2 for c in box))
        result.paste(small,((size[0]-ow)//2,0),alpha)
        return result
    if composition != 'QuietPitchBust_v2' or size != (512,768):
        raise ValueError('Candidate supports existing Hand/Shared framing only; Full is frozen')
    uv_height=((512/768)/(130/112))/1.08
    uv_top=.278-uv_height*.42
    scale=(size[1]*uv_height)/(box[3]-box[1])
    subject_size=(round(1024*scale),round(1536*scale))
    subject=source.resize(subject_size,Image.Resampling.LANCZOS)
    alpha=mask.resize(subject_size,Image.Resampling.LANCZOS)
    result=pitch_stadium_background(source,mask,size)
    subject_top=round(size[1]*uv_top-box[1]*scale)
    fade_start=size[1]*(uv_top+uv_height+.025)
    fade_end=size[1]*(uv_top+uv_height+.12)
    coverage=Image.new('L',subject_size,255);draw=ImageDraw.Draw(coverage)
    for yy in range(subject_size[1]):
        amount=max(0.0,min(1.0,(yy+subject_top-fade_start)/(fade_end-fade_start)))
        if amount:draw.line((0,yy,subject_size[0],yy),fill=round(255*(1-amount)))
    alpha=ImageChops.multiply(alpha,coverage)
    result.paste(subject,((size[0]-subject_size[0])//2,subject_top),alpha)
    return result


def png_bytes(image):
    stream=io.BytesIO();image.save(stream,format='PNG',optimize=False,compress_level=9)
    return stream.getvalue()
