// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SideScrollingCharacter.generated.h"

class UCameraComponent;
class UAnimInstance;
class UAnimMontage;
class UAnimSequenceBase;
class UInputAction;
class USceneComponent;
class USkeletalMesh;
class USoundBase;
class AModengEnemy;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSideScrollingPlayerDeathFinishedSignature);

enum class ESideScrollingCombatAnimationPhase : uint8
{
	None,
	Attacking,
	Sheathing,
	HitReact,
	AirToFloorStart,
	AirToFloorLoop,
	AirToFloorEnd
};

/**
 *  A player-controllable character side scrolling game
 */
UCLASS(abstract)
class ASideScrollingCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Player camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

protected:

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Drop from Platform Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* DropAction;

	/** Interact Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* InteractAction;

	/** Impulse to manually push physics objects while we're in midair */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Jump")
	float JumpPushImpulse = 600.0f;

	/** Multiplies JumpZVelocity for the second jump. */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Jump", meta = (ClampMin = "0.0"))
	float DoubleJumpVerticalMultiplier = 1.0f;

	/** Max distance that interactive objects can be triggered */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Interaction")
	float InteractionRadius = 200.0f;

	/** Distance to check for a basic melee attack */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat")
	float AttackRange = 180.0f;

	/** Radius around the attack point that can hit enemies */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat")
	float AttackRadius = 80.0f;

	/** Damage dealt by the basic melee attack */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat")
	float AttackDamage = 25.0f;

	UPROPERTY(EditAnywhere, Category="Side Scrolling|Debug")
	bool bShowGameplayDebugMessages = false;

	UPROPERTY(EditAnywhere, Category="Side Scrolling|Debug")
	bool bDrawWeaponTraceDebug = false;

	UPROPERTY(EditAnywhere, Category="Side Scrolling|Debug", meta = (ClampMin = "0.0"))
	float WeaponTraceDebugDuration = 0.15f;

	UPROPERTY(EditAnywhere, Category="Side Scrolling|Debug", meta = (ClampMin = "0.0"))
	float WeaponTraceDebugLineThickness = 2.0f;

	UPROPERTY(EditAnywhere, Category="Side Scrolling|Debug")
	FColor WeaponTraceDebugColor = FColor::Cyan;

	UPROPERTY(EditAnywhere, Category="Side Scrolling|Debug")
	FColor WeaponTraceDebugHitColor = FColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Visuals")
	TObjectPtr<USkeletalMesh> PlayerSkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Visuals")
	TSubclassOf<UAnimInstance> PlayerAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Visuals")
	FVector PlayerMeshRelativeLocation = FVector(0.0f, 0.0f, -90.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Visuals")
	FRotator PlayerMeshRelativeRotation = FRotator(0.0f, -90.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Visuals")
	FVector PlayerMeshScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation")
	bool bPlayAttackAnimation = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Ground Combo")
	TObjectPtr<UAnimMontage> GroundAttack1Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Ground Combo")
	TObjectPtr<UAnimMontage> GroundAttack2Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Ground Combo")
	TObjectPtr<UAnimMontage> GroundAttack3Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Ground Combo")
	TObjectPtr<UAnimMontage> GroundAttack4Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Ground Combo", meta = (ClampMin = "0.1"))
	float GroundAttackMontagePlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Air To Floor")
	TObjectPtr<UAnimMontage> AirToFloorAttackMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Roll")
	TObjectPtr<UAnimSequenceBase> RollAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Roll")
	TObjectPtr<UAnimMontage> RollMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Roll")
	FName RollSlotName = TEXT("GroundAttackSlot");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Roll", meta = (ClampMin = "0.1"))
	float RollPlayRate = 1.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Roll", meta = (ClampMin = "0.0"))
	float RollBlendInTime = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Roll", meta = (ClampMin = "0.0"))
	float RollBlendOutTime = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Roll")
	bool bRollInterruptsGroundCombo = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation")
	TObjectPtr<UAnimSequenceBase> CombatToIdleAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation", meta = (ClampMin = "0.1"))
	float AttackAnimationPlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation", meta = (ClampMin = "0.1"))
	float CombatTransitionAnimationPlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation")
	bool bPlayCombatTransitionAnimations = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation")
	FName CombatTransitionSlotName = TEXT("GroundAttackSlot");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation", meta = (ClampMin = "0.0"))
	float CombatTransitionBlendInTime = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation", meta = (ClampMin = "0.0"))
	float CombatTransitionBlendOutTime = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation")
	bool bAllowMovementDuringSheathing = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation")
	bool bRestoreMeshTransformAfterAttackAnimation = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation")
	bool bMovementInterruptsAttackAnimation = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation")
	bool bBlockRepeatedAttacksUntilAnimationEnds = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AttackHitTimeRatio = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AttackHitWindowStartRatio = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AttackHitWindowEndRatio = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Visuals")
	bool bLockFacingToSideScrollingAxis = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Visuals")
	float FacingYawRight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Visuals")
	float FacingYawLeft = 180.0f;

	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat", meta = (ClampMin = "0.0"))
	float AttackKnockbackDistance = 30.0f;

	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat")
	bool bUseWeaponTraceForAttack = true;

	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat", meta = (ClampMin = "1.0"))
	float WeaponTraceRadius = 24.0f;

	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat")
	FName WeaponTraceStartComponentName = TEXT("KatanaTraceStart");

	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat")
	FName WeaponTraceEndComponentName = TEXT("KatanaTraceEnd");

	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat")
	FName SkillWeaponTraceStartComponentName = TEXT("SkillKatanaTraceStart");

	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat")
	FName SkillWeaponTraceEndComponentName = TEXT("SkillKatanaTraceEnd");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Side Scrolling|Health")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Health", meta = (ClampMin = "1.0"))
	float HealthTestMultiplier = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Health", meta = (ClampMin = "0.0"))
	float HitInvulnerabilityDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Health")
	bool bIgnoreDamageWhileRolling = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Hit")
	TObjectPtr<UAnimSequenceBase> HitReactionAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Death")
	TObjectPtr<UAnimSequenceBase> DeathAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Death")
	TObjectPtr<UAnimMontage> DeathMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Death")
	FName DeathSlotName = TEXT("GroundAttackSlot");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Death", meta = (ClampMin = "0.1"))
	float DeathPlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Death", meta = (ClampMin = "0.0"))
	float DeathBlendInTime = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Death", meta = (ClampMin = "0.0"))
	float DeathBlendOutTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Death", meta = (ClampMin = "0.0"))
	float DeathResultDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Hit")
	FName HitReactionSlotName = TEXT("GroundAttackSlot");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Hit", meta = (ClampMin = "0.1"))
	float HitReactionPlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Hit", meta = (ClampMin = "0.0"))
	float HitReactionBlendInTime = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Hit", meta = (ClampMin = "0.0"))
	float HitReactionBlendOutTime = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Hit", meta = (ClampMin = "0.0"))
	float HitReactionLockDuration = 0.28f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Hit", meta = (ClampMin = "0.0"))
	float HitReactionKnockbackImpulse = 380.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Skill", meta = (ClampMin = "0.0"))
	float SkillReleaseDuration = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Skill")
	bool bSkillGrantsInvulnerability = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Skill", meta = (ClampMin = "1.0"))
	float MaxMana = 200.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Side Scrolling|Skill")
	float CurrentMana = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Skill")
	bool bAutoRegenerateMana = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Skill", meta = (ClampMin = "0.0"))
	float ManaRegenPerSecond = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Skill", meta = (ClampMin = "0.0"))
	float Skill1ManaCost = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Skill", meta = (ClampMin = "0.0"))
	float Skill2ManaCost = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Skill", meta = (ClampMin = "0.0"))
	float Skill3ManaCost = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Skill", meta = (ClampMin = "0.0"))
	float Skill1CooldownDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Skill", meta = (ClampMin = "0.0"))
	float Skill2CooldownDuration = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Skill", meta = (ClampMin = "0.0"))
	float Skill3CooldownDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Skill")
	TObjectPtr<UAnimSequenceBase> NumberSkill1Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Skill")
	TObjectPtr<UAnimMontage> NumberSkill1Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Skill")
	TObjectPtr<UAnimSequenceBase> NumberSkill2Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Skill")
	TObjectPtr<UAnimMontage> NumberSkill2Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Skill")
	TObjectPtr<UAnimSequenceBase> NumberSkill3Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Skill")
	TObjectPtr<UAnimMontage> NumberSkill3Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Skill")
	FName SkillSlotName = TEXT("GroundAttackSlot");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Skill", meta = (ClampMin = "0.1"))
	float SkillPlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Skill", meta = (ClampMin = "0.0"))
	float SkillBlendInTime = 0.04f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Animation|Skill", meta = (ClampMin = "0.0"))
	float SkillBlendOutTime = 0.12f;

	/** Extra damage gained for each weapon level after level 1 */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat", meta = (ClampMin = "0.0"))
	float DamageGainPerWeaponLevel = 10.0f;

	/** Extra attack range gained for each weapon level after level 1 */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat", meta = (ClampMin = "0.0"))
	float RangeGainPerWeaponLevel = 35.0f;

	/** Extra attack radius gained for each weapon level after level 1 */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat", meta = (ClampMin = "0.0"))
	float RadiusGainPerWeaponLevel = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Audio")
	TObjectPtr<USoundBase> AttackHitSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Audio", meta = (ClampMin = "0.0"))
	float AttackHitSoundVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Side Scrolling|Audio", meta = (ClampMin = "0.01"))
	float AttackHitSoundPitch = 1.0f;

	/** Ink needed to reach the next weapon level */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Progression", meta = (ClampMin = "1"))
	int32 InkNeededPerWeaponLevel = 3;

	/** Highest weapon level allowed in this prototype */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Progression", meta = (ClampMin = "1"))
	int32 MaxWeaponLevel = 5;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Side Scrolling|Progression")
	int32 CurrentInk = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Side Scrolling|Progression")
	int32 WeaponLevel = 1;

	/** Time to disable input after a wall jump to preserve momentum */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Wall Jump")
	float DelayBetweenWallJumps = 0.3f;

	/** Distance to trace ahead of the character for wall jumps */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Wall Jump")
	float WallJumpTraceDistance = 50.0f;

	/** Horizontal impulse to apply to the character during wall jumps */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Wall Jump")
	float WallJumpHorizontalImpulse = 500.0f;

	/** Multiplies the jump Z velocity for wall jumps. */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Wall Jump")
	float WallJumpVerticalMultiplier = 1.4f;

	/** Collision object type to use for soft collision traces (dropping down floors) */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Soft Platforms")
	TEnumAsByte<ECollisionChannel> SoftCollisionObjectType;

	/** Distance to trace down during soft collision checks */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Soft Platforms")
	float SoftCollisionTraceDistance = 1000.0f;

	/** Last recorded time when this character started falling */
	float LastFallTime = 0.0f;

	/** Max amount of time that can pass since we started falling when we allow a regular jump */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Coyote Time", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float MaxCoyoteTime = 0.16f;

	/** Wall jump lockout timer */
	FTimerHandle WallJumpTimer;

	FTimerHandle AttackAnimationTimer;
	FTimerHandle AttackHitTimer;
	FTimerHandle AttackHitWindowStartTimer;
	FTimerHandle AttackHitWindowEndTimer;
	FTimerHandle HitInvulnerabilityTimer;
	FTimerHandle HitReactionTimer;
	FTimerHandle DeathAnimationTimer;
	FTimerHandle SkillReleaseTimer;

	struct FActiveAttackHitWindow
	{
		FVector PreviousWeaponTraceStart = FVector::ZeroVector;
		FVector PreviousWeaponTraceEnd = FVector::ZeroVector;
		TSet<AModengEnemy*> HitEnemies;
		float FacingSign = 1.0f;
		float DamageMultiplier = 1.0f;
		float KnockbackDistance = 30.0f;
		float WeaponTraceRadius = 24.0f;
		float MinimumWeaponMotionSpeed = 180.0f;
		bool bUseAutomaticWeaponMotionHitWindow = false;
		bool bForceCurrentSegmentHit = false;
		bool bHasPreviousWeaponTrace = false;
		bool bRegisteredHit = false;
	};

	struct FDeferredAttackKnockback
	{
		TWeakObjectPtr<AModengEnemy> Enemy;
		float FacingSign = 1.0f;
		float Distance = 0.0f;
	};

	FTransform MeshTransformBeforeAttackAnimation;
	TArray<FActiveAttackHitWindow> ActiveAttackHitWindows;
	TArray<FDeferredAttackKnockback> DeferredAttackKnockbacks;
	float PendingAttackFacingSign = 1.0f;
	float CurrentAttackDamageMultiplier = 1.0f;
	float CurrentAttackKnockbackDistance = 30.0f;
	float CurrentWeaponTraceRadius = 24.0f;
	float CurrentMinimumWeaponMotionSpeed = 180.0f;
	int32 CurrentGroundComboStep = 0;
	int32 ActiveSkillIndex = 0;
	float Skill1CooldownRemaining = 0.0f;
	float Skill2CooldownRemaining = 0.0f;
	float Skill3CooldownRemaining = 0.0f;
	ESideScrollingCombatAnimationPhase CurrentCombatAnimationPhase = ESideScrollingCombatAnimationPhase::None;
	bool bCurrentUseAutomaticWeaponMotionHitWindow = false;
	bool bAttackHitPending = false;
	bool bAttackHitWindowActive = false;
	bool bAttackRegisteredHit = false;
	bool bDeferAttackKnockback = false;
	bool bComboInputQueued = false;
	bool bUseSkillWeaponTrace = false;
	bool bGroundAttackMontageInProgress = false;
	bool bGroundComboInputWindowOpen = false;
	bool bGroundMoveCancelWindowOpen = false;
	bool bAirToFloorAttackInProgress = false;
	bool bRollInProgress = false;
	bool bRollCancelWindowOpen = false;
	bool bRollInvincible = false;
	bool bRollAttackQueued = false;
	bool bRollJumpQueued = false;
	bool bRollPausedFalling = false;
	float RollMoveQueuedValue = 0.0f;
	float SavedRollGravityScale = 1.75f;
	UAnimMontage* ActiveGroundAttackMontage = nullptr;
	UAnimMontage* ActiveAirToFloorAttackMontage = nullptr;
	UAnimMontage* ActiveCombatTransitionMontage = nullptr;
	UAnimMontage* ActiveHitReactionMontage = nullptr;
	UAnimMontage* ActiveDeathMontage = nullptr;
	UAnimMontage* ActiveRollMontage = nullptr;
	UAnimMontage* ActiveSkillMontage = nullptr;

	/** Last captured horizontal movement input value */
	float ActionValueY = 0.0f;

	float LastFacingX = 1.0f;

	/** Last captured platform drop axis value */
	float DropValue = 0.0f;

	/** If true, this character has already wall jumped */
	bool bHasWallJumped = false;

	/** If true, this character has already double jumped */
	bool bHasDoubleJumped = false;

	/** If true, this character is moving along the side scrolling axis */
	bool bMovingHorizontally = false;

	bool bAttackAnimationInProgress = false;
	bool bHitReactionInProgress = false;
	bool bSkillReleaseInProgress = false;
	bool bDamageInvulnerable = false;
	bool bPlayerDefeated = false;
	bool bDeathAnimationFinished = false;

public:
	
	/** Constructor */
	ASideScrollingCharacter();

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Frame update */
	virtual void Tick(float DeltaSeconds) override;

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Collision handling */
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	/** Landing handling */
	virtual void Landed(const FHitResult& Hit) override;

	/** Handle movement mode changes to keep track of coyote time jumps */
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for drop from platform input */
	void Drop(const FInputActionValue& Value);

	/** Called for drop from platform input release */
	void DropReleased(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Forward);

	/** Handles drop inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoDrop(float Value);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Handles interact inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoInteract();

	/** Handles skill inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSkill1();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSkill2();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSkill3();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSkillByIndex(int32 SkillIndex);

	UFUNCTION(BlueprintPure, Category="Input")
	bool IsSkillReleaseInProgress() const;

	UFUNCTION(BlueprintPure, Category="Input")
	int32 GetActiveSkillIndex() const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Skill")
	float GetManaPercent() const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Skill")
	float GetCurrentMana() const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Skill")
	float GetMaxMana() const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Skill")
	float GetSkillManaCost(int32 SkillIndex) const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Skill")
	float GetSkillCooldownDuration(int32 SkillIndex) const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Skill")
	float GetSkillCooldownRemaining(int32 SkillIndex) const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Skill")
	float GetSkillCooldownPercent(int32 SkillIndex) const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Skill")
	bool CanUseSkillByIndex(int32 SkillIndex) const;

	UFUNCTION(BlueprintImplementableEvent, Category="Input")
	void OnSkillInputPressedByIndex(int32 SkillIndex);

	UFUNCTION(BlueprintImplementableEvent, Category="Input")
	void OnSkillReleaseFinishedByIndex(int32 SkillIndex);

	/** Handles basic attack inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAttack();

	/** Handles roll inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoRoll();

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Progression")
	void AddInk(int32 Amount);

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Progression")
	int32 GetWeaponLevel() const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Progression")
	int32 GetCurrentInk() const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Progression")
	float GetInkProgressPercent() const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Combat")
	float GetCurrentAttackDamage() const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Combat")
	float GetCurrentAttackRange() const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Combat")
	float GetCurrentAttackRadius() const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Combat")
	float GetSideScrollingFacingSign() const;

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Audio")
	void PlayAttackHitSoundAtLocation(FVector HitLocation);

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Health")
	bool ApplyDamageToPlayer(float DamageAmount, AActor* DamageSource = nullptr);

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Health")
	float RestoreHealth(float RestoreAmount);

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Health")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Skill")
	float RestoreMana(float RestoreAmount);

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Health")
	bool IsPlayerDefeated() const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Health")
	bool IsPlayerDeathAnimationFinished() const;

	UPROPERTY(BlueprintAssignable, Category="Side Scrolling|Health")
	FSideScrollingPlayerDeathFinishedSignature OnDeathAnimationFinished;

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Visuals")
	bool SetSceneComponentVisibleByName(FName ComponentName, bool bVisible, bool bPropagateToChildren = true);

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Visuals")
	void SetCombatWeaponDrawnForNotify(bool bDrawn);

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Visuals")
	void SetAnimationWeaponModeForNotify(bool bUseSkeletalWeapon, bool bUseSheathedWeapon, FName SkeletalWeaponComponentName, FName SocketWeaponComponentName);

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Visuals")
	void SetSkillWeaponModeForNotify(FName WeaponModeName, FName NormalHandComponentName = TEXT("Sword_Hand"), FName SkillHandComponentName = TEXT("Sword_SkillHand"), FName BoneComponentName = TEXT("Sword_Bone"));

protected:

	/** Handles advanced jump logic */
	void MultiJump();

	/** Checks for soft collision with platforms */
	void CheckForSoftCollision();

	/** Resets wall jump lockout. Called from timer after a wall jump */
	void ResetWallJump();

	void TryUpgradeWeapon();
	void ConfigurePlayerVisuals();
	void StartGroundAttackMontage();
	bool PlayGroundAttackMontageStep(int32 ComboStepIndex);
	UAnimMontage* GetGroundAttackMontage(int32 ComboStepIndex) const;
	void FinishGroundAttackAndStartSheathe(bool bStopActiveMontage);
	void FinishGroundAttackMontageState(bool bInterrupted);
	void OnGroundAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void StartAirToFloorAttack();
	void BeginAirToFloorLoop();
	void FinishAirToFloorImpact();
	float PlayAirToFloorAnimation(UAnimSequenceBase* AnimationToPlay, bool bLooping, ESideScrollingCombatAnimationPhase NewAnimationPhase);
	void OnAirToFloorAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void StopActiveAirToFloorAttackMontage(float BlendOutTime);
	bool PlayCombatTransitionAnimation(UAnimSequenceBase* AnimationToPlay, ESideScrollingCombatAnimationPhase NewAnimationPhase);
	void StartSheatheOrRestoreAnimation();
	void FinishCombatTransitionState();
	void OnCombatTransitionMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void SetCombatWeaponDrawn(bool bDrawn);
	bool PlayHitReaction(AActor* DamageSource);
	void FinishHitReaction();
	void OnHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	bool PlayDeathAnimation(AActor* DamageSource);
	void FinishDeathAnimation();
	void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void ClearDamageInvulnerability();
	bool CanStartSkill() const;
	bool HasEnoughManaForSkill(int32 SkillIndex) const;
	bool IsSkillCooldownReady(int32 SkillIndex) const;
	void UpdateSkillResources(float DeltaSeconds);
	void SpendManaForSkill(int32 SkillIndex);
	void StartSkillCooldown(int32 SkillIndex);
	void StartSkillRelease(int32 SkillIndex);
	void FinishSkillRelease();
	float PlaySkillReleaseAnimation(int32 SkillIndex);
	UAnimSequenceBase* GetSkillAnimationForIndex(int32 SkillIndex) const;
	UAnimMontage* GetSkillMontageForIndex(int32 SkillIndex) const;
	void StopActiveSkillMontage(float BlendOutTime);
	void OnSkillMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void StopActiveRollMontage(float BlendOutTime);
	void TryConsumeRollQueuedInput();
	void PauseFallingForAirRoll();
	void RestoreRollFallingMovement();
	void ApplyPendingAttackHit();
	void BeginAttackHitWindow();
	void EndAttackHitWindow();
	void ClearAttackHitWindows(bool bReportMisses = false);
	void RefreshAttackHitWindowState();
	void UpdateAttackHitWindow();
	void FlushDeferredAttackKnockbacks();
	void RestorePlayerAnimationBlueprint();
	void FinishAttackAnimation();
	void InterruptAttackAnimation();
	void ResetAttackCombo();
	void UpdateFacingDirection(float FacingSign);
	void RefreshSwordInScabbardVisibility();
	USceneComponent* FindSceneComponentByName(FName ComponentName) const;
	bool ResolveWeaponTraceComponents(USceneComponent*& OutTraceStartComponent, USceneComponent*& OutTraceEndComponent) const;
	bool ApplyWeaponTraceAttackHit(FActiveAttackHitWindow& AttackWindow, bool& bOutTraceAttempted);
	bool ApplyFallbackBoxAttackHit(FActiveAttackHitWindow& AttackWindow);
	void DamageEnemyFromAttack(AModengEnemy* Enemy, const FActiveAttackHitWindow& AttackWindow);

public:

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Combat")
	void OpenGroundComboInputWindow();

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Combat")
	void CloseGroundComboInputWindow();

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Combat")
	void CommitGroundCombo();

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Combat")
	void OpenGroundMoveCancelWindow();

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Combat")
	void LoopGroundCombo();

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Combat")
	void BeginGroundAttackTrace(int32 ComboStepIndex);

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Combat")
	void EndGroundAttackTrace();

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Combat")
	void FinishGroundAttackMontage();

	void FinishGroundAttackMontageFromMontage(UAnimMontage* SourceMontage);

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Combat")
	void BeginRollInvincible();

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Combat")
	void EndRollInvincible();

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Combat")
	void OpenRollCancelWindow();

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Combat")
	void FinishRoll();

	/** Sets the soft collision response. True passes, False blocks */
	void SetSoftCollision(bool bEnabled);

public:

	/** Returns true if the character has just double jumped */
	UFUNCTION(BlueprintPure, Category="Side Scrolling")
	bool HasDoubleJumped() const;

	/** Returns true if the character has just wall jumped */
	UFUNCTION(BlueprintPure, Category="Side Scrolling")
	bool HasWallJumped() const;
};
