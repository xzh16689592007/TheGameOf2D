// Copyright Epic Games, Inc. All Rights Reserved.


#include "SideScrollingCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequenceBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "Engine/HitResult.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "Components/SceneComponent.h"
#include "ModengEnemy.h"
#include "ModengLantern.h"
#include "SideScrollingInteractable.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ASideScrollingCharacter::ASideScrollingCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// create the camera component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);

	Camera->SetRelativeLocationAndRotation(FVector(0.0f, 300.0f, 0.0f), FRotator(0.0f, -90.0f, 0.0f));

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetGenerateOverlapEvents(false);
	GetMesh()->SetCastShadow(true);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyMesh(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	if (MannyMesh.Succeeded())
	{
		PlayerSkeletalMesh = MannyMesh.Object;
		GetMesh()->SetSkeletalMesh(PlayerSkeletalMesh);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> SideScrollerAnimClass(TEXT("/Game/Variant_SideScrolling/Anims/ABP_Manny_SideScroller"));
	if (SideScrollerAnimClass.Succeeded())
	{
		PlayerAnimClass = SideScrollerAnimClass.Class;
		GetMesh()->SetAnimInstanceClass(PlayerAnimClass);
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> GroundAttack1Asset(TEXT("/Game/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/AM_Tomoe_GroundAttack_1.AM_Tomoe_GroundAttack_1"));
	if (GroundAttack1Asset.Succeeded())
	{
		GroundAttack1Montage = GroundAttack1Asset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> GroundAttack2Asset(TEXT("/Game/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/AM_Tomoe_GroundAttack_2.AM_Tomoe_GroundAttack_2"));
	if (GroundAttack2Asset.Succeeded())
	{
		GroundAttack2Montage = GroundAttack2Asset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> GroundAttack3Asset(TEXT("/Game/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/AM_Tomoe_GroundAttack_3.AM_Tomoe_GroundAttack_3"));
	if (GroundAttack3Asset.Succeeded())
	{
		GroundAttack3Montage = GroundAttack3Asset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> GroundAttack4Asset(TEXT("/Game/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AttackInGround/AM_Tomoe_GroundAttack_4.AM_Tomoe_GroundAttack_4"));
	if (GroundAttack4Asset.Succeeded())
	{
		GroundAttack4Montage = GroundAttack4Asset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> AirToFloorAttackAsset(TEXT("/Game/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AirToFloor_01/AM_Tomoe_AirToFloorAttack.AM_Tomoe_AirToFloorAttack"));
	if (AirToFloorAttackAsset.Succeeded())
	{
		AirToFloorAttackMontage = AirToFloorAttackAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> RollAnimationAsset(TEXT("/Game/MoDeng/Animations/Tomoe/Roll/Roll_F_0_Seq_Short.Roll_F_0_Seq_Short"));
	if (RollAnimationAsset.Succeeded())
	{
		RollAnimation = RollAnimationAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> RollMontageAsset(TEXT("/Game/MoDeng/Animations/Tomoe/Roll/AM_Tomoe_Roll.AM_Tomoe_Roll"));
	if (RollMontageAsset.Succeeded())
	{
		RollMontage = RollMontageAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> SkillAnimationAsset(TEXT("/Game/MoDeng/Animations/Tomoe/Attack/Skill/AS_Combo_Attack_All_Seq.AS_Combo_Attack_All_Seq"));
	if (SkillAnimationAsset.Succeeded())
	{
		SkillAnimation = SkillAnimationAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> SkillMontageAsset(TEXT("/Game/MoDeng/Animations/Tomoe/Attack/Skill/AS_Combo_Attack_All_Seq_Montage.AS_Combo_Attack_All_Seq_Montage"));
	if (SkillMontageAsset.Succeeded())
	{
		SkillMontage = SkillMontageAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> HitReactionAsset(TEXT("/Game/MoDeng/Animations/Tomoe/Hit/Hit_F_Seq.Hit_F_Seq"));
	if (HitReactionAsset.Succeeded())
	{
		HitReactionAnimation = HitReactionAsset.Object;
	}

	// configure the collision capsule
	GetCapsuleComponent()->SetCapsuleSize(35.0f, 90.0f);

	// configure the Pawn properties
	bUseControllerRotationYaw = false;

	// configure the character movement component
	GetCharacterMovement()->GravityScale = 1.75f;
	GetCharacterMovement()->MaxAcceleration = 1500.0f;
	GetCharacterMovement()->BrakingFrictionFactor = 1.0f;
	GetCharacterMovement()->bUseSeparateBrakingFriction = true;
	GetCharacterMovement()->Mass = 500.0f;

	GetCharacterMovement()->SetWalkableFloorAngle(75.0f);
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
	GetCharacterMovement()->bIgnoreBaseRotation = true;

	GetCharacterMovement()->PerchRadiusThreshold = 15.0f;
	GetCharacterMovement()->LedgeCheckThreshold = 6.0f;

	GetCharacterMovement()->JumpZVelocity = 750.0f;
	GetCharacterMovement()->AirControl = 1.0f;

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 750.0f, 0.0f);
	GetCharacterMovement()->bOrientRotationToMovement = false;

	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, 1.0f, 0.0f));
	GetCharacterMovement()->bConstrainToPlane = true;

	// Ground jump plus one air jump. Wall jump is handled separately.
	JumpMaxCount = 2;
}

void ASideScrollingCharacter::BeginPlay()
{
	Super::BeginPlay();

	const float HealthMultiplier = FMath::Max(1.0f, HealthTestMultiplier);
	MaxHealth *= HealthMultiplier;
	CurrentHealth = MaxHealth;

	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseControllerDesiredRotation = false;
	}

	ConfigurePlayerVisuals();
	UpdateFacingDirection(LastFacingX);

	if (USceneComponent* HandSword = FindSceneComponentByName(TEXT("Sword_Hand")))
	{
		HandSword->SetVisibility(false, true);
		HandSword->SetHiddenInGame(true, true);
	}

	if (USceneComponent* SkillHandSword = FindSceneComponentByName(TEXT("Sword_SkillHand")))
	{
		SkillHandSword->SetVisibility(false, true);
		SkillHandSword->SetHiddenInGame(true, true);
	}

	if (USceneComponent* BoneSword = FindSceneComponentByName(TEXT("Sword_Bone")))
	{
		BoneSword->SetVisibility(false, true);
		BoneSword->SetHiddenInGame(true, true);
	}

	if (USceneComponent* SheathedSword = FindSceneComponentByName(TEXT("Sword_InScabbard")))
	{
		SheathedSword->SetVisibility(true, true);
		SheathedSword->SetHiddenInGame(false, true);
	}

	if (USceneComponent* Scabbard = FindSceneComponentByName(TEXT("Sword_Sheathed")))
	{
		Scabbard->SetVisibility(true, true);
		Scabbard->SetHiddenInGame(false, true);
	}
}

void ASideScrollingCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bAttackHitWindowActive)
	{
		UpdateAttackHitWindow();
	}

}

void ASideScrollingCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the wall jump timer
	GetWorld()->GetTimerManager().ClearTimer(WallJumpTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackAnimationTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackHitTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackHitWindowStartTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackHitWindowEndTimer);
	GetWorld()->GetTimerManager().ClearTimer(HitInvulnerabilityTimer);
	GetWorld()->GetTimerManager().ClearTimer(HitReactionTimer);
	GetWorld()->GetTimerManager().ClearTimer(SkillReleaseTimer);
}

void ASideScrollingCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASideScrollingCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ASideScrollingCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASideScrollingCharacter::Move);

		// Dropping from platform
		EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Triggered, this, &ASideScrollingCharacter::Drop);
		EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Completed, this, &ASideScrollingCharacter::DropReleased);

	}

	PlayerInputComponent->BindKey(EKeys::J, IE_Pressed, this, &ASideScrollingCharacter::DoAttack);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ASideScrollingCharacter::DoAttack);
	PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ASideScrollingCharacter::DoRoll);
}

void ASideScrollingCharacter::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// only apply push impulse if we're falling
	if (!GetCharacterMovement()->IsFalling())
	{
		return;
	}

	// ensure the colliding component is valid
	if (OtherComp)
	{
		// ensure the component is movable and simulating physics
		if (OtherComp->Mobility == EComponentMobility::Movable && OtherComp->IsSimulatingPhysics())
		{
			const FVector PushDir = FVector(ActionValueY > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);

			// push the component away
			OtherComp->AddImpulse(PushDir * JumpPushImpulse, NAME_None, true);
		}
	}
}

void ASideScrollingCharacter::Landed(const FHitResult& Hit)
{
	// reset the double jump
	bHasDoubleJumped = false;
	RestoreRollFallingMovement();

	if (bAirToFloorAttackInProgress)
	{
		FinishAirToFloorImpact();
	}
}

void ASideScrollingCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode /*= 0*/)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	// are we falling?
	if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Falling)
	{
		// save the game time when we started falling, so we can check it later for coyote time jumps
		LastFallTime = GetWorld()->GetTimeSeconds();
	}
}

void ASideScrollingCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MoveVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MoveVector.Y);
}

void ASideScrollingCharacter::Drop(const FInputActionValue& Value)
{
	// route the input
	DoDrop(Value.Get<float>());
}

void ASideScrollingCharacter::DropReleased(const FInputActionValue& Value)
{
	// reset the input
	DoDrop(0.0f);
}

void ASideScrollingCharacter::DoMove(float Forward)
{
	if (bHitReactionInProgress || bSkillReleaseInProgress || bPlayerDefeated)
	{
		ActionValueY = 0.0f;
		return;
	}

	if (bRollInProgress)
	{
		RollMoveQueuedValue = Forward;
		if (bRollCancelWindowOpen && !FMath::IsNearlyZero(Forward))
		{
			const float QueuedMove = Forward;
			FinishRoll();
			DoMove(QueuedMove);
		}
		return;
	}

	// is movement temporarily disabled after wall jumping?
	if (!bHasWallJumped)
	{
		if (bAttackAnimationInProgress)
		{
			if (bGroundMoveCancelWindowOpen && !FMath::IsNearlyZero(Forward) && (bGroundAttackMontageInProgress || CurrentCombatAnimationPhase == ESideScrollingCombatAnimationPhase::AirToFloorEnd))
			{
				if (bGroundAttackMontageInProgress)
				{
					FinishGroundAttackAndStartSheathe(true);
				}
				else
				{
					StopActiveAirToFloorAttackMontage(0.04f);
					StartSheatheOrRestoreAnimation();
				}
			}
			else
			{
				const bool bUsesUpperBodySheatheSlot = CombatTransitionSlotName == FName(TEXT("UpperBodyCombatSlot"));
				const bool bCanMoveDuringCurrentAttack = CurrentCombatAnimationPhase == ESideScrollingCombatAnimationPhase::Sheathing && (bAllowMovementDuringSheathing || bUsesUpperBodySheatheSlot);
				if (!bCanMoveDuringCurrentAttack)
				{
					ActionValueY = 0.0f;
					if (!bAirToFloorAttackInProgress)
					{
						if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
						{
							MovementComponent->StopMovementImmediately();
						}
					}
					return;
				}
			}
		}

		// save the movement values
		ActionValueY = Forward;
		if (!FMath::IsNearlyZero(Forward))
		{
			LastFacingX = Forward > 0.0f ? 1.0f : -1.0f;
			UpdateFacingDirection(LastFacingX);
		}

		// figure out the movement direction
		const FVector MoveDir = FVector(1.0f, Forward > 0.0f ? 0.1f : -0.1f, 0.0f);

		// apply the movement input
		AddMovementInput(MoveDir, Forward);
	}
}

