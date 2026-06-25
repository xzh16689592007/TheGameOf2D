import bpy


# -----------------------------
# Edit these values before run.
# -----------------------------

# Edits the currently loaded Blender scene only. No FBX import/export.

# Usually your FBX stores visible motion on pelvis.
BONE_NAME = "pelvis"

# 0 = X, 1 = Y, 2 = Z.
AXIS_INDEX = 0

# "set_end": set last key offset from first key to TARGET_END_OFFSET.
# "set_path": set total path distance to TARGET_PATH_DISTANCE.
# "scale": multiply current offsets by SCALE.
MODE = "scale"

# UE FBX curves are usually centimeters. 250 means 250 cm.
TARGET_END_OFFSET = 250.0
TARGET_PATH_DISTANCE = 250.0
SCALE = 0.5

# If True, use selected pose bone instead of BONE_NAME.
USE_SELECTED_POSE_BONE = False

# If True, creates a backup copy of the Action before editing.
MAKE_ACTION_BACKUP = True


def get_armature():
    obj = bpy.context.object
    if obj and obj.type == "ARMATURE":
        return obj

    for selected in bpy.context.selected_objects:
        if selected.type == "ARMATURE":
            return selected

    for obj in bpy.context.scene.objects:
        if obj.type == "ARMATURE":
            return obj

    raise RuntimeError("No Armature found in the current scene.")


def get_bone_name(armature):
    if USE_SELECTED_POSE_BONE and bpy.context.mode == "POSE" and bpy.context.active_pose_bone:
        return bpy.context.active_pose_bone.name

    if BONE_NAME in armature.pose.bones:
        return BONE_NAME

    for fallback in ("root", "Root", "pelvis", "Pelvis"):
        if fallback in armature.pose.bones:
            print(f"Bone '{BONE_NAME}' not found. Using '{fallback}' instead.")
            return fallback

    names = ", ".join(bone.name for bone in armature.pose.bones[:40])
    raise RuntimeError(f"Bone '{BONE_NAME}' not found. First bones: {names}")


def action_candidates(armature):
    result = []
    ad = armature.animation_data
    if ad:
        if ad.action:
            result.append(ad.action)
        for track in getattr(ad, "nla_tracks", []):
            for strip in getattr(track, "strips", []):
                if strip.action and strip.action not in result:
                    result.append(strip.action)
    return result or list(bpy.data.actions)


def get_fcurves(action, armature):
    if hasattr(action, "fcurves"):
        return list(action.fcurves)

    curves = []
    ad = armature.animation_data
    active_slot = getattr(ad, "action_slot", None) if ad else None
    slots = [active_slot] if active_slot else []
    if hasattr(action, "slots"):
        slots.extend(slot for slot in action.slots if slot not in slots)

    for layer in getattr(action, "layers", []):
        for strip in getattr(layer, "strips", []):
            if hasattr(strip, "channelbag"):
                for slot in slots:
                    try:
                        channelbag = strip.channelbag(slot)
                    except TypeError:
                        channelbag = None
                    if channelbag and hasattr(channelbag, "fcurves"):
                        curves.extend(channelbag.fcurves)
            for channelbag in getattr(strip, "channelbags", []):
                if hasattr(channelbag, "fcurves"):
                    curves.extend(channelbag.fcurves)

    unique = []
    seen = set()
    for curve in curves:
        pointer = curve.as_pointer()
        if pointer not in seen:
            seen.add(pointer)
            unique.append(curve)
    return unique


def find_curve(action, armature, bone_name, axis_index):
    path = f'pose.bones["{bone_name}"].location'
    for curve in get_fcurves(action, armature):
        if curve.data_path == path and curve.array_index == axis_index:
            return curve
    return None


def path_distance(curve):
    points = curve.keyframe_points
    if len(points) < 2:
        return 0.0
    total = 0.0
    prev = points[0].co.y
    for point in points[1:]:
        total += abs(point.co.y - prev)
        prev = point.co.y
    return total


def end_offset(curve):
    points = curve.keyframe_points
    if len(points) < 2:
        return 0.0
    return points[-1].co.y - points[0].co.y


def edit_curve(curve):
    points = curve.keyframe_points
    if len(points) < 2:
        raise RuntimeError("Curve has fewer than 2 keys.")

    start_value = points[0].co.y
    before_end = end_offset(curve)
    before_path = path_distance(curve)

    if MODE == "scale":
        factor = SCALE
    elif MODE == "set_end":
        if abs(before_end) <= 0.0001:
            raise RuntimeError("Current end offset is zero. Use MODE='scale' or MODE='set_path'.")
        factor = TARGET_END_OFFSET / before_end
    elif MODE == "set_path":
        if before_path <= 0.0001:
            raise RuntimeError("Current path distance is zero. Use another axis or bone.")
        factor = TARGET_PATH_DISTANCE / before_path
    else:
        raise RuntimeError("MODE must be 'scale', 'set_end', or 'set_path'.")

    for point in points:
        point.co.y = start_value + (point.co.y - start_value) * factor
        point.handle_left.y = start_value + (point.handle_left.y - start_value) * factor
        point.handle_right.y = start_value + (point.handle_right.y - start_value) * factor

    curve.update()
    return factor, before_end, end_offset(curve), before_path, path_distance(curve)


def main():
    armature = get_armature()
    bone_name = get_bone_name(armature)

    edited = False
    for action in action_candidates(armature):
        curve = find_curve(action, armature, bone_name, AXIS_INDEX)
        if not curve:
            continue

        if MAKE_ACTION_BACKUP and not action.name.endswith("_backup"):
            action.copy().name = action.name + "_backup"

        factor, before_end, after_end, before_path, after_path = edit_curve(curve)
        action.update_tag()
        edited = True

        print(
            f"Edited CURRENT scene action '{action.name}', bone '{bone_name}', axis {AXIS_INDEX}: "
            f"factor={factor:.6f}, end {before_end:.4f}->{after_end:.4f}, "
            f"path {before_path:.4f}->{after_path:.4f}"
        )

    if not edited:
        raise RuntimeError(
            f"No current Action curve found for bone '{bone_name}' axis {AXIS_INDEX}. "
            "Try AXIS_INDEX=1 or 2, or set USE_SELECTED_POSE_BONE=True."
        )

    current_frame = bpy.context.scene.frame_current
    bpy.context.scene.frame_set(current_frame)
    bpy.context.view_layer.update()

    if hasattr(bpy.ops.wm, "redraw_timer"):
        bpy.ops.wm.redraw_timer(type="DRAW_WIN_SWAP", iterations=1)


if __name__ == "__main__":
    main()
