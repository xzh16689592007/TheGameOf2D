# TheGameOf2D Project Handoff

## Project Basics

- Project path: `D:\UE_project\TheGameOf2D`
- Unreal project file: `D:\UE_project\TheGameOf2D\TheGameOf2D.uproject`
- Engine: Unreal Engine 5.7
- GitHub repo: `https://github.com/xzh16689592007/TheGameOf2D`
- Branch: `main`
- Game concept: `墨灯守巷`, a 2.5D side-view lantern-defense action prototype.
- Training requirement: gameplay logic should use C++; Blueprint can be used for assets, tuning, UI, materials, VFX, animation, etc.

## Current Design Direction

- Use 3D assets and 3D scene, but lock gameplay into a 2.5D side-scrolling style.
- Player movement is constrained to a side-scrolling plane.
- No Paper2D/PaperZD currently.
- Main gameplay loop:
  - Lanterns have durability.
  - Enemies move toward the nearest lit lantern.
  - Enemies attack lanterns and reduce durability.
  - Player repairs lanterns with `E` or `F`.
  - Player attacks enemies with left mouse button or `J`.
  - Killing enemies gives ink.
  - Ink upgrades weapon level automatically.
  - Spawner controls waves.
  - All lanterns extinguished means defeat.
  - Clearing all waves means victory.

## Important Existing Content

- Main custom level: `Content/MoDeng/Maps/L_Level01_Street.umap`
- Main custom assets folder: `Content/MoDeng`
- Imported player character folder: `Content/SamuraiGirlTomoe`
  - Source library path: `D:\UE素材库\2025角色集合更新\A人物\文件\aa109-Samurai-Girl-Tomoe\SamuraiGirlTomoe`
  - Important resources:
    - `Content/SamuraiGirlTomoe/Mesh/SK_SAMURAIGIRL_01.uasset`
    - `Content/SamuraiGirlTomoe/Mesh/SK_SAMURAIGIRL_02.uasset`
    - `Content/SamuraiGirlTomoe/Mesh/SKEL_Tomoe_Skeleton.uasset`
    - `Content/SamuraiGirlTomoe/Mesh/PA_Tomoe_PhysicsAsset.uasset`
    - `Content/SamuraiGirlTomoe/Mesh/IKR_Tomoe.uasset`
- Imported sword combat animation folder: `Content/CombatMasterAnimBundle`
  - Source library path: `D:\UE素材库\Combat Master Sword Combat Anims Bundle\CombatMasterAnimBundle\CombatMasterAnimBundle`
  - Use the `IP` / `InPlace` animation folders first for this 2.5D project.
  - Avoid `RM` / `RootMotion` animations for now because player movement is controlled by C++.
  - Useful source attack examples:
    - `Content/CombatMasterAnimBundle/Animations/DynamicKatanaAnimsV2/IP/Attack/Anim_DK2_Combo_A1_IP.uasset`
    - `Content/CombatMasterAnimBundle/Animations/DynamicKatanaAnimsV2/IP/Attack/Anim_DK2_Combo_A2_IP.uasset`
    - `Content/CombatMasterAnimBundle/Animations/DynamicKatanaAnims/InPlace/ComboAttack/Anim_DK_Combo_A01_IP.uasset`
  - Weapon examples:
    - `Content/CombatMasterAnimBundle/Weapon/Katana`
    - `Content/CombatMasterAnimBundle/Weapon/Sword`
- Current Tomoe player animation assets:
  - `Content/MoDeng/Animations/RTG_CombatKatana_To_Tomoe.uasset`
  - `Content/MoDeng/Animations/Tomoe/ABP_Tomoe_SideScroller.uasset`
  - `Content/MoDeng/Animations/Tomoe/Attack/Anim_DK2_Combo_A1_IP.uasset`
  - `Content/MoDeng/Animations/Tomoe/Attack/Anim_Katana_Attack_Combo1.uasset`
  - `Content/MoDeng/Animations/Tomoe/Attack/Anim_Katana_Attack_Slash1.uasset`
  - `Content/MoDeng/Animations/Tomoe/Attack/Anim_Katana_Attack_Slash2.uasset`
