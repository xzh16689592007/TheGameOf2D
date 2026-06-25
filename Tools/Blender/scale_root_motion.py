import bpy
from mathutils import Vector


# -----------------------------
# Edit these values before run.
# -----------------------------

# "target_distance": set total root translation length to TARGET_DISTANCE.
# "scale": multiply existing root translation by SCALE.
MODE = "target_distance"

# Root motion should usually be edited on the real root bone, not pelvis.
# UE skeleton helper bones such as IK/weapon bones often do not follow pelvis.
ROOT_BONE_NAME = "root"

# If ROOT_BONE_NAME is not found, these names are tried in order.
FALLBACK_ROOT_BONE_NAMES = ["Root", "root", "pelvis", "Pelvis"]

# If True, the script picks the bone with the largest keyed translation on AXES.
# Useful for UE FBX files where the visible root bone has no location keys.
AUTO_DETECT_MOVING_BONE = True

# "auto": edit the biggest moving target, including Armature object location and pose bone location.
# "bone": edit pose bone location only.
# "object": edit Armature object location only.
TARGET_KIND = "auto"

# Prints the largest location curves before editing.
PRINT_DIAGNOSTICS = True

# Blender units. If your UE import uses 1 Blender unit = 1 meter, 2.5 means 250 cm in UE.
TARGET_DISTANCE = 2.5

# Used only when MODE = "scale".
SCALE = 0.5

# Which axes to edit. For UE-style forward motion imported into Blender this is often X.
AXES = {"X": True, "Y": False, "Z": False}

# If selected AXES has zero motion, try X/Y/Z and use the axis with the most motion.
AUTO_DETECT_AXES_IF_ZERO = True

# "path_length": measures all frame-to-frame movement, good for lunges or curves.
# "end_to_end": measures only final position minus first position.
MOTION_MEASURE = "path_length"

# If True, keeps first keyed root location unchanged and edits offsets from that frame.
KEEP_START_LOCATION = True


def get_selected_armature():
    obj = bpy.context.object
    if obj and obj.type == "ARMATURE":
        return obj

    for selected in bpy.context.selected_objects:
        if selected.type == "ARMATURE":
            return selected

    raise RuntimeError("Select the Armature object before running this script.")


def get_action(armature):
    if not armature.animation_data:
        raise RuntimeError(f"Armature '{armature.name}' has no animation data.")

    actions = []
    if armature.animation_data.action:
        actions.append(armature.animation_data.action)

    for track in getattr(armature.animation_data, "nla_tracks", []):
        for strip in getattr(track, "strips", []):
            action = getattr(strip, "action", None)
            if action and action not in actions:
                actions.append(action)

    if not actions:
        raise RuntimeError(f"Armature '{armature.name}' has no active Action or NLA strip Action.")

    for action in actions:
        if get_action_fcurves(action, armature):
            if action != armature.animation_data.action:
                print(f"Using NLA strip action '{action.name}'.")
            return action

    return actions[0]


def get_object_location_curves(action, armature):
    all_curves = get_action_fcurves(action, armature)
    return {curve.array_index: curve for curve in all_curves if curve.data_path == "location"}


def location_data_path(root_bone_name):
    return f'pose.bones["{root_bone_name}"].location'


def get_action_fcurves(action, armature):
    if hasattr(action, "fcurves"):
        return list(action.fcurves)

    curves = []
    active_slot = getattr(armature.animation_data, "action_slot", None)
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

    unique_curves = []
    seen = set()
    for curve in curves:
        key = curve.as_pointer()
        if key not in seen:
            seen.add(key)
            unique_curves.append(curve)

    return unique_curves


def find_location_curves(action, armature, root_bone_name):
    path = location_data_path(root_bone_name)
    all_curves = get_action_fcurves(action, armature)
    curves = {curve.array_index: curve for curve in all_curves if curve.data_path == path}
    if not curves:
        raise RuntimeError(
            f"No location keyframes found for root bone '{root_bone_name}' in action '{action.name}'."
        )
    return curves