void ASideScrollingCharacter::DoDrop(float Value)
{
	if (bSkillReleaseInProgress)
	{
		DropValue = 0.0f;
		return;
	}

	// save the movement value
	DropValue = Value;
}

void ASideScrollingCharacter::DoJumpStart()
{
	if (bHitReactionInProgress || bSkillReleaseInProgress || bPlayerDefeated)
	{
		return;
	}

	if (bRollInProgress)
	{
		bRollJumpQueued = true;
		if (bRollCancelWindowOpen)
		{
			TryConsumeRollQueuedInput();
		}
		return;
	}

	if (bAttackAnimationInProgress && !bAirToFloorAttackInProgress)
	{
		return;
	}

	// handle advanced jump behaviors
	MultiJump();
}

void ASideScrollingCharacter::DoJumpEnd()
{
	if (bSkillReleaseInProgress)
	{
		return;
	}

	StopJumping();
}

void ASideScrollingCharacter::DoInteract()
{
	if (bHitReactionInProgress || bSkillReleaseInProgress || bPlayerDefeated)
	{
		return;
	}

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, TEXT("Interact pressed"));
	}

	const FVector Start = GetActorLocation();

	FCollisionShape ColSphere;
	ColSphere.SetSphere(InteractionRadius);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	TArray<FOverlapResult> OverlapResults;
	GetWorld()->OverlapMultiByObjectType(OverlapResults, Start, FQuat::Identity, ObjectParams, ColSphere, QueryParams);

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* HitActor = OverlapResult.GetActor();
		if (!HitActor)
		{
			continue;
		}

		if (ISideScrollingInteractable* Interactable = Cast<ISideScrollingInteractable>(HitActor))
		{
			Interactable->Interaction(this);
			return;
		}
	}

	AModengLantern* ClosestRepairableLantern = nullptr;
	float ClosestLanternDistanceSq = TNumericLimits<float>::Max();
	for (TActorIterator<AModengLantern> It(GetWorld()); It; ++It)
	{
		AModengLantern* Lantern = *It;
		if (!Lantern || !Lantern->CanRepairFromLocation(Start, InteractionRadius))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(Start, Lantern->GetActorLocation());
		if (DistanceSq <= ClosestLanternDistanceSq)
		{
			ClosestLanternDistanceSq = DistanceSq;
			ClosestRepairableLantern = Lantern;
		}
	}

	if (ClosestRepairableLantern)
	{
		ClosestRepairableLantern->RepairByDefaultAmount();
		return;
	}

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("No interact target"));
	}
}

void ASideScrollingCharacter::DoSkill()
{
	if (!CanStartSkill())
	{
		return;
	}

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Purple, TEXT("Skill pressed"));
	}

	StartSkillRelease();
}

bool ASideScrollingCharacter::IsSkillReleaseInProgress() const
{
	return bSkillReleaseInProgress;
}

void ASideScrollingCharacter::DoAttack()
{
	if (bHitReactionInProgress || bSkillReleaseInProgress || bPlayerDefeated)
	{
		return;
	}

	if (bRollInProgress)
	{
		const bool bAirRollInProgress = bRollPausedFalling || (GetCharacterMovement() && GetCharacterMovement()->IsFalling());
		if (bAirRollInProgress)
		{
			StopActiveRollMontage(0.04f);
			RestorePlayerAnimationBlueprint();
			DoAttack();
			return;
		}

		bRollAttackQueued = true;
		if (bRollCancelWindowOpen)
		{
			TryConsumeRollQueuedInput();
		}
		return;
	}

	if (CurrentCombatAnimationPhase == ESideScrollingCombatAnimationPhase::Sheathing)
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackAnimationTimer);
		if (bAttackHitWindowActive)
		{
			ClearAttackHitWindows();
		}

		UAnimMontage* MontageToStop = ActiveCombatTransitionMontage;
		ActiveCombatTransitionMontage = nullptr;
		if (MontageToStop)
		{
			if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
			{
				AnimInstance->Montage_Stop(0.03f, MontageToStop);
			}
		}

		ResetAttackCombo();
		bAttackAnimationInProgress = false;
		SetCombatWeaponDrawn(true);

		if (GetCharacterMovement() && GetCharacterMovement()->IsFalling())
		{
			StartAirToFloorAttack();
		}
		else
		{
			StartGroundAttackMontage();
		}
		return;
	}

	if (bAttackAnimationInProgress)
	{
		if (bAirToFloorAttackInProgress)
		{
			return;
		}

		if (bGroundAttackMontageInProgress && bGroundComboInputWindowOpen)
		{
			bComboInputQueued = true;
		}
		return;
	}

	if (GetCharacterMovement() && GetCharacterMovement()->IsFalling())
	{
		StartAirToFloorAttack();
		return;
	}

	StartGroundAttackMontage();
}

void ASideScrollingCharacter::DoRoll()
{
	if (bHitReactionInProgress || bSkillReleaseInProgress || bPlayerDefeated)
	{
		return;
	}

	if (bRollInProgress || bAttackAnimationInProgress || (!RollMontage && !RollAnimation))
	{
		return;
	}

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	UAnimInstance* AnimInstance = CharacterMesh ? CharacterMesh->GetAnimInstance() : nullptr;
	if (!CharacterMesh || !AnimInstance)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	MeshTransformBeforeAttackAnimation = CharacterMesh->GetRelativeTransform();
	bRollInProgress = true;
	bRollCancelWindowOpen = false;
	bRollInvincible = false;
	bRollAttackQueued = false;
	bRollJumpQueued = false;
	RollMoveQueuedValue = 0.0f;
	bAttackAnimationInProgress = true;
	CurrentCombatAnimationPhase = ESideScrollingCombatAnimationPhase::None;
	PauseFallingForAirRoll();

	const float SafePlayRate = FMath::Max(RollPlayRate, 0.1f);
	const bool bRollMontageUsesExpectedSlot = RollMontage && RollMontage->SlotAnimTracks.ContainsByPredicate(
		[this](const FSlotAnimationTrack& SlotTrack)
		{
			return SlotTrack.SlotName == RollSlotName;
		});

	if (bRollMontageUsesExpectedSlot)
	{
		ActiveRollMontage = RollMontage.Get();
		const float Duration = AnimInstance->Montage_Play(ActiveRollMontage, SafePlayRate);
		if (Duration <= 0.0f)
		{
			ActiveRollMontage = nullptr;
		}
	}
	else
	{
		UAnimSequenceBase* RollAnimationToPlay = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/MoDeng/Animations/Tomoe/Roll/Roll_F_0_Seq_Short.Roll_F_0_Seq_Short"));
		if (!RollAnimationToPlay)
		{
			RollAnimationToPlay = RollAnimation.Get();
		}

		ActiveRollMontage = AnimInstance->PlaySlotAnimationAsDynamicMontage(
			RollAnimationToPlay,
			RollSlotName,
			FMath::Max(RollBlendInTime, 0.0f),
			FMath::Max(RollBlendOutTime, 0.0f),
			SafePlayRate);
	}

	if (!ActiveRollMontage)
	{
		FinishRoll();
		return;
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ASideScrollingCharacter::OnRollMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, ActiveRollMontage);
}

void ASideScrollingCharacter::StartAirToFloorAttack()
{
	UAnimSequenceBase* StartAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AirToFloor_01/Attack_Air_To_Floor_Start_01_Seq.Attack_Air_To_Floor_Start_01_Seq"));
	UAnimSequenceBase* LoopAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AirToFloor_01/Attack_Air_To_Floor_Loop_01_Seq.Attack_Air_To_Floor_Loop_01_Seq"));

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	UAnimInstance* AnimInstance = CharacterMesh ? CharacterMesh->GetAnimInstance() : nullptr;

	bAirToFloorAttackInProgress = true;
	SetCombatWeaponDrawn(true);

	PendingAttackFacingSign = LastFacingX >= 0.0f ? 1.0f : -1.0f;
	CurrentAttackDamageMultiplier = 1.35f;
	CurrentAttackKnockbackDistance = 45.0f;
	CurrentWeaponTraceRadius = FMath::Max(WeaponTraceRadius, 30.0f);
	CurrentMinimumWeaponMotionSpeed = 80.0f;
	bCurrentUseAutomaticWeaponMotionHitWindow = true;
	bAttackHitPending = true;
	bAttackHitWindowActive = false;
	bAttackRegisteredHit = false;
	bComboInputQueued = false;
	ActiveAttackHitWindows.Empty();

	GetWorld()->GetTimerManager().ClearTimer(AttackAnimationTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackHitTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackHitWindowStartTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackHitWindowEndTimer);

	if (CharacterMesh && PlayerAnimClass && CharacterMesh->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
	{
		CharacterMesh->GlobalAnimRateScale = 1.0f;
		CharacterMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		CharacterMesh->SetAnimInstanceClass(PlayerAnimClass);
		AnimInstance = CharacterMesh->GetAnimInstance();
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->Velocity.X = 0.0f;
		MovementComponent->Velocity.Y = 0.0f;
		MovementComponent->Velocity.Z = FMath::Min(MovementComponent->Velocity.Z, -900.0f);
	}

	if (bPlayAttackAnimation && AirToFloorAttackMontage && AnimInstance)
	{
		if (!bAttackAnimationInProgress)
		{
			MeshTransformBeforeAttackAnimation = CharacterMesh->GetRelativeTransform();
		}

		ActiveAirToFloorAttackMontage = AirToFloorAttackMontage.Get();
		bAttackAnimationInProgress = true;
		CurrentCombatAnimationPhase = ESideScrollingCombatAnimationPhase::AirToFloorStart;

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ASideScrollingCharacter::OnAirToFloorAttackMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, ActiveAirToFloorAttackMontage);

		const float Duration = AnimInstance->Montage_Play(ActiveAirToFloorAttackMontage, FMath::Max(AttackAnimationPlayRate, 0.1f));
		if (Duration > 0.0f)
		{
			AnimInstance->Montage_JumpToSection(TEXT("Start"), ActiveAirToFloorAttackMontage);
			AnimInstance->Montage_SetNextSection(TEXT("Start"), TEXT("Loop"), ActiveAirToFloorAttackMontage);
			AnimInstance->Montage_SetNextSection(TEXT("Loop"), TEXT("Loop"), ActiveAirToFloorAttackMontage);
			return;
		}

		ActiveAirToFloorAttackMontage = nullptr;
	}

	const float StartDuration = PlayAirToFloorAnimation(StartAnimation ? StartAnimation : LoopAnimation, false, ESideScrollingCombatAnimationPhase::AirToFloorStart);

	if (StartDuration <= 0.0f)
	{
		BeginAirToFloorLoop();
	}
}