- Imported Fab asset folder: `Content/ROG_Creatures`
  - Current imported pack: `ROG Creatures: Stickman`
  - Important resources:
    - `Content/ROG_Creatures/Stickman/Meshes/SM_Stickman.uasset`
    - `Content/ROG_Creatures/Stickman/Meshes/SK_Stickman.uasset`
    - `Content/ROG_Creatures/Stickman/Animations/ABP_Stickman.uasset`
    - `Content/ROG_Creatures/Stickman/Animations/A_Stickman_Idle.uasset`
    - `Content/ROG_Creatures/Stickman/Animations/A_Stickman_Walk.uasset`
- Imported lantern art folder: `Content/Fab/China_lantern`
  - C++ defaults now use `ChinaLamp.uasset` and `M_ChinaLamp_Textured.uasset` for `AModengLantern`.
- Imported modular skeleton enemy folder: `Content/ModularCharacterSkeleton`
  - Used by basic, fast, and exploder enemy C++ loadouts.
  - Important resources include `Content/ModularCharacterSkeleton/Meshes/SK_Skeleton.uasset` and modular body part meshes.
- Imported skeleton wizard enemy folder: `Content/StylizedSkeletonWizard`
  - Used by the ranged enemy C++ loadout.
  - Important resources include `Content/StylizedSkeletonWizard/Meshes/SK_Body.uasset`, wizard modular parts, staff meshes, and spell/idle/walk/death animations.
- Important Blueprints:
  - `Content/MoDeng/Blueprints/BP_ModengLantern.uasset`
  - `Content/MoDeng/Blueprints/BP_ModengEnemy.uasset`
  - `Content/MoDeng/Blueprints/BP_ModengFastEnemy.uasset`
  - `Content/MoDeng/Blueprints/BP_ModengExploderEnemy.uasset`
  - `Content/MoDeng/Blueprints/BP_ModengEnemySpawner.uasset`
  - `Content/Variant_SideScrolling/Blueprints/BP_SideScrollingCharacter.uasset`

## Important C++ Classes

- `AModengLantern`
  - Files:
    - `Source/TheGameOf2D/ModengLantern.h`
    - `Source/TheGameOf2D/ModengLantern.cpp`
  - Has durability, repair, damage, light intensity/color update.
  - Implements `ISideScrollingInteractable`.
  - C++ defaults now load the China lantern mesh/material from `Content/Fab/China_lantern`.
  - Recent teammate updates improved lantern light visibility and repair interaction behavior.

- `AModengEnemy`
  - Files:
    - `Source/TheGameOf2D/ModengEnemy.h`
    - `Source/TheGameOf2D/ModengEnemy.cpp`
  - Base enemy class.
  - Finds nearest non-extinguished lantern.
  - Moves along X axis toward the target lantern.
  - Attacks lantern when in range.
  - Has health and ink reward.
  - Enemy collision is query/overlap only, so enemies do not physically block player or each other.
  - Has `EnemyBody` as a `UStaticMeshComponent` for current visuals.
  - Uses inherited `ACharacter::Mesh` for animated skeletal visuals when available.
  - C++ default skeletal visuals now load `Content/ModularCharacterSkeleton/Meshes/SK_Skeleton.uasset`.
  - C++ default modular loadout attaches skeleton armor/body-part meshes to `ACharacter::Mesh`.
  - C++ default animation hooks load modular skeleton idle, walk, attack, hit, and death animation sequences.
  - C++ can directly drive idle/walk looping animations with `bUseDirectLocomotionAnimations`, which avoids relying on the animation blueprint receiving movement speed from manual `SetActorLocation` movement.
  - Attacking a lantern temporarily plays the attack animation sequence, then restores the animation blueprint.
  - Taking non-lethal damage plays a hit animation and still updates the health bar.
  - Dying plays the death animation sequence, disables collision immediately, then destroys after the animation/delay.
  - `EnemyBody` remains as a static mesh fallback and is hidden when skeletal visuals are enabled.
  - `BP_ModengEnemy` previously used `SM_Stickman` on `EnemyBody`; C++ now prefers the modular skeleton setup at runtime.
  - C++ supports optional material color override and hit flash for placeholder visuals. For imported art, keep `bOverrideBodyMaterialColor` false so the model keeps its own material.

