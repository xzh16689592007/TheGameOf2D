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
  - `BP_ModengEnemy` currently uses `SM_Stickman` on `EnemyBody`.
  - C++ supports optional material color override and hit flash for placeholder visuals. For imported art, keep `bOverrideBodyMaterialColor` false so the model keeps its own material.

- `AModengFastEnemy`
  - Files:
    - `Source/TheGameOf2D/ModengFastEnemy.h`
    - `Source/TheGameOf2D/ModengFastEnemy.cpp`
  - Inherits `AModengEnemy`.
  - Faster, lower health, faster attack interval.
  - Currently still uses placeholder visual defaults unless its Blueprint is updated.

- `AModengExploderEnemy`
  - Files:
    - `Source/TheGameOf2D/ModengExploderEnemy.h`
    - `Source/TheGameOf2D/ModengExploderEnemy.cpp`
  - Inherits `AModengEnemy`.
  - Overrides attack behavior.
  - Bursts near lantern, deals larger damage, then dies.
  - Currently still uses placeholder visual defaults unless its Blueprint is updated.

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
- The current enemy Stickman is still used as a static mesh, so it does not play idle/walk/attack animations yet.
- `SK_Stickman` and `ABP_Stickman` are present and ready for the next upgrade step.
- Recommended next visual upgrade:
  - Add/enable a skeletal mesh component for enemies, or use the inherited `ACharacter::Mesh`.
  - Assign `SK_Stickman`.
  - Assign `ABP_Stickman`.
  - Hide or remove the static `EnemyBody` once skeletal visuals are working.

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

Note: this handoff file may be updated again in the same working session after the latest Stickman asset commit is created.

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

Team clone instructions:

```bash
git clone https://github.com/xzh16689592007/TheGameOf2D.git
cd TheGameOf2D
git lfs pull
```

## Known Issues / Rough Edges

- Basic enemy has been visually swapped to the imported Stickman static mesh, but it is not animated yet.
- Fast and exploder enemies still need their own distinct model/animation setup.
- HUD is C++ Canvas HUD, not polished UMG.
- Enemy movement currently follows X axis only. This is fine for ground-level lanterns, but platform lanterns need route points, flying enemies, ranged enemies, or a 2.5D path system later.
- Most debug messages now have exposed `bShowGameplayDebugMessages` toggles and are off by default.
- If all lanterns are on platforms or unreachable by X-only enemies, enemies may not behave as intended.
- Current game lacks polished enemy animations, sound, particles, and UI art.

## Suggested Next Steps

1. Commit/push the current Stickman asset integration if not already done:
   - `Content/ROG_Creatures/**`
   - `Content/MoDeng/Blueprints/BP_ModengEnemy.uasset`
   - C++ material override changes in `AModengEnemy`
   - This updated `PROJECT_HANDOFF.md`

2. Upgrade enemies from static Stickman mesh to animated skeletal visuals:
   - Use `SK_Stickman`.
   - Use `ABP_Stickman`.
   - Start with `BP_ModengEnemy`.
   - Keep collision on the capsule, not on the mesh.

3. Add enemy health bars:
   - C++ `UWidgetComponent` on enemies.
   - Hide at full health or show only after damage.

4. Add lantern interaction prompt:
   - Show `E / F Repair` near damaged lanterns when player is close.

5. Later: upgrade enemy movement beyond X-axis:
   - Route point system for platform levels.
   - Flying enemy.
   - Ranged enemy.
   - Climber enemy.