void ASideScrollingCharacter::BeginAirToFloorLoop()
{
	if (!bAirToFloorAttackInProgress || !GetCharacterMovement() || !GetCharacterMovement()->IsFalling())
	{
		return;
	}

	if (ActiveAirToFloorAttackMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			AnimInstance->Montage_JumpToSection(TEXT("Loop"), ActiveAirToFloorAttackMontage);
			AnimInstance->Montage_SetNextSection(TEXT("Loop"), TEXT("Loop"), ActiveAirToFloorAttackMontage);
			CurrentCombatAnimationPhase = ESideScrollingCombatAnimationPhase::AirToFloorLoop;
			return;
		}
	}

	UAnimSequenceBase* LoopAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AirToFloor_01/Attack_Air_To_Floor_Loop_01_Seq.Attack_Air_To_Floor_Loop_01_Seq"));
	PlayAirToFloorAnimation(LoopAnimation, true, ESideScrollingCombatAnimationPhase::AirToFloorLoop);
}

void ASideScrollingCharacter::FinishAirToFloorImpact()
{
	if (!bAirToFloorAttackInProgress)
	{
		return;
	}

	UAnimSequenceBase* EndAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/MoDeng/Animations/Tomoe/Attack/Sword_Animations/AirToFloor_01/Attack_Air_To_Floor_End_01_Seq.Attack_Air_To_Floor_End_01_Seq"));

	if (bAttackHitWindowActive)
	{
		UpdateAttackHitWindow();
		ClearAttackHitWindows();
	}

	if (ActiveAirToFloorAttackMontage)
	{
		bAirToFloorAttackInProgress = false;
		CurrentCombatAnimationPhase = ESideScrollingCombatAnimationPhase::AirToFloorEnd;
		if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			AnimInstance->Montage_SetNextSection(TEXT("Loop"), TEXT("End"), ActiveAirToFloorAttackMontage);
			AnimInstance->Montage_JumpToSection(TEXT("End"), ActiveAirToFloorAttackMontage);

			const int32 EndSectionIndex = ActiveAirToFloorAttackMontage->GetSectionIndex(TEXT("End"));
			const float EndSectionLength = EndSectionIndex != INDEX_NONE ? ActiveAirToFloorAttackMontage->GetSectionLength(EndSectionIndex) : 0.0f;
			const float SafePlayRate = FMath::Max(AttackAnimationPlayRate, 0.1f);
			if (EndSectionLength > 0.0f)
			{
				GetWorld()->GetTimerManager().SetTimer(AttackAnimationTimer, this, &ASideScrollingCharacter::FinishAttackAnimation, EndSectionLength / SafePlayRate, false);
			}
		}
		else
		{
			StartSheatheOrRestoreAnimation();
		}
		return;
	}

	bAirToFloorAttackInProgress = false;
	const float EndDuration = PlayAirToFloorAnimation(EndAnimation, false, ESideScrollingCombatAnimationPhase::AirToFloorEnd);
	if (EndDuration <= 0.0f)
	{
		StartSheatheOrRestoreAnimation();
	}
}

void ASideScrollingCharacter::OnAirToFloorAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ActiveAirToFloorAttackMontage)
	{
		return;
	}

	ActiveAirToFloorAttackMontage = nullptr;
	if (bInterrupted)
	{
		RestorePlayerAnimationBlueprint();
		ResetAttackCombo();
		SetCombatWeaponDrawn(false);
		return;
	}

	if (CurrentCombatAnimationPhase == ESideScrollingCombatAnimationPhase::AirToFloorEnd || !bAirToFloorAttackInProgress)
	{
		StartSheatheOrRestoreAnimation();
	}
}

void ASideScrollingCharacter::StopActiveAirToFloorAttackMontage(float BlendOutTime)
{
	UAnimMontage* MontageToStop = ActiveAirToFloorAttackMontage;
	ActiveAirToFloorAttackMontage = nullptr;
	if (!MontageToStop)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->Montage_Stop(FMath::Max(BlendOutTime, 0.0f), MontageToStop);
	}
}

void ASideScrollingCharacter::StartGroundAttackMontage()
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	UAnimInstance* AnimInstance = CharacterMesh ? CharacterMesh->GetAnimInstance() : nullptr;
	if (!bPlayAttackAnimation || !GetGroundAttackMontage(1) || !AnimInstance)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	MeshTransformBeforeAttackAnimation = CharacterMesh->GetRelativeTransform();
	bAttackAnimationInProgress = true;
	bGroundAttackMontageInProgress = true;
	bGroundComboInputWindowOpen = false;
	bGroundMoveCancelWindowOpen = false;
	bComboInputQueued = false;
	bAirToFloorAttackInProgress = false;
	CurrentGroundComboStep = 1;
	CurrentCombatAnimationPhase = ESideScrollingCombatAnimationPhase::Attacking;
	SetCombatWeaponDrawn(true);

	if (!PlayGroundAttackMontageStep(1))
	{
		FinishGroundAttackMontageState(true);
	}
}

bool ASideScrollingCharacter::PlayGroundAttackMontageStep(int32 ComboStepIndex)
{
	UAnimMontage* NextMontage = GetGroundAttackMontage(ComboStepIndex);
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	UAnimInstance* AnimInstance = CharacterMesh ? CharacterMesh->GetAnimInstance() : nullptr;
	if (!bPlayAttackAnimation || !NextMontage || !AnimInstance)
	{
		return false;
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	CurrentGroundComboStep = FMath::Clamp(ComboStepIndex, 1, 4);
	CurrentCombatAnimationPhase = ESideScrollingCombatAnimationPhase::Attacking;
	bAttackAnimationInProgress = true;
	bGroundAttackMontageInProgress = true;
	bGroundComboInputWindowOpen = false;
	bGroundMoveCancelWindowOpen = false;
	bComboInputQueued = false;
	bAirToFloorAttackInProgress = false;
	SetCombatWeaponDrawn(true);

	ActiveGroundAttackMontage = NextMontage;

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ASideScrollingCharacter::OnGroundAttackMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, NextMontage);

	const float Duration = AnimInstance->Montage_Play(NextMontage, FMath::Max(GroundAttackMontagePlayRate, 0.1f));
	if (Duration <= 0.0f)
	{
		ActiveGroundAttackMontage = nullptr;
		return false;
	}

	return true;
}

UAnimMontage* ASideScrollingCharacter::GetGroundAttackMontage(int32 ComboStepIndex) const
{
	switch (ComboStepIndex)
	{
	case 1:
		return GroundAttack1Montage.Get();
	case 2:
		return GroundAttack2Montage.Get();
	case 3:
		return GroundAttack3Montage.Get();
	case 4:
		return GroundAttack4Montage.Get();
	default:
		return nullptr;
	}
}

void ASideScrollingCharacter::OpenGroundComboInputWindow()
{
	if (bGroundAttackMontageInProgress)
	{
		bGroundComboInputWindowOpen = true;
	}
}

void ASideScrollingCharacter::CloseGroundComboInputWindow()
{
	bGroundComboInputWindowOpen = false;
}

void ASideScrollingCharacter::CommitGroundCombo()
{
	if (!bGroundAttackMontageInProgress)
	{
		return;
	}

	if (bComboInputQueued && CurrentGroundComboStep < 4)
	{
		const int32 NextComboStep = CurrentGroundComboStep + 1;
		if (!PlayGroundAttackMontageStep(NextComboStep))
		{
			FinishGroundAttackAndStartSheathe(true);
			return;
		}
	}

	bComboInputQueued = false;
	bGroundComboInputWindowOpen = false;
}

void ASideScrollingCharacter::OpenGroundMoveCancelWindow()
{
	if (bGroundAttackMontageInProgress || CurrentCombatAnimationPhase == ESideScrollingCombatAnimationPhase::AirToFloorEnd)
	{
		bGroundMoveCancelWindowOpen = true;
	}
}

void ASideScrollingCharacter::LoopGroundCombo()
{
	if (!bGroundAttackMontageInProgress || CurrentGroundComboStep < 4)
	{
		return;
	}

	if (bComboInputQueued)
	{
		if (!PlayGroundAttackMontageStep(1))
		{
			FinishGroundAttackAndStartSheathe(true);
		}
		return;
	}

	bGroundComboInputWindowOpen = false;
}

void ASideScrollingCharacter::BeginGroundAttackTrace(int32 ComboStepIndex)
{
	if (bAirToFloorAttackInProgress || CurrentCombatAnimationPhase == ESideScrollingCombatAnimationPhase::AirToFloorEnd)
	{
		PendingAttackFacingSign = LastFacingX >= 0.0f ? 1.0f : -1.0f;
		CurrentAttackDamageMultiplier = 1.35f;
		CurrentAttackKnockbackDistance = 45.0f;
		CurrentWeaponTraceRadius = FMath::Max(WeaponTraceRadius, 30.0f);
		CurrentMinimumWeaponMotionSpeed = 80.0f;
		bCurrentUseAutomaticWeaponMotionHitWindow = true;
		bAttackHitPending = true;
		bAttackRegisteredHit = false;
		BeginAttackHitWindow();
		return;
	}

	const int32 TraceComboStep = FMath::Clamp(ComboStepIndex > 0 ? ComboStepIndex : CurrentGroundComboStep, 1, 4);
	if (CurrentGroundComboStep <= 0)
	{
		CurrentGroundComboStep = TraceComboStep;
	}

	PendingAttackFacingSign = LastFacingX >= 0.0f ? 1.0f : -1.0f;
	CurrentAttackDamageMultiplier = 0.75f + 0.15f * TraceComboStep;
	if (TraceComboStep >= 4)
	{
		CurrentAttackDamageMultiplier = 1.4f;
	}
	CurrentAttackKnockbackDistance = TraceComboStep >= 4 ? 60.0f : 18.0f + 6.0f * TraceComboStep;
	CurrentWeaponTraceRadius = TraceComboStep >= 4 ? 28.0f : 24.0f;
	CurrentMinimumWeaponMotionSpeed = 120.0f;
	bCurrentUseAutomaticWeaponMotionHitWindow = true;
	bAttackHitPending = true;
	bAttackRegisteredHit = false;
	BeginAttackHitWindow();
}

