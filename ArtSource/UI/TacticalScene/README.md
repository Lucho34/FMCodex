# LongShot scene textures

Original production textures generated with the built-in imagegen tool for the user's requested scene polish. These are independent of the deployment pitch and of the generated composition references. No text, brand or gameplay facts are baked into them.

| Source | Package | Import |
|---|---|---|
| T_TacticalScene_Turf.png | /Game/UI/TacticalScene/T_TacticalScene_Turf | sRGB, trilinear, simple-average mipmaps, maximum 1024 |
| T_TacticalScene_Ball.png | /Game/UI/TacticalScene/T_TacticalScene_Ball | RGBA, sRGB, trilinear, simple-average mipmaps, maximum 256 |

Import with `Scripts/ImportTacticalSceneAssets.py` through Unreal's Python commandlet. Only these two named assets are imported/saved. Runtime loading is cached by the persistent scene node, and `Config/DefaultGame.ini` includes this isolated directory for cooking. Original PNGs remain untouched; size/filtering are import settings. Lighting, mowing bands, football shadow, routes, field markings, goal and labels remain native live presentation.

Both source images are 1254 × 1254. UE 5.3 import pads them to 2048 × 2048 so mip generation and the maximum texture size take effect. Runtime samples only UV 0–1254/2048, excluding the padded region; the ball retains real transparency. If sources are regenerated at another extent, update this crop together with the import settings. The crowd strip reuses the existing stadium texture with soft top/end fades; no new crowd or sponsor image is introduced.

## Turf prompt (verbatim)

Production texture asset for a realistic night football pitch in a premium PC game, NOT a UI mockup. A single perfectly top-down ORTHOGRAPHIC overhead close-cropped turf texture covering an approximately 15x15 meter patch of finely manicured professional football grass. Square image. Extremely fine short-blade detail appropriate to a stadium elevated broadcast camera, organic subtle mottling and sparse minor scuffs, coherent fine texture, low contrast, muted natural cool forest green, approximately sRGB base #376447. Soft neutral EVEN lighting, no baked directional shadows or bright spots. No mowing stripes: the game will add mowing stripes itself. Avoid yellow/lime/neon green, oversharpened crunchy speckles, oversized grass blades, carpet fibers or noisy grain. Seamless tileable edges, uniform texture scale throughout, no perspective. Fill the entire square edge to edge with grass only. NO pitch markings, lines, goals, footballs, people, text, stadium, border, logo, or watermark. This asset is the actual grass material under live geometric pitch markings.

## Ball prompt (verbatim)

Production game sprite asset: ONE classic black-and-white pentagon-panel soccer ball isolated on a genuinely transparent background. Square canvas, ball centered and taking 84 percent of canvas width. Photorealistic polished football with off-white slightly textured leather hexagons and charcoal black pentagons, thin dark stitched seams. Front three-quarter view with instantly recognizable black pentagon at center-left, multiple surrounding black panels distributed around the curvature. Soft cool stadium lighting from upper left, gentle white highlight, shaded lower-right hemisphere, clear round silhouette and convincing spherical volume. Strong readable panel pattern suitable for displaying at 24-36 pixels in a strategy game. No external cast shadow; shadow is drawn separately by the engine. No contact plane, no grass, no background color, no stadium, no text, no brand, no watermark, no border, no glows, no additional objects. Exactly one standalone ball with clean alpha edges.
