"""Generate deterministic purpose-sized portrait sources from canonical Masters."""

from __future__ import annotations

import hashlib
import io
import json
import os
from pathlib import Path
import sys

sys.dont_write_bytecode = True

from PIL import Image, ImageDraw, ImageFilter, __version__ as PILLOW_VERSION

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from SharedPortraitImportCatalog import (  # noqa: E402
    is_canonical, expand_runtime_entries, runtime_size, runtime_role, runtime_asset_name,
    hand_composition, selected_runtime_roles,
    resolved_crop, family_provenance_path, crop_metadata_hash,
    MASTER_SIZE,
    RUNTIME_SIZE,
    asset_path,
    load_catalog,
    master_path,
    provenance_path,
    runtime_derivative_path,
    select_entries,
    validate_source_png,
)


GENERATOR_VERSION = 1
RESAMPLING_CONTRACT = "Pillow.Resampling.LANCZOS"
ENCODER_CONTRACT = "PNG RGB compress_level=9 optimize=false metadata=none"


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def sha256_file(path: Path) -> str:
    return sha256_bytes(path.read_bytes())


def extract_hand_subject(source, crop):
 """Offline trimap extraction only. The original Master supplies every person pixel."""
 import cv2
 import numpy as np
 from importlib.metadata import version
 if version("opencv-python-headless") != "4.10.0.84" or np.__version__ != "1.24.1":
  raise RuntimeError("Hand extraction requires Scripts/PlayerPortraitBuildRequirements.txt")
 cv2.setNumThreads(1);cv2.setRNGSeed(0)
 small=source.resize((512,768),Image.Resampling.LANCZOS)
 region=Image.new('L',(512,768),cv2.GC_BGD);d=ImageDraw.Draw(region)
 d.ellipse((92,26,430,458),fill=cv2.GC_PR_FGD)
 d.polygon([(192,265),(337,265),(512,400),(512,768),(0,768),(0,420)],fill=cv2.GC_PR_FGD)
 d.ellipse((210,148,300,327),fill=cv2.GC_FGD)
 d.rectangle((122,466,411,710),fill=cv2.GC_FGD)
 y,h=crop[1],crop[3]
 d.ellipse((225,round((y+h*.07)*768),288,round((y+h*.16)*768)),fill=cv2.GC_FGD)
 mask=np.array(region);cv2.grabCut(np.array(small),mask,None,np.zeros((1,65)),np.zeros((1,65)),6,cv2.GC_INIT_WITH_MASK)
 binary=np.where((mask==cv2.GC_FGD)|(mask==cv2.GC_PR_FGD),255,0).astype('uint8')
 _,labels=cv2.connectedComponents(binary,8);binary=(labels==labels[220,256]).astype('uint8')*255
 flood=binary.copy();cv2.floodFill(flood,None,(0,0),255);binary=cv2.bitwise_or(binary,cv2.bitwise_not(flood))
 return Image.fromarray(binary).filter(ImageFilter.GaussianBlur(.5))

def hand_background(size):
 w,h=size;im=Image.new('RGB',size);px=im.load()
 for y in range(h):
  for x in range(w):
   g=max(0,1-((x-w*.48)/(w*.68))**2-((y-h*.32)/(h*.8))**2)**2
   grain=((x*37+y*17)%13-6)/9
   px[x,y]=tuple(round(base+(1-y/h)*top+g*gain+grain) for base,top,gain in [(5,2,3),(15,3,8),(26,4,11)])
 return im