void ASideScrollingCharacter::EndGroundAttackTrace()
{
	EndAttackHitWindow();
}

void ASideScrollingCharacter::FinishGroundAttackMontage()
{
	FinishGroundAttackAndStartSheathe(true);
}

void ASideScrollingCharacter::FinishGroundAttackMontageFromMontage(UAnimMontage* SourceMontage)
{
	if (SourceMontage && SourceMontage != ActiveGroundAttackMontage)
	{
		return;
	}

	FinishGroundAttackMontage();
}

void ASideScrollingCharacter::FinishGroundAttackAndStartSheathe(bool bStopActiveMontage)
{
	if (!bGroundAttackMontageInProgress)
	{
		return;
	}

	if (bAttackHitWindowActive)
	{
		ClearAttackHitWindows();
	}

	UAnimMontage* MontageToStop = ActiveGroundAttackMontage;
	ActiveGroundAttackMontage = nullptr;
	bGroundAttackMontageInProgress = false;
	bGroundComboInputWindowOpen = false;
	bGroundMoveCancelWindowOpen = false;
	bComboInputQueued = false;
	bAttackHitPending = false;
	CurrentGroundComboStep = 0;

	if (bStopActiveMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			if (MontageToStop)
			{
				AnimInstance->Montage_Stop(0.08f, MontageToStop);
			}
		}
	}

	StartSheatheOrRestoreAnimation();
}

void ASideScrollingCharacter::OnGroundAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ActiveGroundAttackMontage)
	{
		return;
	}

	ActiveGroundAttackMontage = nullptr;
	if (bInterrupted)
	{
		FinishGroundAttackMontageState(bInterrupted);
		return;
	}

	FinishGroundAttackAndStartSheathe(false);
}

void ASideScrollingCharacter::FinishGroundAttackMontageState(bool bInterrupted)
{
	if (bAttackHitWindowActive)
	{
		ClearAttackHitWindows();
	}

	bGroundAttackMontageInProgress = false;
	bGroundComboInputWindowOpen = false;
	bGroundMoveCancelWindowOpen = false;
	bAttackAnimationInProgress = false;
	bAttackHitPending = false;
	bComboInputQueued = false;
	CurrentGroundComboStep = 0;
	CurrentCombatAnimationPhase = ESideScrollingCombatAnimationPhase::None;
	ActiveGroundAttackMontage = nullptr;
	ActiveAirToFloorAttackMontage = nullptr;
	SetCombatWeaponDrawn(false);

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		CharacterMesh->GlobalAnimRateScale = 1.0f;
		if (bRestoreMeshTransformAfterAttackAnimation)
		{
			CharacterMesh->SetRelativeTransform(MeshTransformBeforeAttackAnimation);
			UpdateFacingDirection(LastFacingX);
		}
	}
}

void ASideScrollingCharacter::ApplyPendingAttackHit()
{
	if (!bAttackHitPending)
	{
		return;
	}

	bAttackHitPending = false;

	bool bTraceAttempted = false;
	FActiveAttackHitWindow AttackWindow;
	AttackWindow.FacingSign = PendingAttackFacingSign >= 0.0f ? 1.0f : -1.0f;
	AttackWindow.DamageMultiplier = CurrentAttackDamageMultiplier;
	AttackWindow.KnockbackDistance = CurrentAttackKnockbackDistance;
	AttackWindow.WeaponTraceRadius = CurrentWeaponTraceRadius;
	AttackWindow.MinimumWeaponMotionSpeed = CurrentMinimumWeaponMotionSpeed;
	AttackWindow.bUseAutomaticWeaponMotionHitWindow = false;
	AttackWindow.bForceCurrentSegmentHit = true;

	if (bUseWeaponTraceForAttack && ApplyWeaponTraceAttackHit(AttackWindow, bTraceAttempted))
	{
		return;
	}

	if (!bTraceAttempted && ApplyFallbackBoxAttackHit(AttackWindow))
	{
		return;
	}

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Silver, TEXT("Attack missed"));
	}
}

void ASideScrollingCharacter::BeginAttackHitWindow()
{
	if (!bAttackHitPending)
	{
		return;
	}

	FActiveAttackHitWindow& AttackWindow = ActiveAttackHitWindows.AddDefaulted_GetRef();
	AttackWindow.FacingSign = PendingAttackFacingSign >= 0.0f ? 1.0f : -1.0f;
	AttackWindow.DamageMultiplier = CurrentAttackDamageMultiplier;
	AttackWindow.KnockbackDistance = CurrentAttackKnockbackDistance;
	AttackWindow.WeaponTraceRadius = CurrentWeaponTraceRadius;
	AttackWindow.MinimumWeaponMotionSpeed = CurrentMinimumWeaponMotionSpeed;
	AttackWindow.bUseAutomaticWeaponMotionHitWindow = bCurrentUseAutomaticWeaponMotionHitWindow;

	bAttackHitWindowActive = true;
	bAttackHitPending = true;
}

void ASideScrollingCharacter::EndAttackHitWindow()
{
	if (ActiveAttackHitWindows.Num() <= 0)
	{
		RefreshAttackHitWindowState();
		return;
	}

	for (FActiveAttackHitWindow& AttackWindow : ActiveAttackHitWindows)
	{
		AttackWindow.bForceCurrentSegmentHit = true;
	}

	UpdateAttackHitWindow();
	if (ActiveAttackHitWindows.Num() <= 0)
	{
		RefreshAttackHitWindowState();
		return;
	}

	if (!ActiveAttackHitWindows[0].bRegisteredHit && bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Silver, TEXT("Attack missed"));
	}
	ActiveAttackHitWindows.RemoveAt(0);
	FlushDeferredAttackKnockbacks();
	RefreshAttackHitWindowState();
}

void ASideScrollingCharacter::ClearAttackHitWindows(bool bReportMisses)
{
	if (bReportMisses && bShowGameplayDebugMessages && GEngine)
	{
		for (const FActiveAttackHitWindow& AttackWindow : ActiveAttackHitWindows)
		{
			if (!AttackWindow.bRegisteredHit)
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Silver, TEXT("Attack missed"));
			}
		}
	}

	ActiveAttackHitWindows.Empty();
	DeferredAttackKnockbacks.Empty();
	RefreshAttackHitWindowState();
}

void ASideScrollingCharacter::RefreshAttackHitWindowState()
{
	bAttackHitWindowActive = ActiveAttackHitWindows.Num() > 0;
	bAttackHitPending = bAttackHitWindowActive;
	bAttackRegisteredHit = false;
	for (const FActiveAttackHitWindow& AttackWindow : ActiveAttackHitWindows)
	{
		bAttackRegisteredHit |= AttackWindow.bRegisteredHit;
	}
}

void ASideScrollingCharacter::UpdateAttackHitWindow()
{
	if (ActiveAttackHitWindows.Num() <= 0)
	{
		RefreshAttackHitWindowState();
		return;
	}

	bDeferAttackKnockback = true;
	DeferredAttackKnockbacks.Reset();
	for (int32 WindowIndex = ActiveAttackHitWindows.Num() - 1; WindowIndex >= 0; --WindowIndex)
	{
		FActiveAttackHitWindow& AttackWindow = ActiveAttackHitWindows[WindowIndex];
		bool bTraceAttempted = false;
		if (bUseWeaponTraceForAttack && ApplyWeaponTraceAttackHit(AttackWindow, bTraceAttempted))
		{
			AttackWindow.bRegisteredHit = true;
		}

		if (!bTraceAttempted)
		{
			if (ApplyFallbackBoxAttackHit(AttackWindow))
			{
				AttackWindow.bRegisteredHit = true;
			}
			ActiveAttackHitWindows.RemoveAt(WindowIndex);
		}
	}
	bDeferAttackKnockback = false;

	FlushDeferredAttackKnockbacks();
	RefreshAttackHitWindowState();
}

USceneComponent* ASideScrollingCharacter::FindSceneComponentByName(FName ComponentName) const
{
	if (ComponentName.IsNone())
	{
		return nullptr;
	}

	TArray<USceneComponent*> SceneComponents;
	GetComponents<USceneComponent>(SceneComponents);
	const FString TargetComponentName = ComponentName.ToString();
	for (USceneComponent* SceneComponent : SceneComponents)
	{
		if (SceneComponent && (SceneComponent->GetFName() == ComponentName || SceneComponent->GetName().StartsWith(TargetComponentName)))
		{
			return SceneComponent;
		}
	}

	return nullptr;
}

bool ASideScrollingCharacter::ResolveWeaponTraceComponents(USceneComponent*& OutTraceStartComponent, USceneComponent*& OutTraceEndComponent) const
{
	OutTraceStartComponent = nullptr;
	OutTraceEndComponent = nullptr;

	if (bUseSkillWeaponTrace)
	{
		OutTraceStartComponent = FindSceneComponentByName(SkillWeaponTraceStartComponentName);
		OutTraceEndComponent = FindSceneComponentByName(SkillWeaponTraceEndComponentName);
		if (OutTraceStartComponent && OutTraceEndComponent)
		{
			return true;
		}
	}

	OutTraceStartComponent = FindSceneComponentByName(WeaponTraceStartComponentName);
	OutTraceEndComponent = FindSceneComponentByName(WeaponTraceEndComponentName);
	return OutTraceStartComponent && OutTraceEndComponent;
}

