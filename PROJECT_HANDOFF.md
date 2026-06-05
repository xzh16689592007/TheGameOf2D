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
- Imported Fab asset folder: `Content/ROG_Creatures`
  - Current imported pack: `ROG Creatures: Stickman`
  - Important resources:
    - `Content/ROG_Creatures/Stickman/Meshes/SM_Stickman.uasset`
    - `Content/ROG_Creatures/Stickman/Meshes/SK_Stickman.uasset`
    - `Content/ROG_Creatures/Stickman/Animations/ABP_Stickman.uasset`
    - `Content/ROG_Creatures/Stickman/Animations/A_Stickman_Idle.uasset`
    - `Content/ROG_Creatures/Stickman/Animations/A_Stickman_Walk.uasset`
- Important Blueprints:
  - `Content/MoDeng/Blueprints/BP_ModengLantern.uasset`
  - `Content/MoDeng/Blueprints/BP_ModengEnemy.uasset`
  - `Content/MoDeng/Blueprints/BP_ModengFastEnemy.uasset`
  - `Content/MoDeng/Blueprints/BP_ModengExploderEnemy.uasset`
  - `Content/MoDeng/Blueprints/BP_ModengEnemySpawner.uasset`

## Important C++ Classes

- `AModengLantern`
  - Files:
    - `Source/TheGameOf2D/ModengLantern.h`
    - `Source/TheGameOf2D/ModengLantern.cpp`
  - Has durability, repair, damage, light intensity/color update.
  - Implements `ISideScrollingInteractable`.

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
  - C++ default skeletal visuals load `SK_Stickman` and `ABP_Stickman`.
  - C++ default animation hooks load Stickman idle, walk, attack, hit, and death animation sequences.
  - C++ can directly drive idle/walk looping animations with `bUseDirectLocomotionAnimations`, which avoids relying on the animation blueprint receiving movement speed from manual `SetActorLocation` movement.
  - Attacking a lantern temporarily plays the attack animation sequence, then restores the animation blueprint.
  - Taking non-lethal damage plays a hit animation and still updates the health bar.
  - Dying plays the death animation sequence, disables collision immediately, then destroys after the animation/delay.
  - `EnemyBody` remains as a static mesh fallback and is hidden when skeletal visuals are enabled.
  - `BP_ModengEnemy` previously used `SM_Stickman` on `EnemyBody`; C++ now prefers the skeletal Stickman setup at runtime.
  - C++ supports optional material color override and hit flash for placeholder visuals. For imported art, keep `bOverrideBodyMaterialColor` false so the model keeps its own material.

- `AModengFastEnemy`
  - Files:
    - `Source/TheGameOf2D/ModengFastEnemy.h`
    - `Source/TheGameOf2D/ModengFastEnemy.cpp`
  - Inherits `AModengEnemy`.
  - Faster, lower health, faster attack interval.
  - Currently keeps placeholder/static visual defaults; `bUseSkeletalMeshVisuals` is disabled in C++ until it gets a distinct animated setup.

- `AModengExploderEnemy`
  - Files:
    - `Source/TheGameOf2D/ModengExploderEnemy.h`
    - `Source/TheGameOf2D/ModengExploderEnemy.cpp`
  - Inherits `AModengEnemy`.
  - Overrides attack behavior.
  - Bursts near lantern, deals larger damage, then dies.
  - Currently keeps placeholder/static visual defaults; `bUseSkeletalMeshVisuals` is disabled in C++ until it gets a distinct animated setup.

- `AModengEnemySpawner`
  - Files:
    - `Source/TheGameOf2D/ModengEnemySpawner.h`
    - `Source/TheGameOf2D/ModengEnemySpawner.cpp`
  - Wave-based spawner.
  - Exposes `EnemyTypes`, `TotalWaves`, `BaseEnemiesPerWave`, `ExtraEnemiesPerWave`, `SpawnInterval`, `MaxAliveEnemies`, `DelayBetweenWaves`.
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
    - Uses a box overlap from the player forward, not a single sphere.
    - Has a short cylinder-based slash/range visual instead of the old debug cube.
    - Applies light knockback to enemies that survive the hit.
    - Supports a configurable `AttackAnimation` sequence, currently driven from C++ with `PlayAnimation()`.
    - C++ records the mesh relative transform before the attack animation and restores it afterward via `bRestoreMeshTransformAfterAttackAnimation`, to reduce root/retarget offset issues.
    - If an attack animation visually lunges or snaps, prefer a retargeted `IP`/InPlace animation and disable root motion on the animation asset.
  - Visual setup:
    - C++ exposes `PlayerSkeletalMesh`, `PlayerAnimClass`, mesh relative transform, and attack animation fields for Blueprint tuning.
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
- Player art upgrade is in progress:
  - `Content/SamuraiGirlTomoe` has been copied into the project.
  - `Content/CombatMasterAnimBundle` has been copied into the project.
  - UE IK Retarget work has started:
    - Source IK Rig was created for CombatMaster/Katana mannequin, named around `IKR_Combat_Katana`.
    - Target IK Rig was created for Tomoe, named around `IKR_Tomoe`.
    - IK Retargeter was created under `Content/MoDeng/Animations`, named around `RTG_CombatKatana_To_Tomoe`.
    - A Tomoe attack animation was retargeted/exported under `Content/MoDeng/Animations/Tomoe/Attack`.
  - User tested and reported Tomoe is visible in game and a retargeted attack can play.
  - Recommended attack source to continue with: `DynamicKatanaAnimsV2/IP/Attack/Anim_DK2_Combo_A1_IP`, not root-motion variants.
  - `BP_SideScrollingCharacter` has been edited manually in UE and now has pending `.uasset` changes.
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

