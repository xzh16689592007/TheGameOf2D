# TheGameOf2D Project Handoff

## Project Basics

- Original project path used in earlier handoffs: `D:\UE_project\TheGameOf2D`
- Current local path used for this handoff: `C:\Users\26474\Documents\Unreal Projects\TheGameOf2D`
- Unreal project file: `TheGameOf2D.uproject`
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
  - Player repairs lanterns with `F`.
  - Player uses the current skill with `E`.
  - Player attacks enemies with left mouse button or `J`.
  - Killing enemies gives ink.
  - Ink upgrades weapon level automatically.
  - Spawner controls waves.
  - All lanterns extinguished means defeat.
  - Clearing all waves means victory.

## Important Existing Content

- Main custom level: `Content/MoDeng/Maps/L_Level01_Street.umap`
- Current second level: `Content/MoDeng/Maps/L_Level02_BridgeMarket.umap`
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
- Current player attack VFX folder: `Content/FX`
  - `Content/FX/NewNiagaraSystem.uasset`
    - Current prototype slash/ribbon effect used by Tomoe attack montages.
    - This should be renamed later to something descriptive such as `NS_Tomoe_SlashRibbon`.
  - `Content/FX/Particles`
    - `P_HitPoint.uasset`: Cascade hit-point flash used during early VFX testing.
    - `P_Trail.uasset`: Cascade trail asset; its emitter loop settings were changed so it no longer triggers the animation-notify infinite-loop warning, but the current active slash effect is Niagara.
  - `Content/FX/Particles/Materials`
    - `M_Trail_01.uasset` and `M_Trail_02.uasset` were repaired by assigning missing texture samples from the local FX texture folder.
    - `M_Trail_02` is currently the more useful ribbon material for the Niagara slash.
    - `M_Lightning.uasset`, `M_Flare.uasset`, and `M_Sprite.uasset` are available for future lightning, flash, or sprite particles.
  - `Content/FX/Particles/Textures`
    - Useful trail textures include `T_Lightning`, `T_Tile_Noise_Tendril_01`, and `T_Turbulence_Seamless`.
- Imported projectile / spell VFX folders:
  - `Content/PewPewPack`
    - Added in commit `8e67129`.
    - Used by enemy/Boss ranged projectile visuals.
    - Current C++ defaults:
      - Ranged wizard projectile: `Content/PewPewPack/ParticleSystems/Blaster/Par_Blaster_2.uasset`
      - Boss ranged volley projectile: `Content/PewPewPack/ParticleSystems/Helix/Par_Helix_03.uasset`
    - The full pack was committed through Git LFS because the referenced projectile systems depend on pack materials, meshes, textures, and vector fields.
  - `Content/FireEnergyVFX`
    - Added in commit `8e67129`.
    - Current C++ default Boss area-skill charge effect: `Content/FireEnergyVFX/Particles/P_PulseBeam.uasset`
    - The full pack was committed through Git LFS so dependent materials/textures stay available.
  - Do not commit `Content/Niagara_Effects05/Meshes/SM_box.Bat`; it is a tiny imported text/promotional file and is not a gameplay asset.
- Imported sword animation folder: `Content/Sword_Animations`
  - Source library path: `D:\UE素材库\Sword Animation Pack宝剑动画套装\Sword Animation Pack宝剑动画套装\Sword Animation Pack\Sword Animation Pack 5.1\Sword_Animations`
  - This is the currently preferred player attack source for the prototype because it has cleanly grouped ground combo sequence assets.
  - This pack includes its own UE4 mannequin source character:
    - `Content/Sword_Animations/Demo/Mannequin/Character/Mesh/SK_Mannequin.uasset`
    - `Content/Sword_Animations/Demo/Mannequin/Character/Mesh/UE4_Mannequin_Skeleton.uasset`
    - `Content/Sword_Animations/Demo/Mannequin/Character/Mesh/SK_Mannequin_PhysicsAsset.uasset`
    - `Content/Sword_Animations/Demo/Mannequin/Character/Mesh/Sword.uasset`
  - The pack does not appear to include UE5 IK Rig / IK Retargeter assets (`IKR_*`, `RTG_*`) by default, so a project-local source IK Rig was created for its `SK_Mannequin`.
  - Useful source attack assets:
    - `Content/Sword_Animations/Animations/Sequence/02_Attack/01_Combo_Attack_01/Combo_Attack_01_01_Seq.uasset`
    - `Content/Sword_Animations/Animations/Sequence/02_Attack/01_Combo_Attack_01/Combo_Attack_01_02_Seq.uasset`
    - `Content/Sword_Animations/Animations/Sequence/02_Attack/01_Combo_Attack_01/Combo_Attack_01_03_Seq.uasset`
    - `Content/Sword_Animations/Animations/Sequence/02_Attack/01_Combo_Attack_01/Combo_Attack_01_04_Seq.uasset`
    - `Content/Sword_Animations/Animations/Sequence/02_Attack/08_Attack_Air_to_Floor_01`
    - `Content/Sword_Animations/Animations/Sequence/02_Attack/09_Attack_Air_To_Floor_02`
    - `Content/Sword_Animations/Animations/Sequence/02_Attack/10_Attack_Air_To_Floor_03`
  - Retargeted Tomoe attack assets are under `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations`.
- Previously tested sword animation pack: `SwordAnimsetPro`
  - Source library path: `D:\UE素材库\SwordAnimsetPro\SwordAnimsetPro`
  - It was tested because `Attack_Combo_01_Anim` looked strong on its source mannequin, but the auto-retargeted Tomoe result was visibly distorted/squashed.
  - It is not currently part of the active player combo setup.
  - The project-local `Content/SwordAnimsetPro` folder and failed Tomoe retarget test folder under `Content/MoDeng/Animations/Tomoe/Attack/SwordAnimsetPro` were removed as unused test content.
