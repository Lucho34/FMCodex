"""Create the bounded non-shipping 192x128 portrait validation set."""

from pathlib import Path

from PIL import Image, ImageFilter, ImageStat


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = PROJECT_ROOT / "ArtSource" / "UI" / "PrototypeTeams"
OUTPUT_ROOT = (
    PROJECT_ROOT
    / "ArtSource"
    / "UI"
    / "Diagnostics"
    / "HandMicroSharpness"
)
PORTRAITS = (
    ("Arsenal", "DavidRaya"),
    ("Arsenal", "WilliamSaliba"),
    ("Arsenal", "BukayoSaka"),
    ("Arsenal", "MartinOdegaard"),
    ("ManchesterCity", "GianluigiDonnarumma"),
    ("ManchesterCity", "ErlingHaaland"),
)


def source_path(team: str, player: str) -> Path:
    return (
        SOURCE_ROOT
        / team
        / "HandMicroPortraits"
        / f"T_Prototype_{team}_{player}_HandMicro_06.png"
    )


def output_path(team: str, player: str) -> Path:
    return OUTPUT_ROOT / f"T_Prototype_{team}_{player}_HandMicro_Runtime192.png"


def main() -> None:
    OUTPUT_ROOT.mkdir(parents=True, exist_ok=True)
    for team, player in PORTRAITS:
        source_file = source_path(team, player)
        output_file = output_path(team, player)
        with Image.open(source_file) as source:
            if source.size != (1536, 1024):
                raise RuntimeError(
                    f"Unexpected master size for {source_file}: {source.size}"
                )
            if source.format != "PNG" or source.mode not in ("RGB", "RGBA"):
                raise RuntimeError(
                    f"Unexpected master format/mode for {source_file}: "
                    f"{source.format}/{source.mode}"
                )
            edge_rms = ImageStat.Stat(
                source.convert("L").filter(ImageFilter.FIND_EDGES)
            ).rms[0]
            runtime = source.convert("RGB").resize(
                (192, 128), Image.Resampling.LANCZOS, reducing_gap=3.0
            )
            runtime.save(output_file, format="PNG", optimize=True)

        with Image.open(output_file) as check:
            if (
                check.size != (192, 128)
                or check.format != "PNG"
                or check.mode != "RGB"
            ):
                raise RuntimeError(
                    f"Unexpected diagnostic output: {output_file} "
                    f"{check.size} {check.format}/{check.mode}"
                )
        print(
            "FMCODEX_HAND_MICRO_SHARPNESS_DERIVATIVE_ITEM=PASS "
            f"player={player} source={source_file} source_size=1536x1024 "
            f"source_format=PNG source_mode=RGB_OR_RGBA output={output_file} "
            f"source_edge_rms={edge_rms:.3f} jpeg_quantization=absent "
            "output_size=192x128 output_format=PNG output_mode=RGB "
            "resample=LANCZOS sharpen=none crop=none reframe=none"
        )

    print(
        "FMCODEX_HAND_MICRO_SHARPNESS_DERIVATIVE=PASS "
        f"source_count={len(PORTRAITS)} output_count={len(PORTRAITS)} "
        "source_size=1536x1024 output_size=192x128 "
        "resample=LANCZOS sharpen=none crop=none reframe=none"
    )


if __name__ == "__main__":
    main()