- `AModengFastEnemy`
  - Files:
    - `Source/TheGameOf2D/ModengFastEnemy.h`
    - `Source/TheGameOf2D/ModengFastEnemy.cpp`
  - Inherits `AModengEnemy`.
  - Faster, lower health, faster attack interval.
  - Uses modular skeleton visuals through `ApplyEnemyLoadout()`.
  - Current loadout uses lighter/unarmed body parts and unarmed idle/run/attack/hit/death animations.

- `AModengExploderEnemy`
  - Files:
    - `Source/TheGameOf2D/ModengExploderEnemy.h`
    - `Source/TheGameOf2D/ModengExploderEnemy.cpp`
  - Inherits `AModengEnemy`.
  - Overrides attack behavior.
  - Bursts near lantern, deals larger damage, then dies.
  - Uses modular skeleton visuals through `ApplyEnemyLoadout()`.
  - Current loadout uses heavier/brute body parts and weapon-style attack/death animations.

- `AModengRangedEnemy`
  - Files:
    - `Source/TheGameOf2D/ModengRangedEnemy.h`
    - `Source/TheGameOf2D/ModengRangedEnemy.cpp`
  - Inherits `AModengEnemy`.
  - Uses the skeleton wizard mesh, wizard modular parts, staff, and spell-cast animation loadout.
  - Attacks lanterns from range by spawning `AModengMagicProjectile`.

- `AModengMagicProjectile`
  - Files:
    - `Source/TheGameOf2D/ModengMagicProjectile.h`
    - `Source/TheGameOf2D/ModengMagicProjectile.cpp`
  - Projectile actor used by `AModengRangedEnemy`.
  - Travels toward the target lantern and applies impact damage/radius behavior on hit.

- `AModengEnemySpawner`
  - Files:
    - `Source/TheGameOf2D/ModengEnemySpawner.h`
    - `Source/TheGameOf2D/ModengEnemySpawner.cpp`
  - Wave-based spawner.
  - Exposes `EnemyTypes`, `TotalWaves`, `BaseEnemiesPerWave`, `ExtraEnemiesPerWave`, `SpawnInterval`, `MaxAliveEnemies`, `DelayBetweenWaves`.
  - Recent teammate update adds `AModengRangedEnemy` to the default enemy type list if it is not already present.
  - Checks victory/defeat.
  - Shows C++ result UI on victory/defeat.
  - Exposes `OnGameEnded`, `OnVictory`, and `OnDefeat` delegates for Blueprint/UI/VFX hooks.
  - Restores gameplay input on BeginPlay as a safety net after restarting from the result screen.

- `UModengResultWidget`
  - Files:
    - `Source/TheGameOf2D/ModengResultWidget.h`
    - `Source/TheGameOf2D/ModengResultWidget.cpp`
  - Native C++ UMG result widget.
  - Shows Victory/Defeat text.
  - Provides Restart and Quit buttons.
  - Restart restores `GameOnly` input mode, hides mouse cursor, unpauses, removes the widget, then reloads the current level.

- `AModengHUD`
  - Files:
    - `Source/TheGameOf2D/ModengHUD.h`
    - `Source/TheGameOf2D/ModengHUD.cpp`
  - C++ HUD drawn with `AHUD::DrawHUD`.
  - Displays wave status, enemies alive, lantern status, weapon level, ink, damage, range.
  - It is assigned in `ASideScrollingGameMode` constructor.