def find_best_moving_bone(action, armature, preferred_bone_name, indices):
    all_curves = get_action_fcurves(action, armature)
    curves_by_bone = {}
    detection_indices = [0, 1, 2] if AUTO_DETECT_AXES_IF_ZERO else indices

    for curve in all_curves:
        path = curve.data_path
        prefix = 'pose.bones["'
        suffix = '"].location'
        if not path.startswith(prefix) or not path.endswith(suffix):
            continue

        bone_name = path[len(prefix):-len(suffix)]
        if curve.array_index not in detection_indices:
            continue

        curves_by_bone.setdefault(bone_name, {})[curve.array_index] = curve

    if preferred_bone_name in curves_by_bone:
        preferred_curves = curves_by_bone[preferred_bone_name]
        if measure_motion_distance(preferred_curves, detection_indices) > 0.0001:
            return preferred_bone_name, preferred_curves

    best_bone_name = None
    best_curves = None
    best_distance = 0.0

    for bone_name, curves in curves_by_bone.items():
        distance = measure_motion_distance(curves, detection_indices)
        if distance > best_distance:
            best_bone_name = bone_name
            best_curves = curves
            best_distance = distance

    if best_bone_name and best_curves:
        print(f"Auto detected moving bone '{best_bone_name}' with distance {best_distance:.4f}.")
        return best_bone_name, best_curves

    raise RuntimeError("No keyed bone location motion found on the selected axes.")


def collect_location_targets(action, armature):
    all_curves = get_action_fcurves(action, armature)
    targets = []

    object_curves = {curve.array_index: curve for curve in all_curves if curve.data_path == "location"}
    if object_curves:
        targets.append(("object", armature.name, object_curves))

    prefix = 'pose.bones["'
    suffix = '"].location'
    curves_by_bone = {}
    for curve in all_curves:
        path = curve.data_path
        if path.startswith(prefix) and path.endswith(suffix):
            bone_name = path[len(prefix):-len(suffix)]
            curves_by_bone.setdefault(bone_name, {})[curve.array_index] = curve

    for bone_name, curves in curves_by_bone.items():
        targets.append(("bone", bone_name, curves))

    return targets


def find_best_location_target(action, armature, preferred_bone_name, indices):
    detection_indices = [0, 1, 2] if AUTO_DETECT_AXES_IF_ZERO else indices
    targets = collect_location_targets(action, armature)

    if PRINT_DIAGNOSTICS:
        ranked = []
        for kind, name, curves in targets:
            ranked.append((measure_motion_distance(curves, detection_indices), kind, name, curves))
        ranked.sort(reverse=True, key=lambda item: item[0])
        print("Location motion diagnostics:")
        for distance, kind, name, curves in ranked[:12]:
            axes = sorted(curves.keys())
            print(f"  {kind}:{name} axes={axes} distance={distance:.4f}")

    candidates = []
    for kind, name, curves in targets:
        if TARGET_KIND == "bone" and kind != "bone":
            continue
        if TARGET_KIND == "object" and kind != "object":
            continue
        candidates.append((kind, name, curves))

    if not candidates:
        raise RuntimeError(f"No location targets found for TARGET_KIND '{TARGET_KIND}'.")

    for kind, name, curves in candidates:
        if kind == "bone" and name == preferred_bone_name:
            if measure_motion_distance(curves, detection_indices) > 0.0001:
                return kind, name, curves

    best = None
    best_distance = 0.0
    for kind, name, curves in candidates:
        distance = measure_motion_distance(curves, detection_indices)
        if distance > best_distance:
            best = (kind, name, curves)
            best_distance = distance

    if best and best_distance > 0.0001:
        print(f"Auto selected {best[0]} '{best[1]}' with distance {best_distance:.4f}.")
        return best

    raise RuntimeError("No location motion found on object or pose bones.")


def axis_indices():
    result = []
    if AXES.get("X", False):
        result.append(0)
    if AXES.get("Y", False):
        result.append(1)
    if AXES.get("Z", False):
        result.append(2)
    if not result:
        raise RuntimeError("Enable at least one axis in AXES.")
    return result


def keyed_location_at_frame(curves, frame):
    value = Vector((0.0, 0.0, 0.0))
    for index, curve in curves.items():
        if 0 <= index <= 2:
            value[index] = curve.evaluate(frame)
    return value


def get_frame_range(curves):
    frames = []
    for curve in curves.values():
        frames.extend(point.co.x for point in curve.keyframe_points)

    if not frames:
        raise RuntimeError("Root location curves have no keyframes.")

    return min(frames), max(frames)


def filtered_vector(vector, indices):
    result = Vector((0.0, 0.0, 0.0))
    for index in indices:
        result[index] = vector[index]
    return result


def keyframes_for_curves(curves):
    frames = set()
    for curve in curves.values():
        for point in curve.keyframe_points:
            frames.add(point.co.x)
    return sorted(frames)


