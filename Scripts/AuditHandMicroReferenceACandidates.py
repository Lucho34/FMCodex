"""Read-only D1/D2/D3 Texture2D audit for Stage 6.13.1.3.10.4.2."""

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
        raise RuntimeError(f"D1/D2/D3 audit load failed: {asset_path}")
    size = (asset.blueprint_get_size_x(), asset.blueprint_get_size_y())
    if size != (192, 128):
        raise RuntimeError(f"Unexpected {role} size {size}: {asset_path}")
    unreal.log(
        "FMCODEX_HAND_MICRO_REFERENCE_A_AUDIT_ITEM=PASS "
        f"role={role} asset={asset_path} dimensions=192x128 "
        f"compression={asset.get_editor_property('compression_settings')} "
        f"lod_group={asset.get_editor_property('lod_group')} "
        f"mips={asset.get_editor_property('mip_gen_settings')} "
        f"filter={asset.get_editor_property('filter')} "
        f"never_stream={asset.get_editor_property('never_stream')} "
        f"srgb={asset.get_editor_property('srgb')} lod_bias={asset.get_editor_property('lod_bias')}"
    )


for team, player in PLAYERS:
    audit(
        "d1_art_conformed",
        "/Game/Developers/FMCodex/HandMicroArtConformance/"
        f"T_Prototype_{team}_{player}_HandMicro_ArtConformedRuntime192",
    )
    audit(
        "d2_rebalanced",
        "/Game/Developers/FMCodex/HandMicroPortraitRebalance/"
        f"T_Prototype_{team}_{player}_HandMicro_RebalancedRuntime192",
    )
    audit(
        "d3_reference_a",
        "/Game/Developers/FMCodex/HandMicroReferenceA/"
        f"T_Prototype_{team}_{player}_HandMicro_ReferenceARuntime192",
    )

unreal.log(
    "FMCODEX_HAND_MICRO_REFERENCE_A_AUDIT=PASS "
    f"player_count={len(PLAYERS)} texture_count={len(PLAYERS) * 3}"
)