bool ASideScrollingCharacter::ApplyWeaponTraceAttackHit(FActiveAttackHitWindow& AttackWindow, bool& bOutTraceAttempted)
{
	bOutTraceAttempted = false;

	USceneComponent* TraceStartComponent = nullptr;
	USceneComponent* TraceEndComponent = nullptr;
	if (!ResolveWeaponTraceComponents(TraceStartComponent, TraceEndComponent))
	{
		if (bShowGameplayDebugMessages && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Orange, TEXT("Weapon trace points missing; using fallback attack box"));
		}
		return false;
	}

	bOutTraceAttempted = true;

	const FVector TraceStart = TraceStartComponent->GetComponentLocation();
	const FVector TraceEnd = TraceEndComponent->GetComponentLocation();
	const float DeltaSeconds = FMath::Max(GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.0f, KINDA_SMALL_NUMBER);
	const float WeaponMotionSpeed = AttackWindow.bHasPreviousWeaponTrace
		? FMath::Max(FVector::Dist(AttackWindow.PreviousWeaponTraceStart, TraceStart), FVector::Dist(AttackWindow.PreviousWeaponTraceEnd, TraceEnd)) / DeltaSeconds
		: 0.0f;
	const bool bWeaponMovingFastEnough = AttackWindow.bForceCurrentSegmentHit || !AttackWindow.bUseAutomaticWeaponMotionHitWindow || (AttackWindow.bHasPreviousWeaponTrace && WeaponMotionSpeed >= AttackWindow.MinimumWeaponMotionSpeed);

	FCollisionShape TraceShape;
	TraceShape.SetSphere(AttackWindow.WeaponTraceRadius);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	AModengEnemy* ClosestEnemy = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max();
	auto CollectClosestEnemy = [&](const TArray<FHitResult>& HitResults, const FVector& DistanceFrom)
	{
		for (const FHitResult& HitResult : HitResults)
		{
			AModengEnemy* Enemy = Cast<AModengEnemy>(HitResult.GetActor());
			if (!Enemy || Enemy->IsDead() || AttackWindow.HitEnemies.Contains(Enemy))
			{
				continue;
			}

			const float DistanceSq = FVector::DistSquared(DistanceFrom, Enemy->GetActorLocation());
			if (DistanceSq < ClosestDistanceSq)
			{
				ClosestDistanceSq = DistanceSq;
				ClosestEnemy = Enemy;
			}

			if (bDrawWeaponTraceDebug)
			{
				DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, AttackWindow.WeaponTraceRadius * 0.6f, 12, WeaponTraceDebugHitColor, false, WeaponTraceDebugDuration, 0, WeaponTraceDebugLineThickness);
			}
		}
	};

	auto SweepWeaponSegment = [&](const FVector& Start, const FVector& End)
	{
		TArray<FHitResult> HitResults;
		if (bWeaponMovingFastEnough)
		{
			GetWorld()->SweepMultiByObjectType(HitResults, Start, End, FQuat::Identity, ObjectParams, TraceShape, QueryParams);
		}
		if (bDrawWeaponTraceDebug)
		{
			const FColor SegmentColor = !bWeaponMovingFastEnough ? FColor::Silver : (HitResults.Num() > 0 ? WeaponTraceDebugHitColor : WeaponTraceDebugColor);
			DrawDebugLine(GetWorld(), Start, End, SegmentColor, false, WeaponTraceDebugDuration, 0, WeaponTraceDebugLineThickness);
			DrawDebugSphere(GetWorld(), Start, AttackWindow.WeaponTraceRadius, 12, SegmentColor, false, WeaponTraceDebugDuration, 0, WeaponTraceDebugLineThickness);
			DrawDebugSphere(GetWorld(), End, AttackWindow.WeaponTraceRadius, 12, SegmentColor, false, WeaponTraceDebugDuration, 0, WeaponTraceDebugLineThickness);
		}
		CollectClosestEnemy(HitResults, Start);
	};

	SweepWeaponSegment(TraceStart, TraceEnd);
	if (AttackWindow.bHasPreviousWeaponTrace && bWeaponMovingFastEnough)
	{
		SweepWeaponSegment(AttackWindow.PreviousWeaponTraceStart, TraceStart);
		SweepWeaponSegment(AttackWindow.PreviousWeaponTraceEnd, TraceEnd);
	}

	AttackWindow.PreviousWeaponTraceStart = TraceStart;
	AttackWindow.PreviousWeaponTraceEnd = TraceEnd;
	AttackWindow.bHasPreviousWeaponTrace = true;
	AttackWindow.bForceCurrentSegmentHit = false;

	if (!ClosestEnemy)
	{
		return false;
	}

	AttackWindow.HitEnemies.Add(ClosestEnemy);
	DamageEnemyFromAttack(ClosestEnemy, AttackWindow);
	return true;
}

bool ASideScrollingCharacter::ApplyFallbackBoxAttackHit(FActiveAttackHitWindow& AttackWindow)
{
	const float CurrentRange = GetCurrentAttackRange();
	const float CurrentRadius = GetCurrentAttackRadius();
	const FVector AttackCenter = GetActorLocation() + FVector(AttackWindow.FacingSign * CurrentRange * 0.5f, 0.0f, 35.0f);

	FCollisionShape AttackBox;
	AttackBox.SetBox(FVector3f(CurrentRange * 0.5f, CurrentRadius, CurrentRadius));

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	TArray<FOverlapResult> OverlapResults;
	GetWorld()->OverlapMultiByObjectType(OverlapResults, AttackCenter, FQuat::Identity, ObjectParams, AttackBox, QueryParams);
	if (bDrawWeaponTraceDebug)
	{
		DrawDebugBox(GetWorld(), AttackCenter, FVector(CurrentRange * 0.5f, CurrentRadius, CurrentRadius), OverlapResults.Num() > 0 ? WeaponTraceDebugHitColor : WeaponTraceDebugColor, false, WeaponTraceDebugDuration, 0, WeaponTraceDebugLineThickness);
	}

	AModengEnemy* ClosestEnemy = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max();
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AModengEnemy* Enemy = Cast<AModengEnemy>(OverlapResult.GetActor());
		if (!Enemy || Enemy->IsDead() || AttackWindow.HitEnemies.Contains(Enemy))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(GetActorLocation(), Enemy->GetActorLocation());
		if (DistanceSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			ClosestEnemy = Enemy;
		}
	}

	if (!ClosestEnemy)
	{
		return false;
	}

	AttackWindow.HitEnemies.Add(ClosestEnemy);
	DamageEnemyFromAttack(ClosestEnemy, AttackWindow);
	return true;
}

void ASideScrollingCharacter::DamageEnemyFromAttack(AModengEnemy* Enemy, const FActiveAttackHitWindow& AttackWindow)
{
	if (!Enemy || Enemy->IsDead())
	{
		return;
	}

	Enemy->ApplyDamageToEnemy(GetCurrentAttackDamage() * AttackWindow.DamageMultiplier, this);
	if (!Enemy->IsDead())
	{
		if (bDeferAttackKnockback)
		{
			FDeferredAttackKnockback& DeferredKnockback = DeferredAttackKnockbacks.AddDefaulted_GetRef();
			DeferredKnockback.Enemy = Enemy;
			DeferredKnockback.FacingSign = AttackWindow.FacingSign;
			DeferredKnockback.Distance = AttackWindow.KnockbackDistance;
		}
		else
		{
			Enemy->AddActorWorldOffset(FVector(AttackWindow.FacingSign * AttackWindow.KnockbackDistance, 0.0f, 0.0f), false);
		}
	}

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Player hit enemy"));
	}
}

void ASideScrollingCharacter::ConfigurePlayerVisuals()
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh)
	{
		return;
	}

	if (PlayerSkeletalMesh)
	{
		CharacterMesh->SetSkeletalMesh(PlayerSkeletalMesh);
	}

	CharacterMesh->SetRelativeLocation(PlayerMeshRelativeLocation);
	CharacterMesh->SetRelativeRotation(PlayerMeshRelativeRotation);
	CharacterMesh->SetRelativeScale3D(PlayerMeshScale);
	CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CharacterMesh->SetGenerateOverlapEvents(false);
	CharacterMesh->SetHiddenInGame(PlayerSkeletalMesh == nullptr);
	CharacterMesh->SetVisibility(PlayerSkeletalMesh != nullptr);

	if (PlayerAnimClass)
	{
		CharacterMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		CharacterMesh->SetAnimInstanceClass(PlayerAnimClass);
	}
}

float ASideScrollingCharacter::PlayAirToFloorAnimation(UAnimSequenceBase* AnimationToPlay, bool bLooping, ESideScrollingCombatAnimationPhase NewAnimationPhase)
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!bPlayAttackAnimation || !AnimationToPlay || !CharacterMesh)
	{
		return 0.0f;
	}

	GetWorld()->GetTimerManager().ClearTimer(AttackAnimationTimer);

	if (!bAttackAnimationInProgress)
	{
		MeshTransformBeforeAttackAnimation = CharacterMesh->GetRelativeTransform();
	}

	bAttackAnimationInProgress = true;
	CurrentCombatAnimationPhase = NewAnimationPhase;
	CharacterMesh->PlayAnimation(AnimationToPlay, bLooping);
	CharacterMesh->GlobalAnimRateScale = FMath::Max(AttackAnimationPlayRate, 0.1f);

	const float Duration = AnimationToPlay->GetPlayLength() / CharacterMesh->GlobalAnimRateScale;
	if (!bLooping && Duration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(AttackAnimationTimer, this, &ASideScrollingCharacter::FinishAttackAnimation, Duration, false);
	}

	return Duration;
}

bool ASideScrollingCharacter::PlayCombatTransitionAnimation(UAnimSequenceBase* AnimationToPlay, ESideScrollingCombatAnimationPhase NewAnimationPhase)
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!bPlayAttackAnimation || !AnimationToPlay || !CharacterMesh)
	{
		return false;
	}

	GetWorld()->GetTimerManager().ClearTimer(AttackAnimationTimer);
	ActiveCombatTransitionMontage = nullptr;

	const float SafePlayRate = FMath::Max(CombatTransitionAnimationPlayRate, 0.1f);
	if (!bAttackAnimationInProgress)
	{
		MeshTransformBeforeAttackAnimation = CharacterMesh->GetRelativeTransform();
	}

	if (PlayerAnimClass && CharacterMesh->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
	{
		CharacterMesh->GlobalAnimRateScale = 1.0f;
		CharacterMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		CharacterMesh->SetAnimInstanceClass(PlayerAnimClass);
	}

	bAttackAnimationInProgress = true;
	CurrentCombatAnimationPhase = NewAnimationPhase;

	if (UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance())
	{
		if (UAnimMontage* TransitionMontage = AnimInstance->PlaySlotAnimationAsDynamicMontage(
			AnimationToPlay,
			CombatTransitionSlotName,
			FMath::Max(CombatTransitionBlendInTime, 0.0f),
			FMath::Max(CombatTransitionBlendOutTime, 0.0f),
			SafePlayRate))
		{
			ActiveCombatTransitionMontage = TransitionMontage;

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &ASideScrollingCharacter::OnCombatTransitionMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, TransitionMontage);
			return true;
		}
	}

	CharacterMesh->PlayAnimation(AnimationToPlay, false);
	CharacterMesh->GlobalAnimRateScale = SafePlayRate;

	const float Duration = AnimationToPlay->GetPlayLength() / SafePlayRate;
	if (Duration <= 0.0f)
	{
		RestorePlayerAnimationBlueprint();
		ActiveCombatTransitionMontage = nullptr;
		return false;
	}

	GetWorld()->GetTimerManager().SetTimer(AttackAnimationTimer, this, &ASideScrollingCharacter::FinishAttackAnimation, Duration, false);
	return true;
}