- `ASideScrollingCharacter`
  - Files:
    - `Source/TheGameOf2D/Variant_SideScrolling/SideScrollingCharacter.h`
    - `Source/TheGameOf2D/Variant_SideScrolling/SideScrollingCharacter.cpp`
  - Modified from UE side-scrolling template.
  - Interaction:
    - `E` and `F` call `DoInteract()`.
    - Uses overlap query to find actors implementing `ISideScrollingInteractable`.
  - Attack:
    - `J` and left mouse button call `DoAttack()`.
    - Current primary hit detection uses the katana trace points on `BP_SideScrollingCharacter`:
      - `KatanaTraceStart`
      - `KatanaTraceEnd`
    - During the attack hit window, C++ sweeps the blade segment every frame and also sweeps from the previous frame's trace points to the current frame's trace points, so fast sword motion is still detected.
    - `AttackHitWindowStartRatio` and `AttackHitWindowEndRatio` tune the active damage window for the current attack animation.
    - One enemy can only be damaged once per player attack through `HitEnemiesThisAttack`.
    - If the trace point components are missing, C++ falls back to the old forward box overlap so attacks do not completely break while debugging Blueprint setup.
    - The old yellow cylinder slash/range visual and weapon debug trace drawing have been removed.
    - Applies light knockback to enemies that survive the hit.
    - Supports a configurable `AttackAnimation` sequence, currently driven from C++ with `PlayAnimation()`.
    - Repeated attack input is blocked while the current attack animation is in progress, so rapid clicking cannot repeatedly apply damage.
    - C++ records the mesh relative transform before the attack animation and restores it afterward via `bRestoreMeshTransformAfterAttackAnimation`, to reduce root/retarget offset issues.
    - Movement input can interrupt the current attack animation via `bMovementInterruptsAttackAnimation`, restoring the configured animation blueprint.
    - Movement interruption also clears the pending attack hit window, so moving cancels damage for that attack.
    - If an attack animation visually lunges or snaps, prefer a retargeted `IP`/InPlace animation and disable root motion on the animation asset.
  - Visual setup:
    - C++ exposes `PlayerSkeletalMesh`, `PlayerAnimClass`, mesh relative transform, and attack animation fields for Blueprint tuning.
    - C++ exposes `bLockFacingToSideScrollingAxis`, `FacingYawRight`, and `FacingYawLeft` so the side-scroller character does not slowly rotate toward the camera while tapping movement.
    - `CharacterMovementComponent::bOrientRotationToMovement` is disabled in C++ for the player; facing is now handled explicitly from horizontal input.
    - C++ defaults still load Manny/side-scroller resources as fallback, but `BP_SideScrollingCharacter` can override them.
  - Progression:
    - `AddInk()`
    - weapon level
    - damage/range/radius scaling.

## Current Controls

- Move: side-scrolling template movement input.
- Jump: template jump input.
- Repair/interact: `E` or `F`.
- Attack: left mouse button or `J`.

## Current Visual Asset State

- `BP_ModengEnemy` has been updated manually in UE:
  - `EnemyBody.StaticMesh = SM_Stickman`.
  - `EnemyBody` transform was adjusted so the Stickman stands on the ground.
  - `EnemyBody` material was changed to a Stickman material instance, so it no longer appears white.
- Basic enemy skeletal animation upgrade has started in C++:
  - `AModengEnemy` uses the inherited `ACharacter::Mesh` component.
  - Default skeletal mesh is `SK_Stickman`.
  - Default animation blueprint is `ABP_Stickman`.
  - Default idle animation is `A_Stickman_Idle`.
  - Default walk animation is `A_Stickman_Walk`.
  - Default attack animation is `A_Stickman_Attack_01`.
  - Default hit/knockback animation is `A_Stickman_hit_back`.
  - Default death animation is `A_Stickman_Death`.
  - Collision remains on the capsule, not the mesh.
  - Static `EnemyBody` is retained as a fallback and hidden when skeletal visuals are active.