- Current Tomoe player animation assets:
  - `Content/MoDeng/Animations/RTG_CombatKatana_To_Tomoe.uasset`
  - `Content/MoDeng/Animations/Retarget/IKR_SwordMannequin.uasset`
  - `Content/MoDeng/Animations/RTG_SwordMannequin_To_Tomoe.uasset`
  - `Content/MoDeng/Animations/Tomoe/ABP_Tomoe_SideScroller.uasset`
  - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/AM_Tomoe_GroundAttack_1.uasset`
  - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/AM_Tomoe_GroundAttack_2.uasset`
  - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/AM_Tomoe_GroundAttack_3.uasset`
  - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/AM_Tomoe_GroundAttack_4.uasset`
  - The four ground attack montages now have a `NewNiagaraSystem` animation notify for prototype slash/ribbon VFX.
  - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/Combo_Attack_02_01_Seq.uasset`
  - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/Combo_Attack_02_02_Seq.uasset`
  - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/Combo_Attack_02_03_Seq.uasset`
  - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/Combo_Attack_02_04_Seq.uasset`
  - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/Combo_Attack_02_All_Seq.uasset`
  - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AirToFloor_01/AM_Tomoe_AirToFloorAttack.uasset`
  - `AM_Tomoe_AirToFloorAttack` now also references the same prototype Niagara slash effect.
  - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AirToFloor_01/Attack_Air_To_Floor_Start_01_Seq.uasset`
  - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AirToFloor_01/Attack_Air_To_Floor_Loop_01_Seq.uasset`
  - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AirToFloor_01/Attack_Air_To_Floor_End_01_Seq.uasset`
  - `Content/MoDeng/Animations/Tomoe/Jump/Jump_Start_0_Seq.uasset`
  - `Content/MoDeng/Animations/Tomoe/Jump/Jump_Loop_0_Seq.uasset`
  - `Content/MoDeng/Animations/Tomoe/Jump/Jump_Stop_0_Seq.uasset`
  - `Content/MoDeng/Animations/Tomoe/Roll/AM_Tomoe_Roll.uasset`
  - `Content/MoDeng/Animations/Tomoe/Roll/Roll_F_0_Seq_Short.uasset`
  - `Content/MoDeng/Animations/Tomoe/Idle_Run/Idle_To_Idle_Combat_Seq.uasset`
  - `Content/MoDeng/Animations/Tomoe/Idle_Run/Idle_Combat_To_Idle_Seq.uasset`
  - `Content/MoDeng/Animations/Tomoe/Idle_Run/Idle_Combat_Seq.uasset`
  - `Content/MoDeng/Animations/Tomoe/Idle_Run/Idle_Seq.uasset`
  - `Content/MoDeng/Animations/Tomoe/Idle_Run/Run_Fast_Combat_Loop_Seq.uasset`
  - Old one-off retarget tests under `Content/MoDeng/Animations/Tomoe/Attack`, including `Anim_DK2_Combo_A1_IP` and `Anim_Katana_Attack_*`, were deleted intentionally because they are not used by the current combo.
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
  - Enemy melee damage is delayed by `AttackDamageDelay` after `PlayAttackAnimation()`, so basic/fast enemies no longer apply damage the same frame they enter range. The delayed hit stores the original target and rechecks range before applying damage, so moving out of range can make the attack miss.
  - Taking non-lethal damage plays a hit animation and still updates the health bar.
  - Dying plays the death animation sequence, disables collision immediately, then destroys after the animation/delay.
  - `EnemyBody` remains as a static mesh fallback and is hidden when skeletal visuals are enabled.
  - C++ now has a separate `EnemyWeaponMesh` skeletal component for held weapons, attached to `Weapon_R_Socket` or `hand_r`.
  - `FaceTargetLantern()` centralizes enemy facing so melee, ranged, and Boss attacks point toward the current lantern target before attacking.
  - `BP_ModengEnemy` previously used `SM_Stickman` on `EnemyBody`; C++ now prefers the modular skeleton setup at runtime.
  - C++ supports optional material color override and hit flash for placeholder visuals. For imported art, keep `bOverrideBodyMaterialColor` false so the model keeps its own material.

- `AModengFastEnemy`
  - Files:
    - `Source/TheGameOf2D/ModengFastEnemy.h`
    - `Source/TheGameOf2D/ModengFastEnemy.cpp`
  - Inherits `AModengEnemy`.
  - Faster, lower health, faster attack interval.
  - Uses modular skeleton visuals through `ApplyEnemyLoadout()`.
  - Current loadout was swapped to the heavier/brute modular skeleton look and now carries an axe.
  - Uses weapon idle/run/attack/hit/death animations so its attack reads as a weapon strike instead of a bare-hand hit.

- `AModengExploderEnemy`
  - Files:
    - `Source/TheGameOf2D/ModengExploderEnemy.h`
    - `Source/TheGameOf2D/ModengExploderEnemy.cpp`
  - Inherits `AModengEnemy`.
  - Overrides attack behavior.
  - Bursts near lantern, deals larger damage, then dies.
  - Uses modular skeleton visuals through `ApplyEnemyLoadout()`.
  - Current loadout was swapped to the lighter/fast unarmed modular skeleton look.
  - Spawns the native `AModengExplosionEffect` when it self-destructs.
  - Explosion now has an `ExplosionDelay` after the attack animation starts. The target is rechecked before damage so a player/lantern that leaves range during the windup is not hit by the delayed detonation.

- `AModengRangedEnemy`
  - Files:
    - `Source/TheGameOf2D/ModengRangedEnemy.h`
    - `Source/TheGameOf2D/ModengRangedEnemy.cpp`
  - Inherits `AModengEnemy`.
  - Uses the skeleton wizard mesh, wizard modular parts, staff, and spell-cast animation loadout.
  - Attacks lanterns from range by spawning `AModengMagicProjectile`.
  - Default projectile visual now uses PewPewPack `Par_Blaster_2`.
  - `ProjectileParticleSystem` and `ProjectileEffectScale` are exposed for tuning.

- `AModengMagicProjectile`
  - Files:
    - `Source/TheGameOf2D/ModengMagicProjectile.h`
    - `Source/TheGameOf2D/ModengMagicProjectile.cpp`
  - Projectile actor used by `AModengRangedEnemy`.
  - Travels toward the target lantern and applies impact damage/radius behavior on hit.
  - Now owns a Cascade `ProjectileEffectComponent`.
  - `InitializeProjectile()` accepts an optional `UParticleSystem` and scale so ranged enemies and Boss volleys can share movement/damage logic but use different visuals.
  - The old engine sphere mesh remains as a fallback if no particle system is supplied or a VFX asset fails to load.

- `AModengBossEnemy`
  - Files:
    - `Source/TheGameOf2D/ModengBossEnemy.h`
    - `Source/TheGameOf2D/ModengBossEnemy.cpp`
  - Inherits `AModengEnemy`.
  - Uses the Reaper mesh and Reaper idle/float/attack/death animations from `Content/Reaper`.
  - Basic attack is a scythe melee hit, not the wizard projectile behavior.
  - Has timed ranged projectile volleys, area explosions, and minion summoning in addition to melee.
  - Boss ranged volley now uses PewPewPack `Par_Helix_03` by default, separate from the wizard's Blaster projectile.
  - Boss area skill now has a charge/windup phase using FireEnergyVFX `P_PulseBeam` attached to the Boss before the ground impact executes.
  - Boss melee and normal attacks are paused while area-skill charging, preventing overlapping scythe and area-skill behavior.
  - Area-skill Niagara impact visuals were reduced for performance: default count is `3`, lifetime is `1.0s`, and spacing is `190`. Damage and radius were not reduced by this VFX optimization.
  - Has health-threshold damage reduction shields represented by the health-bar color only; do not re-add the old shield bubble visual unless the team explicitly wants it.
  - First time health drops below 50%, Boss restores/locks to 50%, becomes invulnerable, stops moving/attacking, spawns an expanding fire field, then ends with a final explosion. After this phase, normal Boss logic resumes.
  - Boss fire-field, area skill, final explosion, projectile, and scythe damage can now affect the player through `ApplyDamageToPlayer()` where appropriate, while still damaging lanterns.

- `AModengBossFireField`
  - Files:
    - `Source/TheGameOf2D/ModengBossFireField.h`
    - `Source/TheGameOf2D/ModengBossFireField.cpp`
  - Native C++ fire-field actor used by the Boss half-health phase.
  - Expands from `FireFieldStartRadius` to `FireFieldFinalRadius`.
  - Uses ground lava/scorch layers, multiple flame tongue meshes, embers, smoke puffs, and a flickering point light.
  - This is still a code-built prototype VFX, not Niagara. A real Niagara fire pack would be the best later polish upgrade.

- `AModengExplosionEffect`
  - Files:
    - `Source/TheGameOf2D/ModengExplosionEffect.h`
    - `Source/TheGameOf2D/ModengExplosionEffect.cpp`
  - Native reusable explosion actor.
  - Builds shockwave, core flash, fireball, smoke, scorch, ember meshes, and a short-lived point light from engine primitives/materials.
  - Used by exploder enemies, Boss area skills, and the Boss half-health final explosion.

- `AModengEnemySpawner`
  - Files:
    - `Source/TheGameOf2D/ModengEnemySpawner.h`
    - `Source/TheGameOf2D/ModengEnemySpawner.cpp`
  - Wave-based spawner.
  - Exposes `EnemyTypes`, `TotalWaves`, `BaseEnemiesPerWave`, `ExtraEnemiesPerWave`, `SpawnInterval`, `MaxAliveEnemies`, `DelayBetweenWaves`.
  - Recent teammate update adds `AModengRangedEnemy` to the default enemy type list if it is not already present.
  - Checks victory/defeat.
  - Shows C++ result UI on victory/defeat.
  - On Level 1 victory, shows a level-complete menu instead of immediately loading Level 2.
  - Level-complete menu allows restarting Level 1, entering Level 2, or quitting.
  - Supports spawning a Boss on the final wave / configured Boss encounter.
  - Exposes `OnGameEnded`, `OnVictory`, and `OnDefeat` delegates for Blueprint/UI/VFX hooks.
  - Restores gameplay input on BeginPlay as a safety net after restarting from the result screen.

- `UModengResultWidget`
  - Files:
    - `Source/TheGameOf2D/ModengResultWidget.h`
    - `Source/TheGameOf2D/ModengResultWidget.cpp`
  - Native C++ UMG result widget.
  - Shows Victory/Defeat text, or Level Complete text when Level 1 is cleared.
  - Provides Restart, optional Next Level, and Quit buttons.
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
    - `F` repairs/interacts with nearby lanterns/interactive actors.
    - Uses overlap query to find actors implementing `ISideScrollingInteractable`.
  - Skill:
    - `E` is routed by `ASideScrollingPlayerController::DoSkill()` into `ASideScrollingCharacter::DoSkill()`.
    - The skill can only start while grounded. It is blocked during jump/fall, roll, hit reaction, defeat, and while already releasing.
    - Ground attacks/sheathing can be interrupted into the skill; C++ stops the active attack/transition montage before entering the skill release state.
    - During skill release, movement, jump, drop, interact, attack, and roll inputs are ignored.
    - During skill release, `ApplyDamageToPlayer()` treats the player as invulnerable when `bSkillGrantsInvulnerability` is true.
    - Skill playback is configured by `SkillAnimation`, `SkillMontage`, `SkillSlotName`, play rate, and blend settings. For current setup, prefer assigning the sequence to `SkillAnimation` and leaving `SkillMontage` empty so sequence notifies fire reliably through the dynamic montage on `GroundAttackSlot`.
    - Current default skill assets live under `Content/MoDeng/Animations/Tomoe/Attack/Skill/`, including `AS_Combo_Attack_All_Seq` / `AS_Combo_Attack_All_Seq_Montage`; a shorter user-edited FBX/sequence variant may be used for the actual `SkillAnimation` override in `BP_SideScrollingCharacter`.
    - `SkillReleaseDuration` is only the fallback duration when no skill animation plays; otherwise C++ uses the animation duration.
    - `SideScrollingAnimNotify_SetSkillWeaponMode` is a native skill-only notify for weapon visibility during the skill. Place it on the skill AnimSequence:
      - `SkillHand`: hides normal hand sword and bone sword, shows `Sword_SkillHand`.
      - `SwordBone`: hides hand swords, shows `Sword_Bone` for the authored spinning/turning blade section.
      - `InScabbard`: restores sheathed state at the end of the skill by showing `Sword_InScabbard` and hiding `Sword_Hand`, `Sword_SkillHand`, and `Sword_Bone`.
    - `FinishSkillRelease()` also falls back to `InScabbard`, so a missed final notify should not leave the weapon in skill-hand/bone mode.
  - Attack:
    - `J` and left mouse button call `DoAttack()`.
    - Current ground player attack system is a C++ four-step combo driven by four independent Anim Montages:
      - `GroundAttack1Montage` -> `AM_Tomoe_GroundAttack_1`
      - `GroundAttack2Montage` -> `AM_Tomoe_GroundAttack_2`
      - `GroundAttack3Montage` -> `AM_Tomoe_GroundAttack_3`
      - `GroundAttack4Montage` -> `AM_Tomoe_GroundAttack_4`
    - C++ has default asset paths for those four montages under `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround`, but `BP_SideScrollingCharacter` can still override them.
    - `ABP_Tomoe_SideScroller` has `GroundAttackSlot` inserted after locomotion, and the four montages use `DefaultGroup.GroundAttackSlot`.
    - `ABP_Tomoe_SideScroller` root motion mode should be `Root Motion from Montages Only`.
    - The four source sequences `Combo_Attack_02_01_Seq` through `Combo_Attack_02_04_Seq` have root motion enabled so the split combo can preserve authored attack displacement through the montages.
    - Combo input is notify-driven:
      - `SideScrollingAnimNotify_OpenComboWindow` opens the input buffer.
      - Pressing attack during that window sets `bComboInputQueued`.
      - `SideScrollingAnimNotify_CloseComboWindow` closes the buffer.
      - `SideScrollingAnimNotify_CommitCombo` plays the next ground attack montage if input was queued.
      - `SideScrollingAnimNotify_FinishGroundAttack` starts sheath/restore when no next montage was committed.
      - `SideScrollingAnimNotify_OpenMoveCancelWindow` allows movement input to cancel the current ground attack into sheath/restore from the authored notify point.
      - `SideScrollingAnimNotify_LoopGroundCombo` lets the fourth ground attack consume queued attack input and restart at combo step 1 for another four-hit loop.
    - `FinishGroundAttack` now passes the source montage to C++ so an old montage's finish notify cannot accidentally sheath during the next combo step.
    - `BeginGroundAttackTrace` and `EndGroundAttackTrace` notifies define the authored damage window for each ground hit. `ComboStepIndex` must be 1, 2, 3, or 4 on the begin notify.
    - The fourth ground attack now supports looping back into step 1 when attack input is queued before `Loop Ground Combo`; otherwise it continues to `FinishGroundAttack` and sheathes.
    - Ground combo movement and jumping are blocked while the attack montage is in progress.
    - Air-to-floor attack now uses `AirToFloorAttackMontage` -> `AM_Tomoe_AirToFloorAttack` on `DefaultGroup.GroundAttackSlot`.
      - Montage sections must be named exactly `Start`, `Loop`, and `End`.
      - C++ starts at `Start`, loops `Loop` while airborne, and jumps to `End` when `Landed()` fires.
      - `End` has a timer fallback based on section length so the player does not get stuck if the montage end delegate is missed.
      - The old direct sequence playback path remains as a fallback if the montage asset is missing.
    - Air-to-floor attack still preserves downward motion and plays the end/landing impact when the character lands.
    - `SideScrollingAnimNotify_OpenGroundMoveCancelWindow` shows in UE as `OpenGroundMoveCancelWindow`; it is an alias for the older `Open Move Cancel Window` notify and allows movement to cancel the air-to-floor `End` recovery.
    - `OpenGroundMoveCancelWindow` is optional for basic recovery because C++ now auto-restores after the `End` section, but it is useful for making the recovery cancellable by movement.
    - Right mouse button calls `DoRoll()`.
    - Roll uses `RollMontage` / `AM_Tomoe_Roll` when the montage slot matches `GroundAttackSlot`; otherwise C++ falls back to a dynamic montage from `Roll_F_0_Seq_Short`.
    - Air rolls temporarily pause falling by saving `GravityScale`, setting gravity to `0`, and zeroing vertical velocity. Roll cleanup restores gravity.
    - Pressing attack during an air roll immediately interrupts the roll and starts the air-to-floor attack, without waiting for the roll cancel window.
    - Roll notifies include `BeginRollInvincible`, `EndRollInvincible`, `OpenRollCancelWindow`, and `FinishRoll`.
    - `Idle_Combat_To_Idle_Seq` is still used as the sheath/restore animation through `CombatToIdleAnimation`.
    - Sheath/restore now prefers dynamic montage playback on `CombatTransitionSlotName` instead of hard-switching the mesh to single-node animation.
    - `CombatTransitionSlotName`, `CombatTransitionBlendInTime`, `CombatTransitionBlendOutTime`, and `CombatTransitionAnimationPlayRate` control sheath/restore slot playback.
    - `ABP_Tomoe_SideScroller` now supports an `UpperBodyCombatSlot` layered over locomotion so sheath/restore can affect the upper body while legs keep running.
    - Sheathing can be interrupted by pressing attack again; C++ stops the active sheath montage and immediately starts a new ground or air-to-floor attack.
    - Movement is allowed during sheathing when `bAllowMovementDuringSheathing` is enabled or when `CombatTransitionSlotName` is `UpperBodyCombatSlot`.
    - Before sheath/restore playback, C++ restores `PlayerAnimClass` if the mesh is still in single-node animation mode from the air-to-floor attack. This avoids replaying the air-to-floor end animation instead of sheathing.
    - Current primary hit detection uses the katana trace points on `BP_SideScrollingCharacter`:
      - `KatanaTraceStart`
      - `KatanaTraceEnd`
    - During an active attack trace window, C++ sweeps the blade segment every frame and also sweeps from the previous frame's trace points to the current frame's trace points, so fast sword motion is still detected.
    - `bUseAutomaticWeaponMotionHitWindow` is now enabled by default for current combo steps. In this mode the animation is observed for the whole step, but damage only applies when the katana trace points move faster than `MinimumWeaponMotionSpeed` (default `180`). This fixed cases where a visible swing missed but the slow recovery/landing frames hit instead.
    - Debug trace drawing is available through `bDrawWeaponTraceDebug`. Cyan = weapon trace is active, red = trace hit something, silver = trace was observed but weapon speed was below the automatic motion threshold and therefore non-damaging.
    - `AttackHitWindowStartRatio` and `AttackHitWindowEndRatio` still exist as legacy/manual timing controls and are used when automatic/full-animation windows are disabled.
    - One enemy can only be damaged once per player attack through `HitEnemiesThisAttack`.
    - If the trace point components are missing, C++ falls back to the old forward box overlap so attacks do not completely break while debugging Blueprint setup.
    - The old yellow cylinder slash/range visual has been removed.
    - Applies light knockback to enemies that survive the hit.
    - Repeated attack input is blocked outside the active combo window, so rapid clicking cannot repeatedly apply damage or skip steps.
    - C++ records the mesh relative transform before the attack animation and restores it afterward via `bRestoreMeshTransformAfterAttackAnimation`, to reduce root/retarget offset issues.
    - Movement input no longer interrupts ground attacks until `Open Move Cancel Window` has fired. After that notify, movement cancels the active attack into sheath/restore and the same move input continues into locomotion.
    - If the combo input window feels too strict, tune the `Open Ground Combo Window`, `Close Ground Combo Window`, `Commit Ground Combo`, and `Loop Ground Combo` notify positions in the relevant montage rather than changing C++ first.
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
- Jump: template jump input. `JumpMaxCount = 2`, so the player has one air jump.
- Skill: `E`.
- Repair/interact: `F`.
- Attack: left mouse button or `J`.
- Roll/dodge: right mouse button.

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
  - Player combo attack upgrade current state:
    - `Content/Sword_Animations` has been imported into the project and is the current preferred attack animation source.
    - Old `Combo_Attack_01_*` direct sequence combo logic was removed because the transitions felt unnatural.
    - Current ground combo uses the second Sword_Animations ground set:
      - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/Combo_Attack_02_01_Seq.uasset`
      - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/Combo_Attack_02_02_Seq.uasset`
      - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/Combo_Attack_02_03_Seq.uasset`
      - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/Combo_Attack_02_04_Seq.uasset`
      - `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/Combo_Attack_02_All_Seq.uasset` exists as the authored full-combo reference but is not the active runtime montage.
    - User created four independent ground attack montages:
      - `AM_Tomoe_GroundAttack_1`
      - `AM_Tomoe_GroundAttack_2`
      - `AM_Tomoe_GroundAttack_3`
      - `AM_Tomoe_GroundAttack_4`
    - The four montage assets use `DefaultGroup.GroundAttackSlot`; `ABP_Tomoe_SideScroller` contains `GroundAttackSlot` after locomotion.
    - Root motion is enabled on the four `Combo_Attack_02_0*_Seq` sequences, and AnimBP root motion mode was set to `Root Motion from Montages Only`.
    - C++ now exposes `GroundAttack1Montage` through `GroundAttack4Montage` and default-loads the four `AM_Tomoe_GroundAttack_*` assets from `AttackInGround`.
    - The native notifies under `Source/TheGameOf2D/Variant_SideScrolling/Animation` drive the combo windows, attack traces, combo commit, and finish/sheath.
    - User placed notifies on all four montages. Step 1-3 use open/close/commit/finish; step 4 uses damage window and finish.
    - New direct Sword_Animations retarget setup was created after reviewing the package structure:
      - Source IK Rig: `Content/MoDeng/Animations/Retarget/IKR_SwordMannequin.uasset`
      - Source preview mesh: `Content/Sword_Animations/Demo/Mannequin/Character/Mesh/SK_Mannequin.uasset`
      - Target IK Rig: `Content/SamuraiGirlTomoe/Mesh/IKR_Tomoe.uasset`
      - Target preview mesh: `Content/SamuraiGirlTomoe/Mesh/SK_SAMURAIGIRL_01.uasset`
      - Retargeter: `Content/MoDeng/Animations/RTG_SwordMannequin_To_Tomoe.uasset`
    - `IKR_SwordMannequin` and `IKR_Tomoe` both have `pelvis` set as the retarget pelvis. In the current Chinese UE 5.7 UI this appears as `设置骨盆` / `骨盆: pelvis`, not necessarily as older tutorial wording like `Set Retarget Root`.
    - Retarget chains should exist on both IK Rigs for at least `Pelvis`, `Spine`, `Neck`, `Head`, `LeftClavicle`, `RightClavicle`, `LeftArm`, `RightArm`, `LeftLeg`, and `RightLeg`. Finger chains can remain if auto-created.
    - The `Root: root -> root` chain is optional and can trigger `IK重定向器骨骼链Root无法标准化。链过短。` if present. Do not delete all chains; only remove/ignore the problematic single-bone root chain if that warning matters.
    - UE 5.7 retargeter root-motion warnings were fixed in the Retargeter Operation Stack, not in the right-side Asset Settings panel:
      - Operation Stack includes `Pelvis Motion`, `FK Chains`, `Run IK Rig`, nested `Retarget IK Goals`, `Root Motion`, and `Remap Curves`.
      - Select the `Root Motion` operation and set `源根 = root`, `目标根 = root`, and `目标骨盆 = pelvis`.
      - After this, the retarget output log showed only the success line: `IK重定向器准备将从源文件SK_Mannequin将动画转换目标SK_SAMURAIGIRL_01`.
    - If `隐藏未重定向的骨骼` only shows `pelvis`, the chain mapping is missing or stale. Save both IK Rigs, reopen/recreate the Retargeter, and map source chains to same-named target chains. Tomoe will not move in preview if only the source mannequin is animated and the chains are not mapped.
    - Test retargeting with `Sword_Animations` source animations first, not `A_Stickman_*` assets from another package, so source skeleton issues do not get mixed with chain-mapping issues.
    - `SwordAnimsetPro` was also imported and tested. Its `Attack_Combo_01_Anim` looked good on the source mannequin, but auto-retargeted Tomoe versions looked distorted/squashed, so the project-local test assets were removed and it should not be used as the main combo source without a properly tuned retargeter.
    - The user re-exported the Sword_Animations Tomoe idle/run set under `Content/MoDeng/Animations/Tomoe/Idle_Run`, including `Idle_To_Idle_Combat_Seq`, `Idle_Combat_To_Idle_Seq`, `Idle_Combat_Seq`, `Idle_Seq`, and `Run_Fast_Combat_Loop_Seq`.
    - A full build after the four-montage reflected C++ update succeeded on 2026-06-12 with `Result: Succeeded`.
    - Combo-window tuning should happen in the montage timeline by moving `Open Ground Combo Window`, `Close Ground Combo Window`, and `Commit Ground Combo` notifies.
    - A single whole-combo montage approach using `Combo_Attack_02_All_Seq` was discussed. It is useful as a pose/timing reference, but the active implementation is four separate montages so each hit can independently end into sheath or chain to the next hit.
  - Player sword/scabbard hookup:
    - Originally used `Content/CombatMasterAnimBundle/Weapon/Katana/SM_Katana.uasset`; the user later switched the visible weapon mesh to the `Sword_Animations` pack sword to match the current animation source.
    - Current weapon art comes from `Content/Weapon/Sci-fi_Swords_Pack_1`.
      - Dark long blade meshes: `SM_Dark_Long_Blade_Sword` and `SM_Dark_Long_Blade_Sheath`.
      - Frozen sci-fi sword meshes are also imported for later use.
      - The pack uses shared master material `M_Sci-fi_Swords_Pack_1_Master` and per-weapon material instances such as `MI_Dark_Long_Blade`; if material preview is gray, verify the material instance parent and texture parameters.
    - Add/use a right-hand socket named `Socket_Katana_R` on `hand_r`.
    - `BP_SideScrollingCharacter` now uses a three-component sword display setup:
      - `Sword_Hand` is the hand-held sword for combat/attacks.
      - `Sword_InScabbard` is the fake sheathed/waist sword placed inside the scabbard; it is visible when the weapon is not drawn and hidden while attacking/drawn.
      - `Sword_Sheathed` is the scabbard component; it should stay visible in both drawn and sheathed states.
      - `Sword_SkillHand` is a skill-only hand sword shown by `SideScrollingAnimNotify_SetSkillWeaponMode`.
      - `Sword_Bone` is a skill-only bone/socket-following sword used during the turning/spinning section of the skill.
      - All weapon/scabbard static mesh components should have collision disabled.
      - C++ exposes `SetSceneComponentVisibleByName()` and `SetCombatWeaponDrawnForNotify()` for AnimNotify Blueprints to switch weapon visibility at authored frames.
      - C++ exposes `SetSkillWeaponModeForNotify()` for the native skill weapon notify; use this for the E skill instead of reusing regular attack draw/sheath notifies.
      - `AN_ShowSheathedSword` should call `SetCombatWeaponDrawnForNotify(false)` if the sheathed sword needs to appear before the transition montage naturally ends.
    - Add/keep `KatanaTraceStart` and `KatanaTraceEnd` scene components as children of the hand-held weapon component (`Sword_Hand` / previous `Katana`).
    - `KatanaTraceStart` should sit near the blade root/guard.
    - `KatanaTraceEnd` should sit near the blade tip.
    - Damage comes from C++ sweep traces between those two points during the attack hit window, not from the weapon mesh's own collision.
    - User tested the final behavior: blade contact now applies damage, rapid clicking no longer causes repeated instant damage, and the temporary yellow slash/range visual was removed.
  - Recommended attack source to continue with: current retargeted `Sword_Animations` ground combo under `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations`.
- Enemy health bar issue was fixed defensively in C++:
  - Added `AModengEnemy::EnsureHealthBarWidget()`.
  - Health bar widget class, instance, color, and percent are revalidated before initialize/update/show.
  - User tested later waves and reported the fix works.

## Build Commands

Use this full build command when UE is closed:

```powershell
& "D:\UE_5.7\Engine\Build\BatchFiles\Build.bat" TheGameOf2DEditor Win64 Development -Project="D:\UE_project\TheGameOf2D\TheGameOf2D.uproject" -WaitMutex
```

Current local machine command used for this handoff:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" TheGameOf2DEditor Win64 Development -Project="C:\Users\26474\Documents\Unreal Projects\TheGameOf2D\TheGameOf2D.uproject" -WaitMutex
```

