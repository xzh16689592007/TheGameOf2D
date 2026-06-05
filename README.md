# TheGameOf2D

`TheGameOf2D` is an Unreal Engine 5.7 2.5D side-view action prototype. The current prototype is a lantern-defense game: enemies advance toward lit lanterns, the player repairs lanterns, defeats enemies, earns ink, and survives wave-based attacks.

## Game Concept

Working title: `墨灯守巷`

The game uses 3D assets in a constrained side-scrolling play space. The core loop is:

- Protect lanterns from incoming enemies.
- Repair damaged lanterns with the interact key.
- Attack enemies to earn ink.
- Gain weapon upgrades automatically through ink.
- Survive all enemy waves to win.
- Lose if every lantern is extinguished.

## Current Features

- 2.5D side-scrolling player movement.
- Wave-based enemy spawning.
- Lantern durability, damage, repair, and extinguish logic.
- Basic, fast, and exploder enemy types.
- Basic Stickman enemy skeletal mesh visuals.
- Basic enemy idle, walk, attack, hit, and death animation hooks.
- Enemy health bars.
- Lantern repair prompt.
- C++ HUD showing wave, enemy, lantern, weapon, ink, damage, and range status.
- Victory and defeat result UI with restart and quit actions.
- Git LFS setup for Unreal binary assets.

## Controls

- Move: side-scrolling movement input
- Jump: template jump input
- Repair / interact: `E` or `F`
- Attack: left mouse button or `J`

## Main Project Files

- Unreal project: `TheGameOf2D.uproject`
- Main level: `Content/MoDeng/Maps/L_Level01_Street.umap`
- Main custom content folder: `Content/MoDeng`
- Imported Stickman asset folder: `Content/ROG_Creatures`
- Project handoff notes: `PROJECT_HANDOFF.md`

Important C++ gameplay classes:

- `AModengLantern`
- `AModengEnemy`
- `AModengFastEnemy`
- `AModengExploderEnemy`
- `AModengEnemySpawner`
- `AModengHUD`
- `UModengEnemyHealthWidget`
- `UModengResultWidget`
- `ASideScrollingCharacter`

## Requirements

- Unreal Engine 5.7
- Visual Studio 2026 with MSVC 14.44
- Git LFS

Before opening the project after cloning, install and pull LFS assets:

```bash
git lfs install
git lfs pull
```

## Clone

```bash
git clone https://github.com/xzh16689592007/TheGameOf2D.git
cd TheGameOf2D
git lfs pull
```

Then open:

```text
TheGameOf2D.uproject
```

## Build

When Unreal Editor is closed, build the editor target with:

```powershell
& "D:\UE_5.7\Engine\Build\BatchFiles\Build.bat" TheGameOf2DEditor Win64 Development -Project="D:\UE_project\TheGameOf2D\TheGameOf2D.uproject" -WaitMutex
```

If Unreal Editor is open and Live Coding is active, a full command-line build may fail with:

```text
Unable to build while Live Coding is active
```

In that case, either press `Ctrl + Alt + F11` in Unreal Editor for Live Coding, or close the editor and run the full build.

## Development Notes

- Core gameplay logic is implemented primarily in C++.
- Blueprints are used for assets, tuning, level placement, visuals, UI hooks, materials, VFX, and animation setup.
- The game currently uses 3D assets without Paper2D/PaperZD.
- Gameplay movement is intentionally constrained to a side-scrolling plane.
- Fast and exploder enemies still use placeholder visuals.
- The player character still needs a more polished model and animation set.

For detailed continuation notes, read `PROJECT_HANDOFF.md` before starting new work.

