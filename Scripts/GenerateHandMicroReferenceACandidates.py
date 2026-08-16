"""Generate Stage 6.13.1.3.10.4.2 Reference-A D3 portraits.

D3 is a deterministic source-space reframe of the existing approved
1536x1024 Hand Micro masters.  Per-player 3:2 crop windows target a larger
head, later shoulder entry, and less chest mass than D2.  No pixels are
reconstructed, repainted, sharpened, or enhanced.
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
OUTPUT_ROOT = ROOT / "ArtSource" / "UI" / "Diagnostics" / "HandMicroReferenceA"
MASTER_OUTPUT_ROOT = OUTPUT_ROOT / "ReferenceAMasterViews"
RUNTIME_OUTPUT_ROOT = OUTPUT_ROOT / "Runtime192"


@dataclass(frozen=True)
class Candidate:
    team: str
    player: str
    d2_crop: tuple[int, int, int, int]
    d3_crop: tuple[int, int, int, int]
    head_y: int
    eyes_y: int
    chin_y: int
    shoulders_y: int


CANDIDATES = (
    Candidate("Arsenal", "DavidRaya", (93, 20, 1443, 920),
              (174, 34, 1362, 826), 74, 356, 610, 780),
    Candidate("Arsenal", "WilliamSaliba", (100, 27, 1435, 917),
              (183, 46, 1353, 826), 85, 330, 600, 725),
    Candidate("Arsenal", "BukayoSaka", (108, 23, 1428, 903),
              (228, 40, 1308, 760), 76, 320, 545, 720),
    Candidate("Arsenal", "MartinOdegaard", (85, 0, 1450, 910),
              (168, 15, 1368, 815), 55, 350, 612, 730),
    Candidate("ManchesterCity", "GianluigiDonnarumma", (93, 16, 1443, 916),
              (183, 31, 1353, 811), 70, 340, 600, 720),
    Candidate("ManchesterCity", "ErlingHaaland", (108, 20, 1428, 900),
              (228, 46, 1308, 766), 82, 315, 555, 720),
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
        left, top, right, bottom = candidate.d3_crop
        crop_width = right - left
        crop_height = bottom - top
        if crop_width * 2 != crop_height * 3:
            raise RuntimeError(f"D3 crop is not exactly 3:2: {candidate}")
        if not (0 <= left < right <= image.width and 0 <= top < bottom <= image.height):
            raise RuntimeError(f"D3 crop is outside the approved master: {candidate}")
        d2_width = candidate.d2_crop[2] - candidate.d2_crop[0]
        d2_height = candidate.d2_crop[3] - candidate.d2_crop[1]
        if crop_width >= d2_width or crop_height >= d2_height:
            raise RuntimeError(f"D3 must be a tighter source reframe than D2: {candidate}")

        head_margin = mapped_percent(candidate.head_y, top, crop_height)
        chin_position = mapped_percent(candidate.chin_y, top, crop_height)
        if not 4.0 <= head_margin <= 7.0:
            raise RuntimeError(f"D3 head margin is outside the draft guide: {candidate}")
        if not 68.0 <= chin_position <= 75.0:
            raise RuntimeError(f"D3 chin position is outside the draft guide: {candidate}")

        crop_view = image.convert("RGB").crop(candidate.d3_crop)
        master_view = crop_view.resize(
            MASTER_VIEW_SIZE, Image.Resampling.LANCZOS, reducing_gap=3.0
        )
        runtime = crop_view.resize(
            RUNTIME_SIZE, Image.Resampling.LANCZOS, reducing_gap=3.0
        )

        stem = f"T_Prototype_{candidate.team}_{candidate.player}_HandMicro"
        master_output = MASTER_OUTPUT_ROOT / f"{stem}_ReferenceAMaster.png"
        runtime_output = RUNTIME_OUTPUT_ROOT / f"{stem}_ReferenceARuntime192.png"
        master_view.save(master_output, format="PNG", optimize=False)
        runtime.save(runtime_output, format="PNG", optimize=False)

    d2_scale = SOURCE_SIZE[0] / d2_width
    d3_scale = SOURCE_SIZE[0] / crop_width
    print(
        "FMCODEX_HAND_MICRO_REFERENCE_A_ITEM=PASS "
        f"player={candidate.player} source={source} source_sha256={sha256(source)} "
        f"d2_crop={','.join(map(str, candidate.d2_crop))} "
        f"d3_crop={','.join(map(str, candidate.d3_crop))} "
        f"d2_scale={d2_scale:.4f} d3_scale={d3_scale:.4f} "
        f"relative_presence_gain={(d3_scale / d2_scale - 1.0) * 100.0:.1f}pct "
        "horizontal_anchor=50.0pct runtime_transform=none "
        f"head={mapped_percent(candidate.head_y, top, crop_height):.1f}pct "
        f"eyes={mapped_percent(candidate.eyes_y, top, crop_height):.1f}pct "
        f"chin={mapped_percent(candidate.chin_y, top, crop_height):.1f}pct "
        f"shoulders={mapped_percent(candidate.shoulders_y, top, crop_height):.1f}pct "
        f"master_output={master_output} master_size=1536x1024 "
        f"runtime_output={runtime_output} runtime_size=192x128 "
        "resample=LANCZOS sharpen=none reconstruction=none repaint=none"
    )

print(
    "FMCODEX_HAND_MICRO_REFERENCE_A=PASS "
    f"source_count={len(CANDIDATES)} master_view_count={len(CANDIDATES)} "
    f"runtime_count={len(CANDIDATES)} aspect=3:2 resample=LANCZOS "
    "sharpen=none reconstruction=none"
)