void ASideScrollingCharacter::StartSheatheOrRestoreAnimation()
{
	if (bPlayCombatTransitionAnimations && PlayCombatTransitionAnimation(CombatToIdleAnimation.Get(), ESideScrollingCombatAnimationPhase::Sheathing))
	{
		return;
	}

	RestorePlayerAnimationBlueprint();
	ResetAttackCombo();
	SetCombatWeaponDrawn(false);
}

void ASideScrollingCharacter::FinishCombatTransitionState()
{
	RestorePlayerAnimationBlueprint();
	ResetAttackCombo();
	SetCombatWeaponDrawn(false);
}

void ASideScrollingCharacter::OnCombatTransitionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ActiveCombatTransitionMontage)
	{
		return;
	}

	ActiveCombatTransitionMontage = nullptr;
	FinishCombatTransitionState();
}

void ASideScrollingCharacter::SetCombatWeaponDrawn(bool bDrawn)
{
	if (bUseSkillWeaponTrace)
	{
		for (FActiveAttackHitWindow& AttackWindow : ActiveAttackHitWindows)
		{
			AttackWindow.bHasPreviousWeaponTrace = false;
		}
	}
	bUseSkillWeaponTrace = false;

	if (USceneComponent* HandSword = FindSceneComponentByName(TEXT("Sword_Hand")))
	{
		HandSword->SetVisibility(bDrawn, true);
		HandSword->SetHiddenInGame(!bDrawn, true);
	}

	if (USceneComponent* SkillHandSword = FindSceneComponentByName(TEXT("Sword_SkillHand")))
	{
		SkillHandSword->SetVisibility(false, true);
		SkillHandSword->SetHiddenInGame(true, true);
	}

	if (USceneComponent* BoneSword = FindSceneComponentByName(TEXT("Sword_Bone")))
	{
		BoneSword->SetVisibility(false, true);
		BoneSword->SetHiddenInGame(true, true);
	}

	if (USceneComponent* SheathedSword = FindSceneComponentByName(TEXT("Sword_InScabbard")))
	{
		SheathedSword->SetVisibility(!bDrawn, true);
		SheathedSword->SetHiddenInGame(bDrawn, true);
	}

	if (USceneComponent* Scabbard = FindSceneComponentByName(TEXT("Sword_Sheathed")))
	{
		Scabbard->SetVisibility(true, true);
		Scabbard->SetHiddenInGame(false, true);
	}
}

bool ASideScrollingCharacter::SetSceneComponentVisibleByName(FName ComponentName, bool bVisible, bool bPropagateToChildren)
{
	USceneComponent* SceneComponent = FindSceneComponentByName(ComponentName);
	if (!SceneComponent)
	{
		return false;
	}

	SceneComponent->SetVisibility(bVisible, bPropagateToChildren);
	SceneComponent->SetHiddenInGame(!bVisible, bPropagateToChildren);
	return true;
}

void ASideScrollingCharacter::SetCombatWeaponDrawnForNotify(bool bDrawn)
{
	SetCombatWeaponDrawn(bDrawn);
}

void ASideScrollingCharacter::SetAnimationWeaponModeForNotify(bool bUseSkeletalWeapon, bool bUseSheathedWeapon, FName SkeletalWeaponComponentName, FName SocketWeaponComponentName)
{
	if (bUseSheathedWeapon)
	{
		SetCombatWeaponDrawn(false);
	}
	else
	{
		SetCombatWeaponDrawn(true);
	}

	if (!SkeletalWeaponComponentName.IsNone())
	{
		SetSceneComponentVisibleByName(SkeletalWeaponComponentName, bUseSkeletalWeapon, true);
	}

	if (!SocketWeaponComponentName.IsNone())
	{
		SetSceneComponentVisibleByName(SocketWeaponComponentName, !bUseSkeletalWeapon && !bUseSheathedWeapon, true);
	}
}

void ASideScrollingCharacter::FlushDeferredAttackKnockbacks()
{
	for (const FDeferredAttackKnockback& DeferredKnockback : DeferredAttackKnockbacks)
	{
		AModengEnemy* Enemy = DeferredKnockback.Enemy.Get();
		if (Enemy && !Enemy->IsDead())
		{
			Enemy->AddActorWorldOffset(FVector(DeferredKnockback.FacingSign * DeferredKnockback.Distance, 0.0f, 0.0f), false);
		}
	}

	DeferredAttackKnockbacks.Empty();
}

void ASideScrollingCharacter::SetSkillWeaponModeForNotify(FName WeaponModeName, FName NormalHandComponentName, FName SkillHandComponentName, FName BoneComponentName)
{
	if (WeaponModeName == FName(TEXT("InScabbard")) || WeaponModeName == FName(TEXT("Sheathed")))
	{
		SetCombatWeaponDrawn(false);
		return;
	}

	const bool bNormalHand = WeaponModeName == FName(TEXT("NormalHand"));
	const bool bSkillHand = WeaponModeName == FName(TEXT("SkillHand"));
	const bool bBone = WeaponModeName == FName(TEXT("SwordBone")) || WeaponModeName == FName(TEXT("Bone"));
	const bool bShouldUseSkillWeaponTrace = bSkillHand;
	if (bUseSkillWeaponTrace != bShouldUseSkillWeaponTrace)
	{
		for (FActiveAttackHitWindow& AttackWindow : ActiveAttackHitWindows)
		{
			AttackWindow.bHasPreviousWeaponTrace = false;
		}
	}
	bUseSkillWeaponTrace = bShouldUseSkillWeaponTrace;

	if (USceneComponent* NormalHand = FindSceneComponentByName(NormalHandComponentName))
	{
		NormalHand->SetVisibility(bNormalHand, true);
		NormalHand->SetHiddenInGame(!bNormalHand, true);
	}

	if (USceneComponent* SkillHand = FindSceneComponentByName(SkillHandComponentName))
	{
		SkillHand->SetVisibility(bSkillHand, true);
		SkillHand->SetHiddenInGame(!bSkillHand, true);
	}

	if (USceneComponent* Bone = FindSceneComponentByName(BoneComponentName))
	{
		Bone->SetVisibility(bBone, true);
		Bone->SetHiddenInGame(!bBone, true);
	}
}

bool ASideScrollingCharacter::PlayHitReaction(AActor* DamageSource)
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	UAnimInstance* AnimInstance = CharacterMesh ? CharacterMesh->GetAnimInstance() : nullptr;
	if (!CharacterMesh || !AnimInstance || !HitReactionAnimation)
	{
		return false;
	}

	GetWorld()->GetTimerManager().ClearTimer(HitReactionTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackAnimationTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackHitTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackHitWindowStartTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackHitWindowEndTimer);
	GetWorld()->GetTimerManager().ClearTimer(SkillReleaseTimer);

	if (bAttackHitWindowActive)
	{
		ClearAttackHitWindows();
	}

	if (ActiveGroundAttackMontage)
	{
		AnimInstance->Montage_Stop(0.04f, ActiveGroundAttackMontage);
		ActiveGroundAttackMontage = nullptr;
	}

	if (ActiveAirToFloorAttackMontage)
	{
		AnimInstance->Montage_Stop(0.04f, ActiveAirToFloorAttackMontage);
		ActiveAirToFloorAttackMontage = nullptr;
	}

	if (ActiveCombatTransitionMontage)
	{
		AnimInstance->Montage_Stop(0.04f, ActiveCombatTransitionMontage);
		ActiveCombatTransitionMontage = nullptr;
	}

	if (ActiveRollMontage)
	{
		AnimInstance->Montage_Stop(0.04f, ActiveRollMontage);
		ActiveRollMontage = nullptr;
	}

	if (ActiveSkillMontage)
	{
		AnimInstance->Montage_Stop(0.04f, ActiveSkillMontage);
		ActiveSkillMontage = nullptr;
	}

	if (ActiveHitReactionMontage)
	{
		AnimInstance->Montage_Stop(0.02f, ActiveHitReactionMontage);
		ActiveHitReactionMontage = nullptr;
	}

	if (!bAttackAnimationInProgress)
	{
		MeshTransformBeforeAttackAnimation = CharacterMesh->GetRelativeTransform();
	}

	RestoreRollFallingMovement();
	bHitReactionInProgress = true;
	bAttackAnimationInProgress = true;
	bAttackHitPending = false;
	bComboInputQueued = false;
	bGroundAttackMontageInProgress = false;
	bGroundComboInputWindowOpen = false;
	bGroundMoveCancelWindowOpen = false;
	bAirToFloorAttackInProgress = false;
	bRollInProgress = false;
	bRollCancelWindowOpen = false;
	bRollInvincible = false;
	bRollAttackQueued = false;
	bRollJumpQueued = false;
	bSkillReleaseInProgress = false;
	RollMoveQueuedValue = 0.0f;
	CurrentCombatAnimationPhase = ESideScrollingCombatAnimationPhase::HitReact;
	SetSkillWeaponModeForNotify(TEXT("InScabbard"));
	SetCombatWeaponDrawn(false);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		if (HitReactionKnockbackImpulse > 0.0f && DamageSource)
		{
			const float AwayFromDamageX = FMath::Sign(GetActorLocation().X - DamageSource->GetActorLocation().X);
			const float SafeAwayFromDamageX = FMath::IsNearlyZero(AwayFromDamageX) ? -LastFacingX : AwayFromDamageX;
			MovementComponent->AddImpulse(FVector(SafeAwayFromDamageX * HitReactionKnockbackImpulse, 0.0f, 0.0f), true);
		}
	}

	const float SafePlayRate = FMath::Max(HitReactionPlayRate, 0.1f);
	ActiveHitReactionMontage = AnimInstance->PlaySlotAnimationAsDynamicMontage(
		HitReactionAnimation.Get(),
		HitReactionSlotName,
		FMath::Max(HitReactionBlendInTime, 0.0f),
		FMath::Max(HitReactionBlendOutTime, 0.0f),
		SafePlayRate);

	const float LockDuration = HitReactionLockDuration > 0.0f
		? HitReactionLockDuration
		: HitReactionAnimation->GetPlayLength() / SafePlayRate;

	if (ActiveHitReactionMontage)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ASideScrollingCharacter::OnHitReactionMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, ActiveHitReactionMontage);
	}

	if (LockDuration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(HitReactionTimer, this, &ASideScrollingCharacter::FinishHitReaction, LockDuration, false);
	}
	else
	{
		FinishHitReaction();
	}

	return true;
}

