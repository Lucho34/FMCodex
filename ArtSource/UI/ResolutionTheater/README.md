# Resolution Theater athletes

`T_Theater_Athletes.png` is original artwork generated for FMCodex with the built-in imagegen tool on 2026-09-24. It uses no downloaded artwork, reference tracing, logos, or named athlete likenesses. The two generic adult figures represent attacking and defending motion, not participants or match facts.

The chosen original monochrome atlas is 1448 × 1086 RGBA with transparent background. Its detailed kit and anatomical contours are intentionally tinted steel blue at low opacity in Slate. A subsequent flat-mask experiment was not adopted; the original retained more convincing contours. The source is copied unchanged from the generator output. No post-processing script edits its pixels.

Generation brief: create two original, anatomically convincing adult football athletes in short sleeve jerseys, shorts, socks and boots, isolated on true transparent alpha. Attacker at left facing right, balanced controlled crossing/pass motion; defender at right facing left, bent-knee lateral interception. Keep full bodies and balls, distinct poses and natural clothing/hand/boot contours. White and light gray only, suitable for steel-blue tinting. No text, logos, stadium, grass, floor, shadow, outline or identifiable athlete. Avoid stick figures and childish icon anatomy.

Import with `Scripts/ImportResolutionTheaterAssets.py` using the existing Unreal Python import workflow. It writes only `/Game/UI/ResolutionTheater/T_Theater_Athletes`, preserving alpha with UI RGBA compression, no mipmaps and no streaming. `UFMCodexLocalMatchScreenWidget` holds the texture as a reflected CDO reference for lifetime and cook discovery. The production widget uses separate UV islands with aspect-preserving fit; it never loads from this source PNG or an absolute machine path.

Attack UV: (0, .09)–(.45, .87). Defense UV: (.46, .17)–(1, .87). Each is confined to its outer decoration column, independently of the content column. This is a local High Cross Theater asset, not a change to the shared Match Board or roll visual family.