def compose_balanced_bust(source, size, box):
 crop=[box[0]/1024,box[1]/1536,(box[2]-box[0])/1024,(box[3]-box[1])/1536]
 mask=extract_hand_subject(source,crop);x,y,w,h=crop;box=(x*1024,y*1536,(x+w)*1024,(y+h)*1536);ow=round(size[1]*(box[2]-box[0])/(box[3]-box[1]))
 result=hand_background(size);small=source.resize((ow,size[1]),Image.Resampling.LANCZOS,box=box);alpha=mask.resize((ow,size[1]),Image.Resampling.LANCZOS,box=tuple(c/2 for c in box))
 if ow > size[0]:
  raise RuntimeError("Balanced Hand bust exceeds its canvas")
 result.paste(small,((size[0]-ow)//2,0),alpha)
 return result


def encode_runtime_derivative(master: Path, size=RUNTIME_SIZE, crop=None, composition="CropOnly_v1") -> bytes:
    validate_source_png(master, MASTER_SIZE)
    with Image.open(master) as source:
        source.load()
        if source.mode != "RGB" or source.size != MASTER_SIZE:
            raise RuntimeError(
                f"Shared Portrait Master must decode as opaque RGB "
                f"{MASTER_SIZE[0]}x{MASTER_SIZE[1]}: {master}"
            )
        box = None
        if crop is not None:
            x, y, w, h = crop
            box = (x*MASTER_SIZE[0], y*MASTER_SIZE[1], (x+w)*MASTER_SIZE[0], (y+h)*MASTER_SIZE[1])
        if composition == "BalancedBust_v2":
            derivative = compose_balanced_bust(source, size, box)
        else:
            derivative = source.resize(size, Image.Resampling.LANCZOS, box=box)
        if derivative.mode != "RGB" or derivative.size != size:
            raise RuntimeError(f"Invalid runtime derivative in memory: {master}")
        stream = io.BytesIO()
        derivative.save(
            stream,
            format="PNG",
            optimize=False,
            compress_level=9,
        )
        return stream.getvalue()


def write_if_changed(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.is_file() and path.read_bytes() == data:
        return
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_bytes(data)
    os.replace(temporary, path)


def load_existing_provenance(path: Path) -> dict[str, dict[str, object]]:
    if not path.is_file():
        return {}
    document = json.loads(path.read_text(encoding="utf-8"))
    if document.get("schemaVersion") != 1:
        raise RuntimeError(f"Unsupported Shared Portrait provenance: {path}")
    entries = document.get("entries", [])
    return {str(entry["playerKey"]): entry for entry in entries}


def generate_selected(project_root: Path) -> list[dict[str, object]]:
    catalog = load_catalog(project_root)
    selected = select_entries(catalog)
    records_by_key = load_existing_provenance(provenance_path(project_root))

    for entry in selected:
        if is_canonical(entry):
            continue
        master = master_path(project_root, entry)
        derivative = runtime_derivative_path(project_root, entry)
        if not master.is_file():
            raise RuntimeError(f"Shared Portrait Master is missing: {master}")

        first_encoding = encode_runtime_derivative(master)
        second_encoding = encode_runtime_derivative(master)
        if first_encoding != second_encoding:
            raise RuntimeError(
                f"Runtime derivative generation is not deterministic: {master}"
            )
        write_if_changed(derivative, first_encoding)
        validate_source_png(derivative, RUNTIME_SIZE)

        runtime_asset_path = asset_path(entry)
        record: dict[str, object] = {
            "playerKey": entry["playerKey"],
            "masterSourcePath": master.relative_to(project_root).as_posix(),
            "masterDimensions": list(MASTER_SIZE),
            "masterSha256": sha256_file(master),
            "runtimeDerivativePath": derivative.relative_to(project_root).as_posix(),
            "runtimeDerivativeDimensions": list(RUNTIME_SIZE),
            "runtimeDerivativeSha256": sha256_bytes(first_encoding),
            "runtimeAssetPath": (
                f"{runtime_asset_path}.{entry['assetName']}"
            ),
            "generatorVersion": GENERATOR_VERSION,
            "pillowVersion": PILLOW_VERSION,
            "resampling": RESAMPLING_CONTRACT,
            "encoder": ENCODER_CONTRACT,
            "visualStatus": entry.get("visualStatus", "UNREVIEWED"),
        }
        if entry.get("replacementHistory"):
            record["replacementHistory"] = entry["replacementHistory"]
        records_by_key[entry["playerKey"]] = record
        print(
            "FMCODEX_SHARED_PORTRAIT_DERIVATIVE "
            f"player_key={entry['playerKey']} "
            f"master_sha256={record['masterSha256']} "
            f"derivative_sha256={record['runtimeDerivativeSha256']} "
            f"dimensions={RUNTIME_SIZE[0]}x{RUNTIME_SIZE[1]} "
            "rgb=true opaque=true deterministic=true"
        )

    ordered_records = [
        records_by_key[entry["playerKey"]]
        for entry in catalog
        if entry["playerKey"] in records_by_key
    ]
    provenance_document = {
        "schemaVersion": 1,
        "description": (
            "Generated Shared Portrait runtime-source provenance; Art Masters "
            "remain the source of truth."
        ),
        "entries": ordered_records,
    }
    provenance_bytes = (
        json.dumps(
            provenance_document,
            ensure_ascii=False,
            indent=2,
        )
        + "\n"
    ).encode("utf-8")
    if any(not is_canonical(entry) for entry in selected):
        write_if_changed(provenance_path(project_root), provenance_bytes)
    canonical_records = generate_canonical_selected(project_root, [e for e in selected if is_canonical(e)])
    if canonical_records:
        return canonical_records
    return ordered_records


def generate_canonical_selected(project_root, selected):
    if not selected:
        return []
    if PILLOW_VERSION != "9.4.0":
        raise RuntimeError("Canonical portrait encoder requires pinned Pillow 9.4.0")
    path = family_provenance_path(project_root)
    existing = json.loads(path.read_text(encoding="utf-8"))["entries"] if path.exists() else []
    records = {r["playerKey"]:r for r in existing}
    pending = []
    roles = selected_runtime_roles()
    for entry in selected:
        master = master_path(project_root, entry)
        if sha256_file(master) != entry["masterSha256"].upper():
            raise RuntimeError(f"Master changed without approved manifest revision: {master}")
        record = {"playerKey":entry["playerKey"], "masterSourcePath":entry["masterSourcePath"],
                  "masterDimensions":list(MASTER_SIZE), "masterSha256":sha256_file(master),
                  "masterRevision":entry["masterRevision"], "cropMetadataSha256":crop_metadata_hash(entry),
                  "compositionProfile":entry["compositionProfile"], "generatorVersion":5,
                  "pillowVersion":PILLOW_VERSION, "resampling":RESAMPLING_CONTRACT,
                  "encoder":ENCODER_CONTRACT, "visualStatus":"PER-ROLE ACCEPTANCE ONLY",
                  "sourceProvenance":entry.get("sourceProvenance",{}), "roles":{}}
        previous = records.get(entry["playerKey"])
        previous_roles = previous.get("roles", {}) if previous else {}
        for frozen_role, frozen in previous_roles.items():
            if frozen_role in roles:
                continue
            frozen_entry = dict(entry, runtimeRole=frozen_role)
            if (frozen_role not in ("Shared", "Hand", "Full")
                or previous["masterSha256"] != record["masterSha256"]
                or previous["masterRevision"] != record["masterRevision"]
                or previous["masterSourcePath"] != record["masterSourcePath"]
                or frozen["cropRect"] != resolved_crop(frozen_entry)):
                raise RuntimeError(f"Partial generation would stale frozen {frozen_role}; select affected roles explicitly")
            frozen_path = runtime_derivative_path(project_root, frozen_entry)
            validate_source_png(frozen_path, runtime_size(frozen_entry))
            if (frozen["dimensions"] != list(runtime_size(frozen_entry))
                or frozen["runtimeDerivativePath"] != frozen_path.relative_to(project_root).as_posix()
                or frozen["runtimeAssetPath"] != asset_path(frozen_entry)+"."+runtime_asset_name(frozen_entry)
                or frozen.get("importRecipe") != "DesktopBC7OpaqueSharpen1_v1"
                or sha256_file(frozen_path) != frozen["runtimeDerivativeSha256"]):
                raise RuntimeError(f"Invalid frozen {frozen_role} derivative provenance")
            if frozen_role == "Hand" and (
                frozen.get("handCompositionProfile", "CropOnly_v1") != hand_composition(entry)
                or frozen.get("generatorSha256") != sha256_file(Path(__file__))):
                raise RuntimeError("Partial generation would stale frozen Hand recipe")
            record["roles"][frozen_role] = frozen
        for role_entry in expand_runtime_entries([entry], roles):
            role=runtime_role(role_entry);size=runtime_size(role_entry);crop=resolved_crop(role_entry)
            composition = hand_composition(entry) if role == "Hand" else "CropOnly_v1"
            data=encode_runtime_derivative(master,size,crop,composition)
            if data != encode_runtime_derivative(master,size,crop,composition):
                raise RuntimeError(f"Nondeterministic {role} derivative")
            dest=runtime_derivative_path(project_root,role_entry)
            pending.append((dest,data))
            x,y,w,h=crop
            record["roles"][role]={"cropRect":crop,"sourcePixelBox":[x*1024,y*1536,(x+w)*1024,(y+h)*1536],
                "dimensions":list(size),"runtimeDerivativePath":dest.relative_to(project_root).as_posix(),
                "runtimeDerivativeSha256":sha256_bytes(data),"bytes":len(data),
                "runtimeAssetPath":asset_path(role_entry)+"."+runtime_asset_name(role_entry),
                "importRecipe":"DesktopBC7OpaqueSharpen1_v1"}
            if role == "Hand":
                record["roles"][role].update({"handCompositionProfile":composition,
                    "generatorSha256":sha256_file(Path(__file__)),"generatorVersion":5,
                    "pillowVersion":PILLOW_VERSION,"reframing":"proportional bust framing; original Master pixels; offline seeded subject extraction" if composition == "BalancedBust_v2" else "direct aspect-preserving Master crop",
                    "foregroundExtraction":"OpenCV 4.10.0.84 GrabCut; NumPy 1.24.1; seed=0; threads=1; 512x768 analysis; 6 iterations" if composition == "BalancedBust_v2" else "none"})
            old_role = previous_roles.get(role, {})
            unchanged = (previous is not None and previous["masterSha256"] == record["masterSha256"]
                and previous["masterRevision"] == record["masterRevision"]
                and old_role.get("runtimeDerivativeSha256") == record["roles"][role]["runtimeDerivativeSha256"]
                and old_role.get("cropRect") == crop)
            record["roles"][role]["visualStatus"] = old_role.get("visualStatus", "PENDING USER PIE") if unchanged else "PENDING USER PIE"
        record["roleStatus"] = {role: record["roles"][role].get("visualStatus", "PENDING USER PIE")
            if role in record["roles"] else "DEFERRED" for role in ("Hand", "Shared", "Full")}
        records[entry["playerKey"]]=record
    # Validate the complete selection before publishing any generated input.
    for dest,data in pending:
        write_if_changed(dest,data)
    ordered=[records[key] for key in sorted(records)]
    write_if_changed(path,(json.dumps({"schemaVersion":2,"entries":ordered},indent=2,ensure_ascii=False)+"\n").encode("utf-8"))
    print(f"FMCODEX_CANONICAL_PLAYER_DERIVATIVES=PASS players={len(selected)} textures={len(pending)}")
    return ordered


def main() -> None:
    project_root = SCRIPT_DIR.parent
    records = generate_selected(project_root)
    selected_count = len(select_entries(load_catalog(project_root)))
    print(
        "FMCODEX_SHARED_PORTRAIT_DERIVATIVE_GENERATION=PASS "
        f"selected={selected_count} provenance_records={len(records)}"
    )


if __name__ == "__main__":
    main()