- Fast and exploder enemies still need distinct model/animation setup.
- Player art upgrade current state:
  - `Content/SamuraiGirlTomoe` has been copied into the project.
  - `Content/CombatMasterAnimBundle` has been copied into the project.
  - UE IK Retarget work has started:
    - Source IK Rig was created for CombatMaster/Katana mannequin, named around `IKR_Combat_Katana`.
    - Target IK Rig was created for Tomoe, named around `IKR_Tomoe`.
    - IK Retargeter was created under `Content/MoDeng/Animations`, named around `RTG_CombatKatana_To_Tomoe`.
    - A Tomoe attack animation was retargeted/exported under `Content/MoDeng/Animations/Tomoe/Attack`.
  - `ABP_Tomoe_SideScroller` has been created as the first Tomoe locomotion animation blueprint.
    - Event Graph reads pawn velocity and writes a `Speed` float.
    - Event Graph now also reads character movement `IsFalling` into `IsInAir`.
    - Event Graph now stores vertical velocity in `VelocityZ`.
    - `Locomotion` state machine currently has `Idle`, `Run`, `JumpStart`, and `FallLoop` states.
    - Current idle/run assets are Tomoe demo animations, with `MF_Run_Fwd` used for run.
    - Jump transitions:
      - `Idle -> JumpStart` and `Run -> JumpStart` when `IsInAir` is true.
      - `JumpStart -> FallLoop` when `VelocityZ <= 0`.
      - `FallLoop -> Idle` when grounded and `Speed <= 5`.
      - `FallLoop -> Run` when grounded and `Speed > 5`.
    - Important animation note: Tomoe jump/fall states must use Tomoe-compatible animations. Do not use `Skeleton_Anim_Jump*` from `Content/ModularCharacterSkeleton` directly on Tomoe; it causes visible body deformation/squashing.
    - User tested the corrected jump setup in UE after replacing the wrong jump animation, and the jump/fall state split works.
  - `BP_SideScrollingCharacter` has been updated to use Tomoe player visuals and Tomoe animation setup.
  - User tested and reported Tomoe is visible in game, idle/run works, and a retargeted attack can play.
  - C++ movement interruption and fixed side-scroller facing were added after the user noticed attack sliding and slow camera-facing rotation when tapping movement.
  - Initial katana weapon hookup has started:
    - Use `Content/CombatMasterAnimBundle/Weapon/Katana/SM_Katana.uasset`.
    - Add/use a right-hand socket named `Socket_Katana_R` on `hand_r`.
    - Add a `Katana` static mesh component under `CharacterMesh0` in `BP_SideScrollingCharacter`, parented to `Socket_Katana_R`, with collision disabled.
    - Add `KatanaTraceStart` and `KatanaTraceEnd` scene components as children of `Katana`.
    - `KatanaTraceStart` should sit near the blade root/guard.
    - `KatanaTraceEnd` should sit near the blade tip.
    - Damage comes from C++ sweep traces between those two points during the attack hit window, not from the weapon mesh's own collision.
    - User tested the final behavior: blade contact now applies damage, rapid clicking no longer causes repeated instant damage, and the temporary yellow slash/debug trace visuals were removed.
  - Recommended attack source to continue with: `DynamicKatanaAnimsV2/IP/Attack/Anim_DK2_Combo_A1_IP`, not root-motion variants.
- Enemy health bar issue was fixed defensively in C++:
  - Added `AModengEnemy::EnsureHealthBarWidget()`.
  - Health bar widget class, instance, color, and percent are revalidated before initialize/update/show.
  - User tested later waves and reported the fix works.

## Build Commands

Use this full build command when UE is closed:

```powershell
& "D:\UE_5.7\Engine\Build\BatchFiles\Build.bat" TheGameOf2DEditor Win64 Development -Project="D:\UE_project\TheGameOf2D\TheGameOf2D.uproject" -WaitMutex
```

If UE is open and Live Coding is active, full build will fail with:

```text
Unable to build while Live Coding is active
```

In that case either:

- Press `Ctrl + Alt + F11` inside UE for Live Coding, or
- Close UE and run full build.

Important: after adding new C++ classes, prefer closing UE and running a full build before reopening. Otherwise Blueprints can temporarily show parent class missing.

## Toolchain

- Visual Studio 2026 is installed.
- MSVC 14.44 is used and works.
- Full compile has succeeded multiple times with:

```text
Using Visual Studio 2026 14.44.35227 toolchain
Result: Succeeded
```

The UnrealBuildTool config forcing 14.44 exists at:

```text
C:\Users\Lenovo\AppData\Roaming\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml
```

Expected content:

```xml
<?xml version="1.0" encoding="utf-8"?>
<Configuration xmlns="https://www.unrealengine.com/BuildConfiguration">
  <WindowsPlatform>
    <CompilerVersion>14.44.35207</CompilerVersion>
  </WindowsPlatform>
</Configuration>
```

