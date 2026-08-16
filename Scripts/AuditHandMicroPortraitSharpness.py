"""Read-only Texture2D audit for the bounded portrait runtime validation set."""

import unreal


PLAYERS = (
    ("Arsenal", "DavidRaya"),
    ("Arsenal", "WilliamSaliba"),
    ("Arsenal", "BukayoSaka"),
    ("Arsenal", "MartinOdegaard"),
    ("ManchesterCity", "GianluigiDonnarumma"),
    ("ManchesterCity", "ErlingHaaland"),
)


def audit(role: str, asset_path: str, expected_size: tuple[int, int]) -> None:
    asset = unreal.load_asset(asset_path)
    if not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(f"Texture audit load failed: {asset_path}")
    size = (asset.blueprint_get_size_x(), asset.blueprint_get_size_y())
    if size != expected_size:
        raise RuntimeError(
            f"Unexpected {role} size {size}, expected {expected_size}: {asset_path}"
        )
    unreal.log(
        "FMCODEX_HAND_MICRO_SHARPNESS_AUDIT_ITEM=PASS "
        f"role={role} asset={asset_path} dimensions={size[0]}x{size[1]} "
        f"compression={asset.get_editor_property('compression_settings')} "
        f"lod_group={asset.get_editor_property('lod_group')} "
        f"mip_generation={asset.get_editor_property('mip_gen_settings')} "
        f"filter={asset.get_editor_property('filter')} "
        f"never_stream={asset.get_editor_property('never_stream')} "
        f"srgb={asset.get_editor_property('srgb')} "
        f"lod_bias={asset.get_editor_property('lod_bias')}"
    )


for team, player in PLAYERS:
    audit(
        "production_high_res",
        f"/Game/UI/Portraits/PrototypeTeams/{team}/HandMicro/"
        f"T_Prototype_{team}_{player}_HandMicro_06",
        (1536, 1024),
    )
    audit(
        "diagnostic_runtime192",
        "/Game/Developers/FMCodex/HandMicroDiagnostics/"
        f"T_Prototype_{team}_{player}_HandMicro_Runtime192",
        (192, 128),
    )

unreal.log(
    "FMCODEX_HAND_MICRO_SHARPNESS_AUDIT=PASS "
    f"player_count={len(PLAYERS)} texture_count={len(PLAYERS) * 2}"
)