void ASideScrollingCharacter::FinishHitReaction()
{
	GetWorld()->GetTimerManager().ClearTimer(HitReactionTimer);
	if (!bHitReactionInProgress)
	{
		return;
	}

	bHitReactionInProgress = false;
	UAnimMontage* MontageToStop = ActiveHitReactionMontage;
	ActiveHitReactionMontage = nullptr;
	if (MontageToStop)
	{
		if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			AnimInstance->Montage_Stop(FMath::Max(HitReactionBlendOutTime, 0.0f), MontageToStop);
		}
	}

	if (!bPlayerDefeated)
	{
		RestorePlayerAnimationBlueprint();
		ResetAttackCombo();
	}
}

void ASideScrollingCharacter::OnHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ActiveHitReactionMontage)
	{
		return;
	}

	FinishHitReaction();
}

void ASideScrollingCharacter::ClearDamageInvulnerability()
{
	bDamageInvulnerable = false;
}

bool ASideScrollingCharacter::CanStartSkill() const
{
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	const bool bIsFalling = MovementComponent && MovementComponent->IsFalling();

	return !bPlayerDefeated
		&& !bHitReactionInProgress
		&& !bSkillReleaseInProgress
		&& !bRollInProgress
		&& !bIsFalling;
}

void ASideScrollingCharacter::StartSkillRelease()
{
	if (!CanStartSkill())
	{
		return;
	}

	if (bAttackAnimationInProgress)
	{
		if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			if (ActiveGroundAttackMontage)
			{
				AnimInstance->Montage_Stop(0.04f, ActiveGroundAttackMontage);
			}

			if (ActiveAirToFloorAttackMontage)
			{
				AnimInstance->Montage_Stop(0.04f, ActiveAirToFloorAttackMontage);
			}

			if (ActiveCombatTransitionMontage)
			{
				AnimInstance->Montage_Stop(0.04f, ActiveCombatTransitionMontage);
			}
		}

		InterruptAttackAnimation();
	}

	bSkillReleaseInProgress = true;
	ActionValueY = 0.0f;
	DropValue = 0.0f;
	MeshTransformBeforeAttackAnimation = GetMesh() ? GetMesh()->GetRelativeTransform() : MeshTransformBeforeAttackAnimation;

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	OnSkillInputPressed();

	const float AnimationDuration = PlaySkillReleaseAnimation();
	const float SafeReleaseDuration = AnimationDuration > 0.0f ? AnimationDuration : FMath::Max(SkillReleaseDuration, 0.0f);
	if (SafeReleaseDuration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(SkillReleaseTimer, this, &ASideScrollingCharacter::FinishSkillRelease, SafeReleaseDuration, false);
	}
	else
	{
		FinishSkillRelease();
	}
}

void ASideScrollingCharacter::FinishSkillRelease()
{
	GetWorld()->GetTimerManager().ClearTimer(SkillReleaseTimer);
	if (!bSkillReleaseInProgress)
	{
		return;
	}

	StopActiveSkillMontage(SkillBlendOutTime);
	SetSkillWeaponModeForNotify(TEXT("InScabbard"));
	bSkillReleaseInProgress = false;
	OnSkillReleaseFinished();
}

float ASideScrollingCharacter::PlaySkillReleaseAnimation()
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	UAnimInstance* AnimInstance = CharacterMesh ? CharacterMesh->GetAnimInstance() : nullptr;
	if (!CharacterMesh || !AnimInstance)
	{
		return 0.0f;
	}

	StopActiveSkillMontage(0.0f);

	const float SafePlayRate = FMath::Max(SkillPlayRate, 0.1f);
	const bool bSkillMontageUsesExpectedSlot = SkillMontage && SkillMontage->SlotAnimTracks.ContainsByPredicate(
		[this](const FSlotAnimationTrack& SlotTrack)
		{
			return SlotTrack.SlotName == SkillSlotName;
		});

	float Duration = 0.0f;
	if (bSkillMontageUsesExpectedSlot)
	{
		ActiveSkillMontage = SkillMontage.Get();
		Duration = AnimInstance->Montage_Play(ActiveSkillMontage, SafePlayRate);
		if (Duration <= 0.0f)
		{
			ActiveSkillMontage = nullptr;
		}
	}

	if (!ActiveSkillMontage && SkillAnimation)
	{
		ActiveSkillMontage = AnimInstance->PlaySlotAnimationAsDynamicMontage(
			SkillAnimation.Get(),
			SkillSlotName,
			FMath::Max(SkillBlendInTime, 0.0f),
			FMath::Max(SkillBlendOutTime, 0.0f),
			SafePlayRate);

		Duration = ActiveSkillMontage ? SkillAnimation->GetPlayLength() / SafePlayRate : 0.0f;
	}

	if (ActiveSkillMontage)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ASideScrollingCharacter::OnSkillMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, ActiveSkillMontage);
	}
	else if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("Skill animation failed"));
	}

	return Duration;
}

void ASideScrollingCharacter::StopActiveSkillMontage(float BlendOutTime)
{
	UAnimMontage* MontageToStop = ActiveSkillMontage;
	ActiveSkillMontage = nullptr;
	if (!MontageToStop)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->Montage_Stop(FMath::Max(BlendOutTime, 0.0f), MontageToStop);
	}
}

void ASideScrollingCharacter::OnSkillMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ActiveSkillMontage)
	{
		return;
	}

	FinishSkillRelease();
}

void ASideScrollingCharacter::BeginRollInvincible()
{
	if (bRollInProgress)
	{
		bRollInvincible = true;
	}
}

void ASideScrollingCharacter::EndRollInvincible()
{
	bRollInvincible = false;
}

void ASideScrollingCharacter::OpenRollCancelWindow()
{
	if (bRollInProgress)
	{
		bRollCancelWindowOpen = true;
		TryConsumeRollQueuedInput();
	}
}

void ASideScrollingCharacter::FinishRoll()
{
	StopActiveRollMontage(0.08f);
	ActiveRollMontage = nullptr;
	bRollInProgress = false;
	bRollCancelWindowOpen = false;
	bRollInvincible = false;
	bRollAttackQueued = false;
	bRollJumpQueued = false;
	RollMoveQueuedValue = 0.0f;
	RestorePlayerAnimationBlueprint();
}

void ASideScrollingCharacter::OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ActiveRollMontage)
	{
		return;
	}

	FinishRoll();
}

void ASideScrollingCharacter::StopActiveRollMontage(float BlendOutTime)
{
	UAnimMontage* MontageToStop = ActiveRollMontage;
	ActiveRollMontage = nullptr;
	if (!MontageToStop)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->Montage_Stop(FMath::Max(BlendOutTime, 0.0f), MontageToStop);
	}
}

void ASideScrollingCharacter::PauseFallingForAirRoll()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent || !MovementComponent->IsFalling() || bRollPausedFalling)
	{
		return;
	}

	bRollPausedFalling = true;
	SavedRollGravityScale = MovementComponent->GravityScale;
	MovementComponent->Velocity.Z = 0.0f;
	MovementComponent->GravityScale = 0.0f;
}

void ASideScrollingCharacter::RestoreRollFallingMovement()
{
	if (!bRollPausedFalling)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->GravityScale = SavedRollGravityScale;
		if (MovementComponent->IsFalling())
		{
			MovementComponent->Velocity.Z = FMath::Min(MovementComponent->Velocity.Z, 0.0f);
		}
	}

	bRollPausedFalling = false;
}

void ASideScrollingCharacter::TryConsumeRollQueuedInput()
{
	if (!bRollInProgress || !bRollCancelWindowOpen)
	{
		return;
	}

	if (bRollAttackQueued)
	{
		StopActiveRollMontage(0.04f);
		bRollInProgress = false;
		bRollCancelWindowOpen = false;
		bRollInvincible = false;
		bRollAttackQueued = false;
		bRollJumpQueued = false;
		RollMoveQueuedValue = 0.0f;
		RestorePlayerAnimationBlueprint();
		DoAttack();
		return;
	}

	if (bRollJumpQueued)
	{
		StopActiveRollMontage(0.04f);
		bRollInProgress = false;
		bRollCancelWindowOpen = false;
		bRollInvincible = false;
		bRollAttackQueued = false;
		bRollJumpQueued = false;
		RollMoveQueuedValue = 0.0f;
		RestorePlayerAnimationBlueprint();
		DoJumpStart();
		return;
	}

	if (!FMath::IsNearlyZero(RollMoveQueuedValue))
	{
		const float QueuedMove = RollMoveQueuedValue;
		FinishRoll();
		DoMove(QueuedMove);
	}
}

void ASideScrollingCharacter::FinishAttackAnimation()
{
	if (CurrentCombatAnimationPhase == ESideScrollingCombatAnimationPhase::Sheathing)
	{
		FinishCombatTransitionState();
		return;
	}

	if (CurrentCombatAnimationPhase == ESideScrollingCombatAnimationPhase::AirToFloorStart)
	{
		BeginAirToFloorLoop();
		return;
	}

	if (CurrentCombatAnimationPhase == ESideScrollingCombatAnimationPhase::AirToFloorEnd)
	{
		StartSheatheOrRestoreAnimation();
		return;
	}

	if (bAttackHitWindowActive)
	{
		ClearAttackHitWindows();
	}
	else if (bAttackHitPending)
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackHitTimer);
		ApplyPendingAttackHit();
	}

	StartSheatheOrRestoreAnimation();
}

void ASideScrollingCharacter::RestorePlayerAnimationBlueprint()
{
	RestoreRollFallingMovement();

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh)
	{
		return;
	}

	CharacterMesh->GlobalAnimRateScale = 1.0f;
	if (bRestoreMeshTransformAfterAttackAnimation)
	{
		CharacterMesh->SetRelativeTransform(MeshTransformBeforeAttackAnimation);
		UpdateFacingDirection(LastFacingX);
	}

	bAttackAnimationInProgress = false;
	bAttackHitPending = false;
	bComboInputQueued = false;
	bAirToFloorAttackInProgress = false;
	ActiveCombatTransitionMontage = nullptr;
	ActiveAirToFloorAttackMontage = nullptr;
	ActiveRollMontage = nullptr;
	ActiveSkillMontage = nullptr;
	bRollInProgress = false;
	bRollCancelWindowOpen = false;
	bRollInvincible = false;
	bRollAttackQueued = false;
	bRollJumpQueued = false;
	bRollPausedFalling = false;
	bSkillReleaseInProgress = false;
	RollMoveQueuedValue = 0.0f;
	CurrentCombatAnimationPhase = ESideScrollingCombatAnimationPhase::None;

	if (PlayerAnimClass && CharacterMesh->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
	{
		CharacterMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		CharacterMesh->SetAnimInstanceClass(PlayerAnimClass);
	}
}

