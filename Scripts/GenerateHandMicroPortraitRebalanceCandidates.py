"""Generate the bounded Stage 6.13.1.3.10.4.1 D2 portrait set.

D2 is a deterministic source-space reframe of the existing approved
1536x1024 Hand Micro masters.  It moves the D1 composition a small step toward
greater face presence and later shoulder/jersey entry.  It never reconstructs,
repaints, or sharpens source pixels.
"""

from __future__ import annotations

import hashlib
from dataclasses import dataclass
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SOURCE_SIZE = (1536, 1024)
MASTER_VIEW_SIZE = (1536, 1024)
RUNTIME_SIZE = (192, 128)
OUTPUT_ROOT = ROOT / "ArtSource" / "UI" / "Diagnostics" / "HandMicroPortraitRebalance"
MASTER_OUTPUT_ROOT = OUTPUT_ROOT / "RebalancedMasterViews"
RUNTIME_OUTPUT_ROOT = OUTPUT_ROOT / "Runtime192"


@dataclass(frozen=True)
class Candidate:
    team: str
    player: str
    d1_crop: tuple[int, int, int, int]
    d2_crop: tuple[int, int, int, int]
    head_y: int
    eyes_y: int
    chin_y: int
    shoulders_y: int


CANDIDATES = (
    Candidate("Arsenal", "DavidRaya", (63, 12, 1473, 952),
              (93, 20, 1443, 920), 74, 356, 610, 780),
    Candidate("Arsenal", "WilliamSaliba", (70, 18, 1465, 948),
              (100, 27, 1435, 917), 85, 330, 600, 725),
    Candidate("Arsenal", "BukayoSaka", (76, 18, 1459, 940),
              (108, 23, 1428, 903), 76, 320, 545, 720),
    Candidate("Arsenal", "MartinOdegaard", (49, 0, 1486, 958),
              (85, 0, 1450, 910), 55, 350, 612, 730),
    Candidate("ManchesterCity", "GianluigiDonnarumma", (63, 12, 1473, 952),
              (93, 16, 1443, 916), 70, 340, 600, 720),
    Candidate("ManchesterCity", "ErlingHaaland", (76, 20, 1459, 942),
              (108, 20, 1428, 900), 82, 315, 555, 720),
)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def mapped_percent(source_y: int, crop_top: int, crop_height: int) -> float:
    return 100.0 * (source_y - crop_top) / crop_height


MASTER_OUTPUT_ROOT.mkdir(parents=True, exist_ok=True)
RUNTIME_OUTPUT_ROOT.mkdir(parents=True, exist_ok=True)

for candidate in CANDIDATES:
    source = (
        ROOT / "ArtSource" / "UI" / "PrototypeTeams" / candidate.team
        / "HandMicroPortraits"
        / f"T_Prototype_{candidate.team}_{candidate.player}_HandMicro_06.png"
    )
    if not source.is_file():
        raise RuntimeError(f"Approved portrait master is missing: {source}")

    with Image.open(source) as image:
        image.load()
        if image.size != SOURCE_SIZE:
            raise RuntimeError(
                f"Unexpected source size {image.size}, expected {SOURCE_SIZE}: {source}"
            )
        if image.format != "PNG" or image.mode not in ("RGB", "RGBA"):
            raise RuntimeError(
                f"Expected RGB/RGBA PNG, got {image.format}/{image.mode}: {source}"
            )
        left, top, right, bottom = candidate.d2_crop
        crop_width = right - left
        crop_height = bottom - top
        if crop_width * 2 != crop_height * 3:
            raise RuntimeError(f"D2 crop is not exactly 3:2: {candidate}")
        if not (0 <= left < right <= image.width and 0 <= top < bottom <= image.height):
            raise RuntimeError(f"D2 crop is outside the approved master: {candidate}")
        d1_width = candidate.d1_crop[2] - candidate.d1_crop[0]
        d1_height = candidate.d1_crop[3] - candidate.d1_crop[1]
        if crop_width >= d1_width or crop_height >= d1_height:
            raise RuntimeError(f"D2 must be a bounded tighter reframe than D1: {candidate}")

        crop_view = image.convert("RGB").crop(candidate.d2_crop)
        master_view = crop_view.resize(
            MASTER_VIEW_SIZE, Image.Resampling.LANCZOS, reducing_gap=3.0
        )
        runtime = crop_view.resize(
            RUNTIME_SIZE, Image.Resampling.LANCZOS, reducing_gap=3.0
        )

        stem = f"T_Prototype_{candidate.team}_{candidate.player}_HandMicro"
        master_output = MASTER_OUTPUT_ROOT / f"{stem}_RebalancedMaster.png"
        runtime_output = RUNTIME_OUTPUT_ROOT / f"{stem}_RebalancedRuntime192.png"
        master_view.save(master_output, format="PNG", optimize=False)
        runtime.save(runtime_output, format="PNG", optimize=False)

    d1_scale = SOURCE_SIZE[0] / d1_width
    d2_scale = SOURCE_SIZE[0] / crop_width
    print(
        "FMCODEX_HAND_MICRO_PORTRAIT_REBALANCE_ITEM=PASS "
        f"player={candidate.player} source={source} source_sha256={sha256(source)} "
        f"d1_crop={','.join(map(str, candidate.d1_crop))} "
        f"d2_crop={','.join(map(str, candidate.d2_crop))} "
        f"d1_scale={d1_scale:.4f} d2_scale={d2_scale:.4f} "
        f"relative_presence_gain={(d2_scale / d1_scale - 1.0) * 100.0:.1f}pct "
        "horizontal_anchor=50.0pct "
        f"head={mapped_percent(candidate.head_y, top, crop_height):.1f}pct "
        f"eyes={mapped_percent(candidate.eyes_y, top, crop_height):.1f}pct "
        f"chin={mapped_percent(candidate.chin_y, top, crop_height):.1f}pct "
        f"shoulders={mapped_percent(candidate.shoulders_y, top, crop_height):.1f}pct "
        f"master_output={master_output} master_size=1536x1024 "
        f"runtime_output={runtime_output} runtime_size=192x128 "
        "resample=LANCZOS sharpen=none reconstruction=none repaint=none"
    )

print(
    "FMCODEX_HAND_MICRO_PORTRAIT_REBALANCE=PASS "
    f"source_count={len(CANDIDATES)} master_view_count={len(CANDIDATES)} "
    f"runtime_count={len(CANDIDATES)} aspect=3:2 resample=LANCZOS "
    "sharpen=none reconstruction=none"
)