If UE is open and Live Coding is active, full build will fail with:

```text
Unable to build while Live Coding is active
```

In that case either:

- Press `Ctrl + Alt + F11` inside UE for Live Coding, or
- Close UE and run full build.

Important: after adding new C++ classes, prefer closing UE and running a full build before reopening. Otherwise Blueprints can temporarily show parent class missing.

Very important Live Coding note:

- Do not use Live Coding for changes to reflected Unreal types:
  - `.h` changes
  - `UCLASS`
  - `USTRUCT`
  - `UPROPERTY`
  - `UFUNCTION`
  - new/delete/renamed Blueprint-visible fields
  - function signature changes used by Unreal reflection
- During the player combo work, Live Coding produced `TheGameOf2D_patch_0` crashes on editor shutdown and caused `Attack Combo Steps` to disappear until a full build was run.
- For reflected changes, save assets, close UE completely, confirm no `UnrealEditor`, `LiveCodingConsole`, or `CrashReportClientEditor` process remains, then run the full build command above.
- It is generally safe to use Live Coding only for `.cpp` logic changes that do not alter reflected class/struct layout.

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
8e67129 Update enemy attack timing and projectile effects
f0a53db Update boss effects and enemy spawn handling
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

Latest synced gameplay milestone: commit `8e67129` is pushed to `origin/main`. It adds enemy/Boss ranged VFX resources, fixes enemy attack damage timing, adds Boss area-skill windup VFX, reduces Boss area-skill particle density, and includes the required VFX packs through Git LFS.