void ASideScrollingCharacter::InterruptAttackAnimation()
{
	if (!bAttackAnimationInProgress)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(AttackAnimationTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackHitTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackHitWindowStartTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackHitWindowEndTimer);
	GetWorld()->GetTimerManager().ClearTimer(SkillReleaseTimer);
	StopActiveSkillMontage(0.04f);
	ClearAttackHitWindows();
	bComboInputQueued = false;
	bAirToFloorAttackInProgress = false;
	bRollInProgress = false;
	bRollCancelWindowOpen = false;
	bRollInvincible = false;
	bRollAttackQueued = false;
	bRollJumpQueued = false;
	bSkillReleaseInProgress = false;
	RollMoveQueuedValue = 0.0f;
	RestorePlayerAnimationBlueprint();
	ResetAttackCombo();
}

void ASideScrollingCharacter::ResetAttackCombo()
{
	RestoreRollFallingMovement();

	CurrentAttackDamageMultiplier = 1.0f;
	CurrentAttackKnockbackDistance = AttackKnockbackDistance;
	CurrentWeaponTraceRadius = WeaponTraceRadius;
	CurrentMinimumWeaponMotionSpeed = 180.0f;
	CurrentGroundComboStep = 0;
	CurrentCombatAnimationPhase = ESideScrollingCombatAnimationPhase::None;
	bCurrentUseAutomaticWeaponMotionHitWindow = false;
	bComboInputQueued = false;
	bGroundAttackMontageInProgress = false;
	bGroundComboInputWindowOpen = false;
	bGroundMoveCancelWindowOpen = false;
	bAirToFloorAttackInProgress = false;
	bRollInProgress = false;
	bRollCancelWindowOpen = false;
	bRollInvincible = false;
	bRollAttackQueued = false;
	bRollJumpQueued = false;
	bRollPausedFalling = false;
	bSkillReleaseInProgress = false;
	RollMoveQueuedValue = 0.0f;
	ActiveGroundAttackMontage = nullptr;
	ActiveAirToFloorAttackMontage = nullptr;
	ActiveHitReactionMontage = nullptr;
	ActiveRollMontage = nullptr;
	ActiveSkillMontage = nullptr;
}

void ASideScrollingCharacter::UpdateFacingDirection(float FacingSign)
{
	if (!bLockFacingToSideScrollingAxis)
	{
		return;
	}

	const float DesiredFacingYaw = FacingSign >= 0.0f ? FacingYawRight : FacingYawLeft;
	const float MeshFacingOffsetYaw = DesiredFacingYaw - FacingYawRight;

	SetActorRotation(FRotator(0.0f, FacingYawRight, 0.0f));

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		FRotator MeshFacingRotation = PlayerMeshRelativeRotation;
		MeshFacingRotation.Yaw += MeshFacingOffsetYaw;
		CharacterMesh->SetRelativeRotation(MeshFacingRotation);
	}
}

void ASideScrollingCharacter::AddInk(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	CurrentInk += Amount;

	if (bShowGameplayDebugMessages && GEngine)
	{
		const FString Message = FString::Printf(TEXT("Ink +%d  Total: %d"), Amount, CurrentInk);
		GEngine->AddOnScreenDebugMessage(-1, 1.2f, FColor::Cyan, Message);
	}

	TryUpgradeWeapon();
}

int32 ASideScrollingCharacter::GetWeaponLevel() const
{
	return WeaponLevel;
}

int32 ASideScrollingCharacter::GetCurrentInk() const
{
	return CurrentInk;
}

float ASideScrollingCharacter::GetInkProgressPercent() const
{
	if (WeaponLevel >= MaxWeaponLevel)
	{
		return 1.0f;
	}

	if (InkNeededPerWeaponLevel <= 0)
	{
		return 0.0f;
	}

	return FMath::Clamp(static_cast<float>(CurrentInk) / static_cast<float>(InkNeededPerWeaponLevel), 0.0f, 1.0f);
}

float ASideScrollingCharacter::GetCurrentAttackDamage() const
{
	return AttackDamage + DamageGainPerWeaponLevel * (WeaponLevel - 1);
}

float ASideScrollingCharacter::GetCurrentAttackRange() const
{
	return AttackRange + RangeGainPerWeaponLevel * (WeaponLevel - 1);
}

float ASideScrollingCharacter::GetCurrentAttackRadius() const
{
	return AttackRadius + RadiusGainPerWeaponLevel * (WeaponLevel - 1);
}

bool ASideScrollingCharacter::ApplyDamageToPlayer(float DamageAmount, AActor* DamageSource)
{
	if (DamageAmount <= 0.0f || bPlayerDefeated)
	{
		return false;
	}

	if (bDamageInvulnerable || (bIgnoreDamageWhileRolling && bRollInvincible) || (bSkillGrantsInvulnerability && bSkillReleaseInProgress))
	{
		if (bShowGameplayDebugMessages && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.8f, FColor::Cyan, TEXT("Player avoided damage"));
		}
		return false;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	bDamageInvulnerable = true;
	GetWorld()->GetTimerManager().ClearTimer(HitInvulnerabilityTimer);
	GetWorld()->GetTimerManager().SetTimer(HitInvulnerabilityTimer, this, &ASideScrollingCharacter::ClearDamageInvulnerability, HitInvulnerabilityDuration, false);

	if (CurrentHealth <= 0.0f)
	{
		bPlayerDefeated = true;
		InterruptAttackAnimation();
		PlayHitReaction(DamageSource);
		if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			MovementComponent->DisableMovement();
		}
	}
	else
	{
		PlayHitReaction(DamageSource);
	}

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Player HP: %.0f / %.0f"), CurrentHealth, MaxHealth));
	}

	return true;
}

float ASideScrollingCharacter::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}

	return CurrentHealth / MaxHealth;
}

bool ASideScrollingCharacter::IsPlayerDefeated() const
{
	return bPlayerDefeated;
}

void ASideScrollingCharacter::TryUpgradeWeapon()
{
	while (WeaponLevel < MaxWeaponLevel && CurrentInk >= InkNeededPerWeaponLevel)
	{
		CurrentInk -= InkNeededPerWeaponLevel;
		WeaponLevel++;

		if (bShowGameplayDebugMessages && GEngine)
		{
			const FString Message = FString::Printf(TEXT("Weapon upgraded to Lv.%d"), WeaponLevel);
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Emerald, Message);
		}
	}
}

void ASideScrollingCharacter::MultiJump()
{
	// does the user want to drop to a lower platform?
	if (DropValue > 0.0f)
	{
		CheckForSoftCollision();
		return;
	}

	// reset the drop value
	DropValue = 0.0f;

	// if we're grounded, disregard advanced jump logic
	if (!GetCharacterMovement()->IsFalling())
	{
		Jump();
		return;
	}

	// if we have a horizontal input, try for wall jump first
	if (!bHasWallJumped && !FMath::IsNearlyZero(ActionValueY))
	{
		// trace ahead of the character for walls
		FHitResult OutHit;

		const FVector Start = GetActorLocation();
		const FVector End = Start + (FVector(ActionValueY > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f) * WallJumpTraceDistance);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

		if (OutHit.bBlockingHit)
		{
			// rotate to the bounce direction
			const FRotator BounceRot = UKismetMathLibrary::MakeRotFromX(OutHit.ImpactNormal);
			SetActorRotation(FRotator(0.0f, BounceRot.Yaw, 0.0f));

			// calculate the impulse vector
			FVector WallJumpImpulse = OutHit.ImpactNormal * WallJumpHorizontalImpulse;
			WallJumpImpulse.Z = GetCharacterMovement()->JumpZVelocity * WallJumpVerticalMultiplier;

			// launch the character away from the wall
			LaunchCharacter(WallJumpImpulse, true, true);

			// enable wall jump lockout for a bit
			bHasWallJumped = true;

			// schedule wall jump lockout reset
			GetWorld()->GetTimerManager().SetTimer(WallJumpTimer, this, &ASideScrollingCharacter::ResetWallJump, DelayBetweenWallJumps, false);

			return;
		}
	}



	// test for double jump only if we haven't already tested for wall jump
	if (!bHasWallJumped)
	{
		// are we still within coyote time frames?
		if (GetWorld()->GetTimeSeconds() - LastFallTime < MaxCoyoteTime)
		{
			UE_LOG(LogTemp, Warning, TEXT("Coyote Jump"));

			// use the built-in CMC functionality to do the jump
			Jump();

		// no coyote time jump
		} else {
		
			// The movement component handles double jump but we still need to manage the flag for animation
			if (!bHasDoubleJumped)
			{
				// raise the double jump flag
				bHasDoubleJumped = true;

				const float DoubleJumpZVelocity = GetCharacterMovement()->JumpZVelocity * DoubleJumpVerticalMultiplier;
				LaunchCharacter(FVector(0.0f, 0.0f, DoubleJumpZVelocity), false, true);
			}
		}
	}
}

void ASideScrollingCharacter::CheckForSoftCollision()
{
	// reset the drop value
	DropValue = 0.0f;

	// trace down 
	FHitResult OutHit;

	const FVector Start = GetActorLocation();
	const FVector End = Start + (FVector::DownVector * SoftCollisionTraceDistance);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(SoftCollisionObjectType);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByObjectType(OutHit, Start, End, ObjectParams, QueryParams);

	// did we hit a soft floor?
	if (OutHit.GetActor())
	{
		// drop through the floor
		SetSoftCollision(true);
	}
}

void ASideScrollingCharacter::ResetWallJump()
{
	// reset the wall jump flag
	bHasWallJumped = false;
}

void ASideScrollingCharacter::SetSoftCollision(bool bEnabled)
{
	// enable or disable collision response to the soft collision channel
	GetCapsuleComponent()->SetCollisionResponseToChannel(SoftCollisionObjectType, bEnabled ? ECR_Ignore : ECR_Block);
}

bool ASideScrollingCharacter::HasDoubleJumped() const
{
	return bHasDoubleJumped;
}

bool ASideScrollingCharacter::HasWallJumped() const
{
	return bHasWallJumped;
}
