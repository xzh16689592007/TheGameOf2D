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

- `AModengFastEnemy`
  - Files:
    - `Source/TheGameOf2D/ModengFastEnemy.h`
    - `Source/TheGameOf2D/ModengFastEnemy.cpp`
  - Inherits `AModengEnemy`.
  - Faster, lower health, faster attack interval.

- `AModengExploderEnemy`
  - Files:
    - `Source/TheGameOf2D/ModengExploderEnemy.h`
    - `Source/TheGameOf2D/ModengExploderEnemy.cpp`
  - Inherits `AModengEnemy`.
  - Overrides attack behavior.
  - Bursts near lantern, deals larger damage, then dies.

- `AModengEnemySpawner`
  - Files:
    - `Source/TheGameOf2D/ModengEnemySpawner.h`
    - `Source/TheGameOf2D/ModengEnemySpawner.cpp`
  - Wave-based spawner.
  - Exposes `EnemyTypes`, `TotalWaves`, `BaseEnemiesPerWave`, `ExtraEnemiesPerWave`, `SpawnInterval`, `MaxAliveEnemies`, `DelayBetweenWaves`.
  - Checks victory/defeat.

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
    - Has a temporary cube attack visual.
  - Progression:
    - `AddInk()`
    - weapon level
    - damage/range/radius scaling.

## Current Controls

- Move: side-scrolling template movement input.
- Jump: template jump input.
- Repair/interact: `E` or `F`.
- Attack: left mouse button or `J`.

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

## Git Status At Handoff

Last local commits:

```text
ef9f795 Add gameplay status HUD
b941d60 Add wave-based enemy spawning
8b6d6ba Initial Unreal project
```

Important: `ef9f795 Add gameplay status HUD` is committed locally but was not pushed because GitHub connection failed.

Current branch state at handoff:

```text
main...origin/main [ahead 1]
```

Run this when network is OK:

```powershell
git push
```

There are also local map/external actor asset changes not committed:

```text
M Content/__ExternalActors__/MoDeng/Maps/L_Level01_Street/3/5P/HGNJC3M1MVZIA2J2W73478.uasset
D Content/__ExternalActors__/MoDeng/Maps/L_Level01_Street/E/OW/X7XMD5DS48EW4F8XSTC9OO.uasset
```

Actually `git status` shows the second as deleted:

```text
D Content/__ExternalActors__/MoDeng/Maps/L_Level01_Street/E/OW/X7XMD5DS48EW4F8XSTC9OO.uasset
```

These are likely user changes from editing the level in UE. Do not revert or commit them without user confirmation.

## Git Setup

- `.gitignore` exists and ignores:
  - `Binaries/`
  - `Intermediate/`
  - `Saved/`
  - `DerivedDataCache/`
  - `.vs/`

- `.gitattributes` exists and tracks `.uasset`, `.umap`, images, audio, video, etc. through Git LFS.
- Git LFS is installed and initialized.

Team clone instructions:

```bash
git clone https://github.com/xzh16689592007/TheGameOf2D.git
cd TheGameOf2D
git lfs pull
```

## Known Issues / Rough Edges

- Attack visual is currently a temporary cube, useful for debugging but visually ugly.
- Enemies are placeholder cube meshes.
- HUD is C++ Canvas HUD, not polished UMG.
- Enemy movement currently follows X axis only. This is fine for ground-level lanterns, but platform lanterns need route points, flying enemies, ranged enemies, or a 2.5D path system later.
- Debug messages still appear in the top-left for many events.
- If all lanterns are on platforms or unreachable by X-only enemies, enemies may not behave as intended.
- Current game lacks polished animations, hit feedback, sound, particles, and UI art.

## Suggested Next Steps

1. Push local HUD commit when network works:

```powershell
git push
```

2. Test HUD in UE:
   - Open `L_Level01_Street`.
   - Play.
   - Confirm the HUD shows wave, enemies, lanterns, weapon stats.

3. Decide whether to commit current map changes:
   - If the scene looks correct, commit the two `Content/__ExternalActors__/MoDeng/...` changes.
   - If not, inspect in UE first. Do not blindly revert unless user explicitly asks.

4. Replace temporary attack cube visual:
   - Option A: C++ spawn a translucent slash mesh.
   - Option B: Blueprint/Niagara slash VFX.
   - Option C: use existing mannequin attack montage if suitable.

5. Add better enemy visuals:
   - Different scale/color/material per enemy type.
   - Fast enemy smaller and darker.
   - Exploder enemy round or glowing red.

6. Add simple UMG start/end/result UI:
   - Victory screen.
   - Defeat screen.
   - Restart button.

7. Later: upgrade enemy movement beyond X-axis:
   - Route point system for platform levels.
   - Flying enemy.
   - Ranged enemy.
   - Climber enemy.

