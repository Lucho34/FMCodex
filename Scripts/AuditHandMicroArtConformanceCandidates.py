"""Read-only UE audit for current C and art-conformed D candidates."""

import unreal


PLAYERS = (
    ("Arsenal", "DavidRaya"),
    ("Arsenal", "WilliamSaliba"),
    ("Arsenal", "BukayoSaka"),
    ("Arsenal", "MartinOdegaard"),
    ("ManchesterCity", "GianluigiDonnarumma"),
    ("ManchesterCity", "ErlingHaaland"),
)


def audit(role: str, asset_path: str) -> None:
    asset = unreal.load_asset(asset_path)
    if not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(f"Texture audit load failed: {asset_path}")
    size = (asset.blueprint_get_size_x(), asset.blueprint_get_size_y())
    if size != (192, 128):
        raise RuntimeError(f"Unexpected {role} size {size}: {asset_path}")
    unreal.log(
        "FMCODEX_HAND_MICRO_ART_CONFORMANCE_AUDIT_ITEM=PASS "
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
        "current_runtime192",
        "/Game/Developers/FMCodex/HandMicroDiagnostics/"
        f"T_Prototype_{team}_{player}_HandMicro_Runtime192",
    )
    audit(
        "art_conformed_runtime192",
        "/Game/Developers/FMCodex/HandMicroArtConformance/"
        f"T_Prototype_{team}_{player}_HandMicro_ArtConformedRuntime192",
    )

unreal.log(
    "FMCODEX_HAND_MICRO_ART_CONFORMANCE_AUDIT=PASS "
    f"player_count={len(PLAYERS)} texture_count={len(PLAYERS) * 2}"
)