def measure_motion_distance(curves, indices):
    frames = keyframes_for_curves(curves)
    if len(frames) < 2:
        return 0.0

    if MOTION_MEASURE == "end_to_end":
        start = keyed_location_at_frame(curves, frames[0])
        end = keyed_location_at_frame(curves, frames[-1])
        return filtered_vector(end - start, indices).length

    if MOTION_MEASURE != "path_length":
        raise RuntimeError('MOTION_MEASURE must be "path_length" or "end_to_end".')

    distance = 0.0
    previous = keyed_location_at_frame(curves, frames[0])
    for frame in frames[1:]:
        current = keyed_location_at_frame(curves, frame)
        distance += filtered_vector(current - previous, indices).length
        previous = current

    return distance


def resolve_motion_axes(curves, indices):
    current_distance = measure_motion_distance(curves, indices)
    if current_distance > 0.0001 or not AUTO_DETECT_AXES_IF_ZERO:
        return indices

    best_indices = indices
    best_distance = 0.0
    for candidate in ([0], [1], [2], [0, 1], [0, 2], [1, 2], [0, 1, 2]):
        distance = measure_motion_distance(curves, candidate)
        if distance > best_distance:
            best_distance = distance
            best_indices = candidate

    if best_distance > 0.0001:
        print(f"Selected axes had zero motion. Using axes {best_indices} with distance {best_distance:.4f}.")
        return best_indices

    return indices


def calculate_scale(curves, indices):
    if MODE == "scale":
        return SCALE

    if MODE != "target_distance":
        raise RuntimeError('MODE must be "target_distance" or "scale".')

    current_distance = measure_motion_distance(curves, indices)

    if current_distance <= 0.0001:
        raise RuntimeError("Current root motion distance is zero on the selected axes.")

    return TARGET_DISTANCE / current_distance


def scale_curve_points(curves, indices, factor):
    start_frame, _ = get_frame_range(curves)
    start_location = keyed_location_at_frame(curves, start_frame)

    for index in indices:
        curve = curves.get(index)
        if not curve:
            continue

        origin = start_location[index] if KEEP_START_LOCATION else 0.0
        for point in curve.keyframe_points:
            offset = point.co.y - origin
            point.co.y = origin + offset * factor

            left_offset = point.handle_left.y - origin
            right_offset = point.handle_right.y - origin
            point.handle_left.y = origin + left_offset * factor
            point.handle_right.y = origin + right_offset * factor

        curve.update()


def main():
    armature = get_selected_armature()
    root_bone_name = ROOT_BONE_NAME
    needs_bone_name = TARGET_KIND != "object"
    if needs_bone_name and root_bone_name not in armature.pose.bones:
        for fallback_name in FALLBACK_ROOT_BONE_NAMES:
            if fallback_name in armature.pose.bones:
                root_bone_name = fallback_name
                print(f"Bone '{ROOT_BONE_NAME}' not found. Using '{root_bone_name}' instead.")
                break

    if TARGET_KIND == "bone" and root_bone_name not in armature.pose.bones:
        available = ", ".join(bone.name for bone in armature.pose.bones[:20])
        raise RuntimeError(
            f"Bone '{ROOT_BONE_NAME}' not found on '{armature.name}'. First bones: {available}"
        )

    action = get_action(armature)
    indices = axis_indices()
    target_kind = "bone"
    if TARGET_KIND == "auto":
        target_kind, root_bone_name, curves = find_best_location_target(action, armature, root_bone_name, indices)
    elif TARGET_KIND == "object":
        target_kind = "object"
        root_bone_name = armature.name
        curves = get_object_location_curves(action, armature)
        if not curves:
            raise RuntimeError(f"Armature object '{armature.name}' has no location curves in action '{action.name}'.")
    elif AUTO_DETECT_MOVING_BONE:
        target_kind = "bone"
        root_bone_name, curves = find_best_moving_bone(action, armature, root_bone_name, indices)
    else:
        target_kind = "bone"
        curves = find_location_curves(action, armature, root_bone_name)

    indices = resolve_motion_axes(curves, indices)
    before_distance = measure_motion_distance(curves, indices)
    factor = calculate_scale(curves, indices)

    scale_curve_points(curves, indices, factor)
    action.update_tag()
    after_distance = measure_motion_distance(curves, indices)

    bpy.context.scene.frame_set(bpy.context.scene.frame_current)
    bpy.context.view_layer.update()

    print(
        f"Scaled root motion for action '{action.name}', {target_kind} '{root_bone_name}', "
        f"axes {indices}, factor {factor:.4f}, distance {before_distance:.4f} -> {after_distance:.4f}."
    )


if __name__ == "__main__":
    main()