Current enemy/Boss update in this handoff:

- Added full `Content/PewPewPack` and `Content/FireEnergyVFX` imports through Git LFS.
- Ranged wizard projectile defaults to PewPewPack `Par_Blaster_2`.
- Boss ranged volley defaults to PewPewPack `Par_Helix_03`.
- `AModengMagicProjectile` now supports a particle-system visual component and keeps its old sphere mesh as fallback.
- Basic and fast melee enemies now delay damage with `AttackDamageDelay` instead of damaging immediately on entering attack range.
- Exploder enemies now delay detonation with `ExplosionDelay`; the target is rechecked at detonation time.
- Boss area skill now has a visible charge using `P_PulseBeam`, and Boss melee/ranged behavior is blocked while the area skill charges.
- Boss area-skill Niagara impact density was reduced for performance: `AreaSkillEffectCount = 3`, `AreaSkillNiagaraEffectLifetime = 1.0`, `AreaSkillEffectSpacing = 190`.
- Full build succeeded on 2026-06-24 with:
  - `Result: Succeeded`
- Push uploaded `530` Git LFS objects, about `308 MB`, in commit `8e67129`.

Current player update in this handoff:

- Added player double jump in C++ (`JumpMaxCount = 2`, custom second-jump launch using `DoubleJumpVerticalMultiplier`).
- Added right mouse roll/dodge:
  - `Roll_F_0_Seq_Short` and `AM_Tomoe_Roll` under `Content/MoDeng/Animations/Tomoe/Roll`.
  - Native roll notifies for invincibility, cancel window, and finish.
  - Air roll pauses falling and restores gravity on all roll cleanup paths.
  - Air roll can be interrupted by attack at any time to start the air-to-floor attack.
