"""Generate the frozen Hand Micro production portrait set.

Every player maps explicitly to a source asset, a source-space 3:2 crop, and
approved focal metrics. Outputs are deterministic 1536x1024 review masters and
192x128 runtime textures made with one Lanczos resize. No repaint, sharpening,
reconstruction, or runtime per-player transform is permitted.
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
    ROOT / "ArtSource" / "UI" / "PrototypeTeams" / "HandMicroApprovedRollout"
)
MASTER_OUTPUT_ROOT = OUTPUT_ROOT / "ApprovedMasterViews"
RUNTIME_OUTPUT_ROOT = OUTPUT_ROOT / "Runtime192"


@dataclass(frozen=True)
class Candidate:
    team: str
    player: str
    source_variant: str
    crop: tuple[int, int, int, int]
    head_y: int
    eyes_y: int
    chin_y: int
    shoulders_y: int
    golden: bool = False


CANDIDATES = (
    Candidate("Arsenal", "DavidRaya", "06", (174, 34, 1362, 826),
              74, 356, 610, 780, True),
    Candidate("Arsenal", "WilliamSaliba", "06", (183, 46, 1353, 826),
              85, 330, 600, 725, True),
    Candidate("Arsenal", "BukayoSaka", "06", (228, 40, 1308, 760),
              76, 320, 545, 720, True),
    Candidate("Arsenal", "MartinOdegaard", "06", (168, 15, 1368, 815),
              55, 350, 612, 730, True),
    Candidate("ManchesterCity", "GianluigiDonnarumma", "06",
              (183, 31, 1353, 811), 70, 340, 600, 720, True),
    Candidate("ManchesterCity", "ErlingHaaland", "06", (228, 46, 1308, 766),
              82, 315, 555, 720, True),
    Candidate("Arsenal", "DeclanRice", "06", (183, 25, 1353, 805),
              65, 309, 596, 708),
    Candidate("Arsenal", "GabrielMartinelli", "Validation_05",
              (153, 45, 1383, 865), 90, 350, 626, 735),
    Candidate("Arsenal", "GabrielMagalhaes", "Validation_05",
              (168, 50, 1368, 850), 91, 357, 635, 715),
    Candidate("Arsenal", "MikelMerino", "Validation_05",
              (168, 35, 1368, 835), 75, 355, 630, 700),
    Candidate("ManchesterCity", "PhilFoden", "06", (228, 35, 1308, 755),
              74, 321, 550, 665),
    Candidate("ManchesterCity", "Rodri", "06", (198, 45, 1338, 805),
              83, 340, 598, 690),
    Candidate("ManchesterCity", "RubenDias", "06", (228, 45, 1308, 765),
              83, 315, 548, 660),
    Candidate("ManchesterCity", "JoskoGvardiol", "Validation_05",
              (228, 35, 1308, 755), 70, 318, 570, 680),
    Candidate("ManchesterCity", "BernardoSilva", "Validation_05",
              (183, 40, 1353, 820), 78, 348, 625, 685),
    Candidate("ManchesterCity", "JeremyDoku", "Validation_05",
              (168, 30, 1368, 830), 68, 352, 620, 680),
)

# Frozen byte-level contract for the accepted 16-player production set.
# Values are (ApprovedMaster SHA-256, ApprovedRuntime192 SHA-256).
EXPECTED_HASHES = {
    "Arsenal/BukayoSaka": (
        "26e5fcaba1c275bbe4da597c83d75487902bae6d209e398cff7469d24fdcedb5",
        "2a96b83777166ce357a5c2baa2c4d5643edb33fb2ee9b51a3342f2a622d7a554"),
    "Arsenal/DavidRaya": (
        "a795914d90bb7f6413497e0452900c38b669292eff8606ba1db68280bba363b9",
        "f712994ba004790e5bcd2ae27c1c87c01794b670bc6ea84dceec0e94407748c3"),
    "Arsenal/DeclanRice": (
        "8e341990e571f08c2e19f4eac7c568289b712bbd49eaf9783dedabcd589c8448",
        "6ccf193aa64eb51176465275c8fdd9dd2b34fb8d6da3d8d97f9022f919464f8a"),
    "Arsenal/GabrielMagalhaes": (
        "04efa9f740396eaf9f698d5791a0d94690a5d22bef01c1fae31dffc848957f36",
        "019876d76821a68ae2aade6001211edc0b31e028a8f274617b8d4b2612e9f2f7"),
    "Arsenal/GabrielMartinelli": (
        "01245f38da395dfb428f661957f42118762dede548884dcf51efd3485545e173",
        "0d05e079f7baa9ef0f9a818b402b86a9fe43976c1701ceee829daf5a7c9650a7"),
    "Arsenal/MartinOdegaard": (
        "ca22383e4b4096b5c9a7bb93355576ee6afbc7a646daead291337ca15110ddcc",
        "03a2ac06d3da7d3b07dbda90a574273649c6d0f520464d9deb7a6535dc45e3c3"),
    "Arsenal/MikelMerino": (
        "a951907e59db57b9d79df199ebf43cfa9088fed4ef18cbe7af36d457391cdc39",
        "d047bfea80a318e0cf88c40634088cd2e9c2137da02c34670119fa89622c04cc"),
    "Arsenal/WilliamSaliba": (
        "e446b1a38a5eb8659a973691a845112b09614555ee7eadca43d7f448738c3639",
        "d44e05c4087282dfd1eec24e5f238329ceccfa8734576ba9edb92919f4a44b11"),
    "ManchesterCity/BernardoSilva": (
        "ff7c95b084193af32d2e54db8ede6c1c0f37a1b1e8c5123f6b842829367e4cc8",
        "a8c7d91538c5ac735826fef3a59e75bc5ff6c2dd5a7ac696a78f4c3c5320971d"),
    "ManchesterCity/ErlingHaaland": (
        "f00af3b70e9ab4c1d1f0943cc724f6613e02fb5d0101ec7330683fa2d8a2c6a8",
        "9fafd9131ef4bc4a2a9ea9b76f25cbb7f4411f77c06317a38877007a40a6e15b"),
    "ManchesterCity/GianluigiDonnarumma": (
        "139b84b6b372e073f9e9656b0e66c42bfb74dba0ab5cd5ba805c2007f80fb8c9",
        "80538ec0c7d76258c91bf6113764038bb0718301bd3335073d87f1027bbb51f9"),
    "ManchesterCity/JeremyDoku": (
        "b8ba8449bf5fc0c17f4889c918e3ac7cf4f361b04f6a875ee7f80e3edfe76130",
        "c52c56cf4eb4a8235cb788832c349d05fc672a4f59fd98c7d5d2a260ccd36a0d"),
    "ManchesterCity/JoskoGvardiol": (
        "a538526db1b0204033c85dad5e408af23ca5c792f9bee4d013409da168c53ede",
        "ec29280a4f4240b39e298f719d3ac43d891a596c2dc5d28f38c1e034e8d13200"),
    "ManchesterCity/PhilFoden": (
        "8d18dcb32487da98b8c265a59ffcb433f1c9fb222fdd74dceec4db1f6c8c60f2",
        "e911a23725f397c222de49d0047365d6c9ecb986290ff898ce263bb8cf680c90"),
    "ManchesterCity/Rodri": (
        "ac02447ce0ebb46700edc0069caefc5024c11a5b52f6c1a14c03306926bc12b2",
        "446f3cc7089162e838c72186fa5ba114195fcd262f2bec3a8a4bd9901f5447fb"),
    "ManchesterCity/RubenDias": (
        "9acbd34fc91433f49701f1a504374ca70dd2377695056d39675d43ca464400fc",
        "455caa7a4c5c77ab671640479ccfce96cfe7c21bb88cae9e405d64eecc261e4d"),
}



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

verified_hashes = 0
for candidate in CANDIDATES:
    source = (
        ROOT / "ArtSource" / "UI" / "PrototypeTeams" / candidate.team
        / "HandMicroPortraits"
        / f"T_Prototype_{candidate.team}_{candidate.player}_HandMicro_"
        f"{candidate.source_variant}.png"
    )
    if not source.is_file():
        raise RuntimeError(f"Approved rollout source is missing: {source}")

    left, top, right, bottom = candidate.crop
    crop_width = right - left
    crop_height = bottom - top
    if crop_width * 2 != crop_height * 3:
        raise RuntimeError(f"Rollout crop is not exactly 3:2: {candidate}")
    if not (0 <= left < right <= SOURCE_SIZE[0]
            and 0 <= top < bottom <= SOURCE_SIZE[1]):
        raise RuntimeError(f"Rollout crop is outside the source: {candidate}")
    head = mapped_percent(candidate.head_y, top, crop_height)
    chin = mapped_percent(candidate.chin_y, top, crop_height)
    if not 4.0 <= head <= 7.0:
        raise RuntimeError(f"Rollout head margin is outside D3 guide: {candidate}")
    if not 68.0 <= chin <= 75.0:
        raise RuntimeError(f"Rollout chin position is outside D3 guide: {candidate}")

    with Image.open(source) as image:
        image.load()
        if image.size != SOURCE_SIZE or image.format != "PNG":
            raise RuntimeError(f"Unexpected rollout source: {source} {image.size}")
        crop_view = image.convert("RGB").crop(candidate.crop)
        master_view = crop_view.resize(
            MASTER_VIEW_SIZE, Image.Resampling.LANCZOS, reducing_gap=3.0
        )
        runtime = crop_view.resize(
            RUNTIME_SIZE, Image.Resampling.LANCZOS, reducing_gap=3.0
        )
        stem = f"T_Prototype_{candidate.team}_{candidate.player}_HandMicro"
        master_output = MASTER_OUTPUT_ROOT / f"{stem}_ApprovedMaster.png"
        runtime_output = RUNTIME_OUTPUT_ROOT / f"{stem}_ApprovedRuntime192.png"
        master_view.save(master_output, format="PNG", optimize=False)
        runtime.save(runtime_output, format="PNG", optimize=False)

    key = f"{candidate.team}/{candidate.player}"
    expected_master, expected_runtime = EXPECTED_HASHES[key]
    if (sha256(master_output) != expected_master
            or sha256(runtime_output) != expected_runtime):
        raise RuntimeError(f"Frozen production output changed pixels: {candidate}")
    verified_hashes += 1

    print(
        "FMCODEX_HAND_MICRO_PRODUCTION_ITEM=PASS "
        f"player={candidate.player} production=true "
        f"source={source} source_sha256={sha256(source)} "
        f"crop={','.join(map(str, candidate.crop))} "
        f"scale={SOURCE_SIZE[0] / crop_width:.4f} horizontal_anchor=50.0pct "
        f"head={head:.1f}pct "
        f"eyes={mapped_percent(candidate.eyes_y, top, crop_height):.1f}pct "
        f"chin={chin:.1f}pct "
        f"shoulders={mapped_percent(candidate.shoulders_y, top, crop_height):.1f}pct "
        f"master_output={master_output} master_size=1536x1024 "
        f"runtime_output={runtime_output} runtime_size=192x128 "
        "resample=LANCZOS sharpen=none reconstruction=none repaint=none "
        "runtime_transform=none"
    )

print(
    "FMCODEX_HAND_MICRO_PRODUCTION=PASS "
    f"inventory_count={len(CANDIDATES)} hashes_verified={verified_hashes} "
    "aspect=3:2 runtime=192x128 resample=LANCZOS sharpen=none "
    "reconstruction=none"
)

