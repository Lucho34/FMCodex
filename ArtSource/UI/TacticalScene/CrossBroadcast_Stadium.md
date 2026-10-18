# Cross broadcast stadium plate

Authored with built-in imagegen from the user's approved visual direction (2026-10-17). This is a Cross-only static environment: grass, advertising, spectators, stands and empty goal. Portraits, names, ball, tactical trajectories, scoreboard and controls are rendered live by UE. User requested removal of the lower-left foreground equipment. Visual USER PIE remains pending.

Source: `T_CrossBroadcast_Stadium.png`. Import using `Scripts/ImportCrossBroadcastBackdrop.py` to `/Game/UI/TacticalScene/T_CrossBroadcast_Stadium`. No existing match-shell assets are replaced.

Generation prompt:

Edit the supplied reference game screenshot into a CLEAN PRODUCTION ENVIRONMENT PLATE for the same game. The input is a reference/edit target, preserve the stadium/environment as faithfully as possible.

Output ONLY the upper football stadium/pitch region of the screenshot, cropped at the exact horizontal boundary where the bottom navy UI begins. Target an ultrawide roughly 2.46:1 landscape image, 1920 x 780 or comparable high resolution. Do not keep the bottom HUD; do not extend or invent a different scene. The environment must match the reference: identical camera angle and perspective, green detailed grass and mowing texture, white penalty box and arc markings in the same positions, white realistic goal on the right, dark navy LED advertising boards with the same "FMCODEX" lettering and teal diagonal stripe graphics, dense photographed spectators, gray railings and the same tiered grandstand structure, restrained night stadium illumination.

REMOVE ALL overlay elements and inpaint their underlying environment cleanly: top left high-cross title and icon, top center scoreboard, every player portrait, name plate, role label, blue/red halo, floor ring, teal trajectory, football, landing ellipse, red/cyan arrows, all game UI. There are NO athletes on the pitch, NO footballs, NO UI and NO tactical lines in this asset. Preserve the real painted white pitch markings only.

ALSO REMOVE the blurry foreground photographers / camera equipment / dark figures in the LOWER LEFT corner and any foreground obstruction in the lower right corner. The user specifically dislikes these artifacts. Replace these areas with continuous clean grass at the correct perspective and focus; no camera gear, silhouettes or blurry objects along the bottom.

This is precise environment extraction/inpainting. Preserve the reference grass color/texture, boards, crowd, stands, camera, goal position and lighting, not a new redesign. Goal geometry must remain coherent, with an empty goalmouth. Crisp production-quality background for live game UI placed later. No watermark.