- Added `AM_Tomoe_AirToFloorAttack` as a three-section montage (`Start`, `Loop`, `End`) under `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AirToFloor_01`.
- C++ now drives air-to-floor attack through that montage:
  - Plays `Start`, loops `Loop` while airborne, jumps to `End` on landing, then restores/sheathes.
  - `End` section has a timer fallback to prevent the player from getting stuck after landing.
  - Direct sequence playback remains as fallback if the montage cannot play.
- Added `SideScrollingAnimNotify_OpenGroundMoveCancelWindow`, displayed in UE as `OpenGroundMoveCancelWindow`, as a searchable alias for the existing movement-cancel notify.
- `OpenGroundMoveCancelWindow` / `Open Move Cancel Window` now also works during the air-to-floor `End` phase.
- Added/updated Tomoe jump assets under `Content/MoDeng/Animations/Tomoe/Jump`.
- Added prototype player attack VFX:
  - Created `Content/FX/NewNiagaraSystem.uasset` as a simple Niagara ribbon/slash system.
  - Added the Niagara system to `AM_Tomoe_GroundAttack_1` through `AM_Tomoe_GroundAttack_4`.
  - Added the same Niagara system to `AM_Tomoe_AirToFloorAttack`.
  - Imported/reused `Content/FX/Particles` Cascade assets, materials, and textures as source VFX resources.
  - Repaired `M_Trail_01` / `M_Trail_02` by assigning missing local FX textures; `M_Trail_02` is the current practical ribbon material.
  - `P_HitPoint` was verified as a visible simple particle notify option; `P_Trail` is less useful for the current Montage setup than the Niagara ribbon.
