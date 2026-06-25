import os
import bpy


# -----------------------------
# Edit these values before run.
# -----------------------------

INPUT_FBX = r"D:\UE素材库\AS_Combo_Attack_All_Seq_short.fbx"
OUTPUT_FBX = r"D:\UE素材库\AS_Combo_Attack_All_Seq_short_rootmotion_edit.fbx"

# This animation stores the real motion on pelvis, based on diagnostics.
BONE_NAME = "pelvis"

# Usually UE FBX animation curves are in centimeters. 250 means 250 cm.
TARGET_DISTANCE = 250.0

# If you prefer multiplying existing motion, set MODE = "scale".
# Examples: SCALE = 0.5 halves the motion, SCALE = 2.0 doubles it.
MODE = "target_distance"
SCALE = 0.5

# Axis to edit in Blender curve index: 0=X, 1=Y, 2=Z.
AXIS_INDEX = 0

# Keeps the first keyed value unchanged and scales offsets from it.
KEEP_START_LOCATION = True


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()


def import_fbx(path):
    if not os.path.exists(path):
        raise RuntimeError(f"Input FBX not found: {path}")

    bpy.ops.import_scene.fbx(filepath=path)

    armatures = [obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE"]
    if not armatures:
        raise RuntimeError("Imported FBX has no Armature.")

    return armatures[0]


def get_actions(armature):
    actions = []

    if armature.animation_data:
        if armature.animation_data.action:
            actions.append(armature.animation_data.action)

        for track in getattr(armature.animation_data, "nla_tracks", []):
            for strip in getattr(track, "strips", []):
                action = getattr(strip, "action", None)
                if action and action not in actions:
                    actions.append(action)

    if not actions:
        actions = list(bpy.data.actions)

    if not actions:
        raise RuntimeError("No animation Action found after FBX import.")

    return actions


def get_fcurves(action, armature):
    if hasattr(action, "fcurves"):
        return list(action.fcurves)

    curves = []
    active_slot = getattr(armature.animation_data, "action_slot", None) if armature.animation_data else None
    slots = [active_slot] if active_slot else []
    if hasattr(action, "slots"):
        slots.extend(slot for slot in action.slots if slot not in slots)

    if hasattr(action, "layers"):
        for layer in action.layers:
            for strip in getattr(layer, "strips", []):
                if hasattr(strip, "channelbag"):
                    for slot in slots:
                        try:
                            channelbag = strip.channelbag(slot)
                        except TypeError:
                            channelbag = None

                        if channelbag and hasattr(channelbag, "fcurves"):
                            curves.extend(channelbag.fcurves)

                if hasattr(strip, "channelbags"):
                    for channelbag in strip.channelbags:
                        if hasattr(channelbag, "fcurves"):
                            curves.extend(channelbag.fcurves)

    unique = []
    seen = set()
    for curve in curves:
        key = curve.as_pointer()
        if key not in seen:
            seen.add(key)
            unique.append(curve)

    return unique


def find_curve(action, armature, bone_name, axis_index):
    data_path = f'pose.bones["{bone_name}"].location'
    for curve in get_fcurves(action, armature):
        if curve.data_path == data_path and curve.array_index == axis_index:
            return curve
    return None


def curve_distance(curve):
    points = curve.keyframe_points
    if len(points) < 2:
        return 0.0

    distance = 0.0
    previous = points[0].co.y
    for point in points[1:]:
        current = point.co.y
        distance += abs(current - previous)
        previous = current

    return distance


def curve_end_to_end(curve):
    points = curve.keyframe_points
    if len(points) < 2:
        return 0.0
    return points[-1].co.y - points[0].co.y


def scale_curve(curve, factor):
    points = curve.keyframe_points
    if not points:
        return

    origin = points[0].co.y if KEEP_START_LOCATION else 0.0
    for point in points:
        point.co.y = origin + (point.co.y - origin) * factor
        point.handle_left.y = origin + (point.handle_left.y - origin) * factor
        point.handle_right.y = origin + (point.handle_right.y - origin) * factor

    curve.update()


def select_imported_objects():
    bpy.ops.object.select_all(action="DESELECT")
    for obj in bpy.context.scene.objects:
        if obj.type in {"ARMATURE", "MESH"}:
            obj.select_set(True)


def export_fbx(path):
    select_imported_objects()
    os.makedirs(os.path.dirname(path), exist_ok=True)
    bpy.ops.export_scene.fbx(
        filepath=path,
        use_selection=True,
        object_types={"ARMATURE", "MESH"},
        bake_anim=True,
        bake_anim_use_all_bones=True,
        bake_anim_use_nla_strips=False,
        bake_anim_use_all_actions=False,
        add_leaf_bones=False,
        apply_scale_options="FBX_SCALE_NONE",
    )


def main():
    clear_scene()
    armature = import_fbx(INPUT_FBX)

    if BONE_NAME not in armature.pose.bones:
        names = ", ".join(bone.name for bone in armature.pose.bones[:40])
        raise RuntimeError(f"Bone '{BONE_NAME}' not found. First bones: {names}")

    edited = False
    for action in get_actions(armature):
        curve = find_curve(action, armature, BONE_NAME, AXIS_INDEX)
        if not curve:
            continue

        before_path_distance = curve_distance(curve)
        before_end_to_end = curve_end_to_end(curve)
        if before_path_distance <= 0.0001:
            print(f"Skip action '{action.name}': zero motion on {BONE_NAME} axis {AXIS_INDEX}.")
            continue

        if MODE == "scale":
            factor = SCALE
        elif MODE == "target_distance":
            factor = TARGET_DISTANCE / before_path_distance
        else:
            raise RuntimeError('MODE must be "target_distance" or "scale".')

        scale_curve(curve, factor)
        action.update_tag()

        after_path_distance = curve_distance(curve)
        after_end_to_end = curve_end_to_end(curve)
        print(
            f"Edited action '{action.name}', bone '{BONE_NAME}', axis {AXIS_INDEX}: "
            f"path {before_path_distance:.4f} -> {after_path_distance:.4f}, "
            f"end {before_end_to_end:.4f} -> {after_end_to_end:.4f}, factor {factor:.6f}"
        )
        edited = True

    if not edited:
        raise RuntimeError(
            f"No editable curve found for bone '{BONE_NAME}' axis {AXIS_INDEX}. "
            "Try AXIS_INDEX = 1 or 2."
        )

    bpy.context.scene.frame_set(bpy.context.scene.frame_current)
    bpy.context.view_layer.update()
    export_fbx(OUTPUT_FBX)
    print(f"Exported edited FBX: {OUTPUT_FBX}")


if __name__ == "__main__":
    main()