## Recent Commits

```text
1af8cdd Fix melee enemy loadouts
468472f Fix lantern repair interaction
6dc11a5 Make lantern light more visible
9568979 Add new enemy visuals and lantern assets
3b809c7 Add katana trace attack window
a4d0a2a Update project handoff after Tomoe setup
9fcc0c0 Add Tomoe player art and combat animation setup
91a9714 Add project README
86c0731 Update project handoff
8c97964 Improve basic enemy animations
7fe7cc4 Update variant enemy blueprints
0043235 Add enemy health bars and repair prompt
786c9bf Tune Stickman enemy appearance
d40f040 Add skeletal enemy visual support
2642cc0 Add Stickman enemy art
50f2eed Differentiate enemy visuals
5b4c2a8 Restore input after restarting from result screen
3be1dee Improve player attack feedback
31baa68 Add victory and defeat result UI
49ebb54 Update Level01 street actors
b8a63ca Add gameplay debug message toggles
ef9f795 Add gameplay status HUD
b941d60 Add wave-based enemy spawning
8b6d6ba Initial Unreal project
```

Latest synced milestone before the current pending commit: commit `1af8cdd` is pulled from `origin/main`. Tomoe player art, CombatMaster sword animation assets, katana trace attack window, new lantern art, modular skeleton enemy visuals, skeleton wizard ranged enemy, lantern repair/light fixes, melee enemy loadout fixes, and `DefaultEngine.ini` game map/game mode changes are all on GitHub.

Current pending update in this handoff:

- `ABP_Tomoe_SideScroller` now has `IsInAir` and `VelocityZ`.
- Tomoe locomotion now separates `JumpStart` and `FallLoop`.
- Jump/fall transitions were tested in UE after replacing an incorrect non-Tomoe jump animation.
- `Config/DefaultEditor.ini` may still be locally modified from editor preview/profile noise and should not be committed unless intentional.

## Git Setup And Notes

- `.gitignore` exists and ignores:
  - `Binaries/`
  - `Intermediate/`
  - `Saved/`
  - `DerivedDataCache/`
  - `.vs/`
- `.gitattributes` exists and tracks `.uasset`, `.umap`, images, audio, video, etc. through Git LFS.
- Git LFS is installed and initialized.
- The Fab asset import under `Content/ROG_Creatures` should be committed through Git LFS.
- `Config/DefaultEditor.ini` may get noisy editor preview-profile changes from opening imported assets. Do not commit those preview-profile changes unless the team explicitly wants editor profile settings in source control.
- Large Tomoe and CombatMaster imports were pushed through Git LFS in commit `9fcc0c0` (`878` LFS objects, about `1.1 GB` uploaded during push).
- Current local working tree may still show `Config/DefaultEditor.ini` modified from editor preview-profile noise. Do not commit that file unless intentional.

Team clone instructions:

```bash
git clone https://github.com/xzh16689592007/TheGameOf2D.git
cd TheGameOf2D
git lfs pull
```

## Known Issues / Rough Edges

- Basic, fast, exploder, and ranged enemies now have C++ skeletal loadouts using the modular skeleton / skeleton wizard asset packs.
- Later-wave enemy health bars previously disappeared. This was fixed by revalidating/initializing the native health widget before health-bar update/show. User reported it works.
- Fast and exploder enemies have distinct modular skeleton loadouts, but their exact scale/readability may still need gameplay tuning.
- Ranged enemy exists and uses `AModengMagicProjectile`; tune projectile speed, impact radius, spawn offset, and wave pacing after playtesting.
- HUD is C++ Canvas HUD, not polished UMG.
- Player is now visually migrated from default Manny/Quinn appearance to Samurai Girl Tomoe for the current prototype.
- Player attack now supports a retargeted sword animation, but the animation/AnimBP setup is still early:
  - Prefer retargeted `IP` / InPlace attack animations.
  - Root-motion or bad retarget choices can cause visual mesh lunges/snaps.
  - C++ mesh-transform restoration helps but is not a substitute for clean in-place animation assets.
  - Movement currently interrupts attack animation intentionally; later this should move to Montage/AnimBP-driven attack states for smoother combat.
  - Attack damage timing is currently C++ timer/window driven, using `AttackHitWindowStartRatio` / `AttackHitWindowEndRatio`. Later this should move to Anim Notifies or Montages for cleaner combo timing.
  - Tomoe base locomotion now has idle/run/jump-start/fall-loop. Landing polish is still not implemented as a dedicated state.
  - Use only Tomoe-compatible or Tomoe-retargeted animation assets in `ABP_Tomoe_SideScroller`; using the modular skeleton enemy jump animations directly causes body deformation.