- Updated the Tomoe weapon preview/runtime setup:
  - Added `Sword_start`, `Sword_end`, and `Show` sockets/preview helpers on `SKEL_Tomoe_Skeleton`.
  - `BP_SideScrollingCharacter` now references the Sci-fi Frozen Sword assets for the visible weapon/scabbard setup.
  - The weapon still uses C++ trace components for damage; the mesh itself should remain non-colliding.
- Added current E skill prototype:
  - `E` triggers a grounded-only skill release; `F` is now the repair/interact key.
  - Skill release locks player input and grants invulnerability while active.
  - `SkillAnimation` / `SkillMontage` / `SkillSlotName` are exposed on `BP_SideScrollingCharacter`; prefer setting the skill sequence on `SkillAnimation` and leaving `SkillMontage` empty while tuning notifies.
  - Added native `SideScrollingAnimNotify_SetSkillWeaponMode` with `SkillHand`, `SwordBone`, and `InScabbard` modes for the authored skill weapon handoff.
  - Current skill weapon component plan: normal combat uses `Sword_Hand`; skill startup uses `Sword_SkillHand`; skill turning section uses `Sword_Bone`; skill end returns to `Sword_InScabbard`.
- Added Blender helper scripts under `Tools/Blender` for experimenting with current-action/root-motion editing:
  - `scale_root_motion.py`
  - `edit_current_action_root_motion.py`
  - `rewrite_fbx_root_motion.py`
  - These are workflow helpers only; verify edited animations in Blender/UE before replacing project assets.
