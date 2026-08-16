"""Generate the bounded Stage 6.13.1.3.10.4 portrait conformance set.

The source-space crop boxes below are intentionally explicit and reviewable.
They enlarge the existing approved subject without reconstructing, repainting,
sharpening, or otherwise synthesizing pixels.  Runtime derivatives are sampled
directly from the original crop view so the saved 1536x1024 review image does
not introduce an extra resampling pass.
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
OUTPUT_ROOT = (
    ROOT / "ArtSource" / "UI" / "Diagnostics" / "HandMicroArtConformance"
)
MASTER_OUTPUT_ROOT = OUTPUT_ROOT / "ConformedMasterViews"
RUNTIME_OUTPUT_ROOT = OUTPUT_ROOT / "Runtime192"


@dataclass(frozen=True)
class Candidate:
    team: str
    player: str
    crop: tuple[int, int, int, int]
    # Manually audited source landmarks.  They document the source-space
    # intent; they are not a facial-recognition or approval metric.
    head_y: int
    eyes_y: int
    chin_y: int
    shoulders_y: int


CANDIDATES = (
    Candidate("Arsenal", "DavidRaya", (63, 12, 1473, 952), 74, 356, 610, 780),
    Candidate("Arsenal", "WilliamSaliba", (70, 18, 1465, 948), 85, 330, 600, 725),
    Candidate("Arsenal", "BukayoSaka", (76, 18, 1459, 940), 76, 320, 545, 720),
    Candidate("Arsenal", "MartinOdegaard", (49, 0, 1486, 958), 55, 350, 612, 730),
    Candidate(
        "ManchesterCity",
        "GianluigiDonnarumma",
        (63, 12, 1473, 952),
        70,
        340,
        600,
        720,
    ),
    Candidate(
        "ManchesterCity",
        "ErlingHaaland",
        (76, 20, 1459, 942),
        82,
        315,
        555,
        720,
    ),
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
        ROOT
        / "ArtSource"
        / "UI"
        / "PrototypeTeams"
        / candidate.team
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
                f"Expected RGB/RGBA PNG approved master, got {image.format}/{image.mode}: {source}"
            )
        left, top, right, bottom = candidate.crop
        crop_width = right - left
        crop_height = bottom - top
        if crop_width * 2 != crop_height * 3:
            raise RuntimeError(f"Crop is not exactly 3:2: {candidate}")
        if not (0 <= left < right <= image.width and 0 <= top < bottom <= image.height):
            raise RuntimeError(f"Crop is outside the approved master: {candidate}")

        crop_view = image.convert("RGB").crop(candidate.crop)
        master_view = crop_view.resize(
            MASTER_VIEW_SIZE, Image.Resampling.LANCZOS, reducing_gap=3.0
        )
        runtime = crop_view.resize(
            RUNTIME_SIZE, Image.Resampling.LANCZOS, reducing_gap=3.0
        )

        stem = f"T_Prototype_{candidate.team}_{candidate.player}_HandMicro"
        master_output = MASTER_OUTPUT_ROOT / f"{stem}_ArtConformedMaster.png"
        runtime_output = RUNTIME_OUTPUT_ROOT / f"{stem}_ArtConformedRuntime192.png"
        master_view.save(master_output, format="PNG", optimize=False)
        runtime.save(runtime_output, format="PNG", optimize=False)

    crop_width = candidate.crop[2] - candidate.crop[0]
    crop_height = candidate.crop[3] - candidate.crop[1]
    source_scale = SOURCE_SIZE[0] / crop_width
    print(
        "FMCODEX_HAND_MICRO_ART_CONFORMANCE_ITEM=PASS "
        f"player={candidate.player} source={source} source_sha256={sha256(source)} "
        f"source_size=1536x1024 source_format=PNG source_mode=RGB_OR_RGBA "
        f"crop={candidate.crop[0]},{candidate.crop[1]},"
        f"{candidate.crop[2]},{candidate.crop[3]} crop_size={crop_width}x{crop_height} "
        f"source_scale={source_scale:.4f} horizontal_anchor=50.0pct "
        f"head={mapped_percent(candidate.head_y, candidate.crop[1], crop_height):.1f}pct "
        f"eyes={mapped_percent(candidate.eyes_y, candidate.crop[1], crop_height):.1f}pct "
        f"chin={mapped_percent(candidate.chin_y, candidate.crop[1], crop_height):.1f}pct "
        f"shoulders={mapped_percent(candidate.shoulders_y, candidate.crop[1], crop_height):.1f}pct "
        f"master_output={master_output} master_size=1536x1024 "
        f"runtime_output={runtime_output} runtime_size=192x128 "
        "resample=LANCZOS sharpen=none reconstruction=none repaint=none"
    )

print(
    "FMCODEX_HAND_MICRO_ART_CONFORMANCE=PASS "
    f"source_count={len(CANDIDATES)} master_view_count={len(CANDIDATES)} "
    f"runtime_count={len(CANDIDATES)} aspect=3:2 resample=LANCZOS "
    "sharpen=none reconstruction=none"
)