- Visible katana setup works, but socket/rotation and trace point placement may still need tuning if future attack animations change the blade path.
- Enemy movement currently follows X axis only. This is fine for ground-level lanterns, but platform lanterns need route points, flying enemies, ranged enemies, or a 2.5D path system later.
- Most debug messages now have exposed `bShowGameplayDebugMessages` toggles and are off by default.
- If all lanterns are on platforms or unreachable by X-only enemies, enemies may not behave as intended.
- Current game lacks polished player art, sound, particles, and UI art.

## Suggested Next Steps

1. Polish Tomoe jump/fall locomotion:
   - Current `JumpStart` / `FallLoop` split is working.
   - Add a dedicated `Land` state only if it does not make movement feel sticky.
   - Tune transition blend duration between `JumpStart` and `FallLoop` if the pose pops.
   - Keep using Tomoe-compatible or Tomoe-retargeted jump/fall animations; do not use enemy skeleton animations directly.

2. Continue polishing Tomoe player setup:
   - Open `BP_SideScrollingCharacter`.
   - Confirm `CharacterMesh0` and class default `PlayerSkeletalMesh` both point to `SK_SAMURAIGIRL_01`.
   - Confirm class default `PlayerAnimClass` points to `ABP_Tomoe_SideScroller`.
   - Confirm `AttackAnimation` points to a Tomoe-retargeted `IP` attack animation under `Content/MoDeng/Animations/Tomoe/Attack`.
   - Confirm `Movement Interrupts Attack Animation` and `Lock Facing To Side Scrolling Axis` are set as desired.
   - If the attack still lunges/snaps, retarget a different `IP` source animation or check the animation asset root-motion settings (`Enable Root Motion=false`, `Force Root Lock=true`).
   - Run Live Coding (`Ctrl+Alt+F11`) or close UE and full-build so the latest C++ mesh-transform restoration is active.

3. Tune the visible katana/weapon:
   - Verify `Socket_Katana_R` is on Tomoe's real `hand_r` bone, not `ik_hand_r`.
   - Verify the `Katana` component in `BP_SideScrollingCharacter` uses `SM_Katana`, parent socket `Socket_Katana_R`, and `NoCollision`.
   - Verify `KatanaTraceStart` and `KatanaTraceEnd` remain children of `Katana`.
   - If a new attack animation misses despite visible blade contact, tune `AttackHitWindowStartRatio`, `AttackHitWindowEndRatio`, and `WeaponTraceRadius` first.
   - Tune socket transform in `SKEL_Tomoe_Skeleton` until the grip sits naturally in the right hand.
   - Keep C++ hit detection separate from weapon mesh collision for now.

4. Later: move player attacks from direct `PlayAnimation()` to Montage or AnimBP attack states:
   - This will make attack blending, interruption, combo timing, Anim Notifies, and future jump/attack interactions cleaner.
   - Keep the existing C++ katana trace damage for gameplay until the animation system is ready for notifies/montages.
   - Combo attacks 2/3 should reuse the same trace-window idea, with per-attack hit windows and one-hit-per-enemy tracking.

5. Later: give fast and exploder enemies distinct polished visuals:
   - Fast enemy can be smaller/lighter/faster-looking.
   - Exploder enemy can be larger/redder/more volatile-looking.
   - They currently remain acceptable placeholders because their shapes distinguish enemy types.

6. Later: upgrade enemy movement beyond X-axis:
   - Route point system for platform levels.
   - Flying enemy.
   - Ranged enemy.
   - Climber enemy.