- A full build after these C++ changes succeeded on 2026-06-25 with:
  - `Result: Succeeded`
- `outputs/` and `Week2_Report_Modeng.docx` are local report-generation artifacts and should not be included in gameplay commits.

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
- `Content/PewPewPack` and `Content/FireEnergyVFX` are now committed through Git LFS. Fresh clones must run `git lfs pull` before opening the project, otherwise enemy/Boss VFX assets will be missing.
- `Config/DefaultEditor.ini` may get noisy editor preview-profile changes from opening imported assets. Do not commit those preview-profile changes unless the team explicitly wants editor profile settings in source control.
- Large Tomoe and CombatMaster imports were pushed through Git LFS in commit `9fcc0c0` (`878` LFS objects, about `1.1 GB` uploaded during push).
- PewPewPack / FireEnergyVFX imports were pushed through Git LFS in commit `8e67129` (`530` LFS objects, about `308 MB` uploaded during push).
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
- Ranged enemy exists and uses `AModengMagicProjectile` with PewPewPack Blaster visuals; tune projectile speed, impact radius, spawn offset, effect scale, and wave pacing after playtesting.
- Boss second-level encounter now exists in C++, but still needs PIE tuning for damage numbers, skill cadence, fire-field radius, and readability. Area-skill impact VFX have already been reduced for performance, but may need further tuning on lower-end machines.
- Boss fire-field visual is code-built from primitive meshes/materials. It is improved over the first prototype, but Niagara/fire texture VFX would be a better production-quality solution.
- Enemy and Boss attacks now route player damage through `ASideScrollingCharacter::ApplyDamageToPlayer()` where implemented, while still supporting lantern damage. Continue checking every new enemy skill against both player and lantern targets.
- HUD is C++ Canvas HUD, not polished UMG.
- Player is now visually migrated from default Manny/Quinn appearance to Samurai Girl Tomoe for the current prototype.
- Player attack now uses the new four-montage ground combo path, but it still needs in-editor feel tuning:
  - Current ground attacks are four independent `AM_Tomoe_GroundAttack_*` montages on `DefaultGroup.GroundAttackSlot`.
  - The four `Combo_Attack_02_0*_Seq` assets have root motion enabled, and `ABP_Tomoe_SideScroller` should stay on `Root Motion from Montages Only`.
  - Combo chaining is driven by native montage notifies, not by old `AttackComboSteps` or direct `PlayAnimation()` ground combo code.
  - Current priority: validate the new cancel/loop notify timing in PIE: movement should only cancel after `Open Move Cancel Window`, and attack queued before `Loop Ground Combo` on step 4 should restart at step 1.
  - Keep `Combo_Attack_02_All_Seq` only as a visual timing/pose reference for natural section-to-section connection.
  - Attack damage timing for ground combo is authored by `BeginGroundAttackTrace` / `EndGroundAttackTrace` notifies. Air-to-floor attack currently opens the attack trace window from C++ and closes it on landing.
  - Air-to-floor landing should transition into `AM_Tomoe_AirToFloorAttack` section `End`, then sheath/restore. If this regresses, first inspect section names (`Start`, `Loop`, `End`) and slot (`DefaultGroup.GroundAttackSlot`) before changing C++.
  - Air-to-floor `End` movement cancel can be tuned by placing `OpenGroundMoveCancelWindow` / `Open Move Cancel Window` in the montage timeline.
  - Tomoe base locomotion now has idle/run/jump-start/fall-loop. Landing polish is still not implemented as a dedicated state.
  - Use only Tomoe-compatible or Tomoe-retargeted animation assets in `ABP_Tomoe_SideScroller`; using the modular skeleton enemy jump animations directly causes body deformation.
- Visible weapon setup now uses the Sci-fi Frozen Sword look in the player Blueprint / Tomoe preview setup. Socket/rotation and trace point placement may still need tuning if future attack animations change the blade path.
- Player attack VFX now exists as a prototype Niagara slash ribbon on the ground and air-to-floor attack montages. It is still an artist-tuning pass, not final polish: rename `NewNiagaraSystem`, tune ribbon color/width/lifetime, and validate readability in PIE.
- Enemy movement currently follows X axis only. This is fine for ground-level lanterns, but platform lanterns need route points, flying enemies, ranged enemies, or a 2.5D path system later.
- Most debug messages now have exposed `bShowGameplayDebugMessages` toggles and are off by default.
- If all lanterns are on platforms or unreachable by X-only enemies, enemies may not behave as intended.
- Current game still lacks polished sound, final particles, and UI art.

## Suggested Next Steps

1. Playtest the second-level Boss encounter:
   - Confirm the Reaper Boss uses scythe melee, ranged volley, area explosion, summon, and half-health fire-field phase.
   - Confirm the Boss ranged volley uses the Helix projectile effect and the wizard ranged enemy uses the Blaster projectile effect.
   - Confirm the Boss area-skill charge effect is readable without obscuring the Boss.
   - Confirm the reduced area-skill impact count performs better and still communicates the danger zone.
   - Tune `HalfHealthInvulnerableDuration`, `FireFieldStartRadius`, `FireFieldFinalRadius`, `FireFieldDamagePerSecond`, and `FireFieldFinalExplosionDamage`.
   - Confirm the half-health phase only triggers once and the Boss resumes normal behavior afterward.
   - Confirm the blue health bar still communicates damage reduction without any shield bubble visual.

2. Validate Level 1 clear flow:
   - Clear Level 1 and confirm the menu offers Restart, Next Level, and Quit.
   - Confirm Next Level opens `L_Level02_BridgeMarket`.
   - Confirm Restart reloads the current level with gameplay input restored.

3. Validate Level 2 spawn/camera:
   - Start Level 2 after the shifted player spawn point.
   - Confirm the camera initially centers on the possessed character and does not leave the player off-screen on the far left.
   - If the level grows further left than `-3000`, tune `CameraXMinBounds` on `ASideScrollingCameraManager`.

4. Recheck fast/exploder readability:
   - Fast enemy should read as the axe-carrying brute.
   - Exploder enemy should read as the lighter unarmed unit and should spawn the explosion effect when it detonates.
   - Confirm basic/fast melee hits now land after the visible attack windup, not immediately on contact.
   - Confirm exploder windup and delayed explosion feel fair; tune `ExplosionDelay` if it feels too slow or too instant.
   - If the names feel counterintuitive after the model swap, tune scale/color/animation speed so gameplay roles stay obvious.

5. Validate sheath movement and interrupt behavior:
   - Confirm `ABP_Tomoe_SideScroller` routes the cached full-body combat pose through `UpperBodyCombatSlot` into a layered blend per bone.
   - Confirm `BP_SideScrollingCharacter.CombatTransitionSlotName = UpperBodyCombatSlot`.
   - Attack once, then hold movement during sheath: the legs should keep running instead of sliding or standing still.
   - Attack again during sheath: sheath should stop and a new attack should begin immediately.
   - Test air-to-floor attack landing: it should play the landing impact/end, then sheath, without replaying the landing attack end in place.