Latest synced milestone before current uncommitted work: basic Stickman enemy visuals and animation feedback are working, enemy health bars and lantern repair prompt are in place, and Git proxy has been configured for Clash Verge on `127.0.0.1:7897`.

Current uncommitted milestone: Tomoe player model and Combat Master sword animation assets have been copied into `Content`, Tomoe retargeting has started, a Tomoe attack animation has been exported and tested, and later-wave enemy health bars have been fixed in C++.

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
- Current working tree includes asset imports and UE-generated/editor changes. Before committing, inspect:
  - `Content/SamuraiGirlTomoe/`
  - `Content/CombatMasterAnimBundle/`
  - `Content/MoDeng/Animations/`
  - `Content/Variant_SideScrolling/Blueprints/BP_SideScrollingCharacter.uasset`
  - `Config/DefaultEngine.ini`
  - `Config/DefaultEditor.ini` (likely noisy; avoid committing unless intentional)
  - C++ files changed for player animation defaults and enemy health-bar widget self-healing.

Team clone instructions:

```bash
git clone https://github.com/xzh16689592007/TheGameOf2D.git
cd TheGameOf2D
git lfs pull
```

## Known Issues / Rough Edges

- Basic enemy now has C++ skeletal Stickman defaults and direct idle/walk/attack/hit/death animation sequence hooks. User tested it in UE and reported no issues.
- Later-wave enemy health bars previously disappeared. This was fixed by revalidating/initializing the native health widget before health-bar update/show. User reported it works.
- Fast and exploder enemies still need their own distinct model/animation setup.
- HUD is C++ Canvas HUD, not polished UMG.
- Player is being migrated from default Manny/Quinn appearance to Samurai Girl Tomoe.
- Player attack now supports a retargeted sword animation, but the animation/AnimBP setup is still rough:
  - Prefer retargeted `IP` / InPlace attack animations.
  - Root-motion or bad retarget choices can cause visual mesh lunges/snaps.
  - C++ mesh-transform restoration helps but is not a substitute for clean in-place animation assets.
  - Tomoe base locomotion still needs a proper Tomoe AnimBP or retargeted side-scroller locomotion animations.
- Enemy movement currently follows X axis only. This is fine for ground-level lanterns, but platform lanterns need route points, flying enemies, ranged enemies, or a 2.5D path system later.
- Most debug messages now have exposed `bShowGameplayDebugMessages` toggles and are off by default.
- If all lanterns are on platforms or unreachable by X-only enemies, enemies may not behave as intended.
- Current game lacks polished player art, sound, particles, and UI art.

## Suggested Next Steps

1. Continue Tomoe player setup:
   - Open `BP_SideScrollingCharacter`.
   - Confirm `CharacterMesh0` and class default `PlayerSkeletalMesh` both point to `SK_SAMURAIGIRL_01`.
   - Confirm `AttackAnimation` points to a Tomoe-retargeted `IP` attack animation under `Content/MoDeng/Animations/Tomoe/Attack`.
   - If the attack still lunges/snaps, retarget a different `IP` source animation or check the animation asset root-motion settings (`Enable Root Motion=false`, `Force Root Lock=true`).
   - Run Live Coding (`Ctrl+Alt+F11`) or close UE and full-build so the latest C++ mesh-transform restoration is active.

2. Build a proper Tomoe locomotion setup:
   - Retarget Tomoe idle/walk/run/jump/fall/land animations, or use the Tomoe demo animations if suitable.
   - Create a Tomoe-compatible side-scroller AnimBP.
   - Keep C++ movement/jump/combat logic unchanged while replacing only visual animation.

3. Add a visible katana/weapon:
   - Use `CombatMasterAnimBundle` katana static mesh assets.
   - Attach the katana to Tomoe hand socket in Blueprint after confirming hand/socket names.
   - Keep C++ hit detection separate from weapon mesh for now.

4. Commit carefully after testing:
   - New `.uasset` imports are large and should go through Git LFS.
   - Exclude noisy `Config/DefaultEditor.ini` if it only contains editor preview/profile changes.

5. Later: give fast and exploder enemies distinct polished visuals:
   - Fast enemy can be smaller/lighter/faster-looking.
   - Exploder enemy can be larger/redder/more volatile-looking.
   - They currently remain acceptable placeholders because their shapes distinguish enemy types.

6. Later: upgrade enemy movement beyond X-axis:
   - Route point system for platform levels.
   - Flying enemy.
   - Ranged enemy.
   - Climber enemy.