6. Validate roll and air-to-floor attack:
   - Right mouse should play `AM_Tomoe_Roll`.
   - Air roll should pause falling briefly and then resume gravity when roll ends.
   - Press attack during air roll: roll should interrupt immediately into air-to-floor attack.
   - Air-to-floor attack should play `Start`, loop `Loop` while airborne, jump to `End` on landing, then restore movement.
   - Searchable movement-cancel notify names are `OpenGroundMoveCancelWindow` and `Open Move Cancel Window`; place one in the latter half of the air-to-floor `End` section if recovery should be cancellable.
   - If the character gets stuck after landing, check that `AM_Tomoe_AirToFloorAttack` has exact section names `Start`, `Loop`, `End` and uses `DefaultGroup.GroundAttackSlot`.

7. Clean up combat debug visuals for normal playtests:
   - Disable `bDrawWeaponTraceDebug` unless actively debugging blade contact.
   - Cyan/red/silver weapon traces are useful for tuning, but they dominate gameplay footage.

8. Tune the four-montage ground combo cancel and loop windows:
   - Open `Content/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/AM_Tomoe_GroundAttack_1.uasset` through `AM_Tomoe_GroundAttack_4.uasset`.
   - Confirm all four use `DefaultGroup.GroundAttackSlot`.
   - Confirm `ABP_Tomoe_SideScroller` root motion mode is `Root Motion from Montages Only`.
   - Press attack once: step 1 should play and then sheath.
   - Press attack during the notify window: step 1 should commit into step 2, then 3, then 4.
   - Move after `Open Move Cancel Window`: the active attack should cancel into sheath/restore and locomotion should continue.
   - On step 4, press attack before `Loop Ground Combo`: the combo should restart at step 1. If no input is queued, step 4 should still reach `Finish Ground Attack` and sheath.
   - If input feels too strict, tune notify positions first, not C++.
   - Use `Combo_Attack_02_All_Seq` as the reference for natural body-pose connection points.

9. Polish draw/sheath flow:
   - Confirm `CombatToIdleAnimation` on `BP_SideScrollingCharacter` points to `Content/MoDeng/Animations/Tomoe/Idle_Run/Idle_Combat_To_Idle_Seq.uasset`.
   - Keep the initial sword state as sheathed: `Sword_Hand` hidden, `Sword_Sheathed` visible, and `Sword_InScabbard` visible.
   - `Sword_Sheathed` is the visible scabbard and should remain visible; `Sword_InScabbard` is the fake sword inside it and is hidden while the hand sword is drawn.
   - `Sword_SkillHand` and `Sword_Bone` are skill-only sword display components controlled by `SideScrollingAnimNotify_SetSkillWeaponMode`.
   - Tune the `AN_ShowSheathedSword` notify position in `Idle_Combat_To_Idle_Seq` to control when the sheathed sword reappears during the return-to-idle animation.
   - For regular draw/sheath visibility, call `SetCombatWeaponDrawnForNotify(false)` or `SetSceneComponentVisibleByName()` on `BP_SideScrollingCharacter`.
   - For the E skill, prefer the native `SetSkillWeaponMode` notify modes: `SkillHand`, `SwordBone`, then `InScabbard`.

10. Polish attack hit feedback and VFX:
   - Add short hit stop on successful player melee hit.
   - Add enemy hit flash/material flash.
   - Add sword whoosh and hit SFX.
   - Rename `Content/FX/NewNiagaraSystem.uasset` to a descriptive player slash name, then update all Montage references.
   - Tune the existing Niagara ribbon/slash on `AM_Tomoe_GroundAttack_1` through `AM_Tomoe_GroundAttack_4` and `AM_Tomoe_AirToFloorAttack`.
   - Keep `P_HitPoint` as a simple Cascade hit-flash fallback or migrate the hit flash to Niagara later.

11. Tune ground attack hit windows:
   - `BeginGroundAttackTrace` should be near visible blade contact.
   - `EndGroundAttackTrace` should be after the blade leaves the hit area.
   - Set `ComboStepIndex` on each begin notify: 1, 2, 3, 4.
   - Enable `bDrawWeaponTraceDebug` temporarily if a visible blade pass misses.

12. Polish Tomoe jump/fall locomotion:
   - Current `JumpStart` / `FallLoop` split is working.
   - Add a dedicated `Land` state only if it does not make movement feel sticky.
   - Tune transition blend duration between `JumpStart` and `FallLoop` if the pose pops.
   - Keep using Tomoe-compatible or Tomoe-retargeted jump/fall animations; do not use enemy skeleton animations directly.

13. Continue polishing Tomoe player setup:
   - Open `BP_SideScrollingCharacter`.
   - Confirm `CharacterMesh0` and class default `PlayerSkeletalMesh` both point to `SK_SAMURAIGIRL_01`.
   - Confirm class default `PlayerAnimClass` points to `ABP_Tomoe_SideScroller`.
   - Confirm `GroundAttack1Montage` through `GroundAttack4Montage` point to the four `AM_Tomoe_GroundAttack_*` assets, or rely on the C++ default paths.
   - Confirm `Lock Facing To Side Scrolling Axis` is set as desired.
   - If ground attacks move too far or snap, check the root motion settings on the four `Combo_Attack_02_0*_Seq` assets and the AnimBP root motion mode.
   - For reflected C++ changes, close UE and full-build instead of using Live Coding.

14. Tune the visible katana/weapon:
   - Verify `Socket_Katana_R` is on Tomoe's real `hand_r` bone, not `ik_hand_r`.
   - Verify the visible hand weapon in `BP_SideScrollingCharacter` uses the intended Sci-fi Frozen Sword mesh, parent socket `Socket_Katana_R`, and `NoCollision`.
   - Verify `KatanaTraceStart` / `KatanaTraceEnd` or the current `Sword_start` / `Sword_end` helper points remain aligned with the blade root and tip.
   - If a new attack animation misses despite visible blade contact, tune `AttackHitWindowStartRatio`, `AttackHitWindowEndRatio`, and `WeaponTraceRadius` first.
   - Tune socket transform in `SKEL_Tomoe_Skeleton` until the grip sits naturally in the right hand.
   - Keep C++ hit detection separate from weapon mesh collision for now.

15. Later: give fast and exploder enemies distinct polished visuals:
   - Fast enemy can be smaller/lighter/faster-looking.
   - Exploder enemy can be larger/redder/more volatile-looking.
   - They currently remain acceptable placeholders because their shapes distinguish enemy types.

16. Later: upgrade enemy movement beyond X-axis:
   - Route point system for platform levels.
   - Flying enemy.
   - Ranged enemy.
   - Climber enemy.
