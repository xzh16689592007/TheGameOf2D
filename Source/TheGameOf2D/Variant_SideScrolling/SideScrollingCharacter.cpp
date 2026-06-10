// Copyright Epic Games, Inc. All Rights Reserved.


#include "SideScrollingCharacter.h"
#include "Animation/AnimInstance.h"
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
#include "Components/SceneComponent.h"
#include "ModengEnemy.h"
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

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> UnarmedAttackAnimation(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01.MM_Attack_01"));
	if (UnarmedAttackAnimation.Succeeded())
	{
		AttackAnimation = UnarmedAttackAnimation.Object;
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

	// enable double jump and coyote time
	JumpMaxCount = 3;
}

void ASideScrollingCharacter::BeginPlay()
{
	Super::BeginPlay();

	ConfigurePlayerVisuals();
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

		// Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ASideScrollingCharacter::DoInteract);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASideScrollingCharacter::Move);

		// Dropping from platform
		EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Triggered, this, &ASideScrollingCharacter::Drop);
		EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Completed, this, &ASideScrollingCharacter::DropReleased);

	}

	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &ASideScrollingCharacter::DoInteract);
	PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &ASideScrollingCharacter::DoInteract);
	PlayerInputComponent->BindKey(EKeys::J, IE_Pressed, this, &ASideScrollingCharacter::DoAttack);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ASideScrollingCharacter::DoAttack);
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
	// is movement temporarily disabled after wall jumping?
	if (!bHasWallJumped)
	{
		// save the movement values
		ActionValueY = Forward;
		if (!FMath::IsNearlyZero(Forward))
		{
			LastFacingX = Forward > 0.0f ? 1.0f : -1.0f;
			UpdateFacingDirection(LastFacingX);
			if (bMovementInterruptsAttackAnimation)
			{
				InterruptAttackAnimation();
			}
		}

		// figure out the movement direction
		const FVector MoveDir = FVector(1.0f, Forward > 0.0f ? 0.1f : -0.1f, 0.0f);

		// apply the movement input
		AddMovementInput(MoveDir, Forward);
	}
}

void ASideScrollingCharacter::DoDrop(float Value)
{
	// save the movement value
	DropValue = Value;
}

void ASideScrollingCharacter::DoJumpStart()
{
	// handle advanced jump behaviors
	MultiJump();
}

void ASideScrollingCharacter::DoJumpEnd()
{
	StopJumping();
}

void ASideScrollingCharacter::DoInteract()
{
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

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("No interact target"));
	}
}

void ASideScrollingCharacter::DoAttack()
{
	if (bBlockRepeatedAttacksUntilAnimationEnds && bAttackAnimationInProgress)
	{
		return;
	}

	const float FacingSign = LastFacingX >= 0.0f ? 1.0f : -1.0f;
	PendingAttackFacingSign = FacingSign;
	bAttackHitPending = true;
	bAttackHitWindowActive = false;
	bHasPreviousWeaponTrace = false;
	bAttackRegisteredHit = false;
	HitEnemiesThisAttack.Empty();

	const float AttackDuration = PlayAttackAnimation();
	GetWorld()->GetTimerManager().ClearTimer(AttackHitTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackHitWindowStartTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackHitWindowEndTimer);

	if (bUseWeaponTraceForAttack && AttackDuration > 0.0f)
	{
		const float WindowStartRatio = FMath::Clamp(AttackHitWindowStartRatio, 0.0f, 1.0f);
		const float WindowEndRatio = FMath::Clamp(FMath::Max(AttackHitWindowEndRatio, WindowStartRatio), 0.0f, 1.0f);
		const float WindowStartDelay = AttackDuration * WindowStartRatio;
		float WindowEndDelay = AttackDuration * WindowEndRatio;
		if (WindowEndDelay <= WindowStartDelay)
		{
			WindowEndDelay = FMath::Min(AttackDuration, WindowStartDelay + 0.05f);
		}

		if (WindowStartDelay > 0.0f)
		{
			GetWorld()->GetTimerManager().SetTimer(AttackHitWindowStartTimer, this, &ASideScrollingCharacter::BeginAttackHitWindow, WindowStartDelay, false);
		}
		else
		{
			BeginAttackHitWindow();
		}

		GetWorld()->GetTimerManager().SetTimer(AttackHitWindowEndTimer, this, &ASideScrollingCharacter::EndAttackHitWindow, WindowEndDelay, false);
	}
	else
	{
		const float HitDelay = AttackDuration > 0.0f ? AttackDuration * FMath::Clamp(AttackHitTimeRatio, 0.0f, 1.0f) : 0.0f;
		if (HitDelay > 0.0f)
		{
			GetWorld()->GetTimerManager().SetTimer(AttackHitTimer, this, &ASideScrollingCharacter::ApplyPendingAttackHit, HitDelay, false);
		}
		else
		{
			ApplyPendingAttackHit();
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
	const float FacingSign = PendingAttackFacingSign >= 0.0f ? 1.0f : -1.0f;

	bool bTraceAttempted = false;
	if (bUseWeaponTraceForAttack && ApplyWeaponTraceAttackHit(FacingSign, bTraceAttempted))
	{
		return;
	}

	if (!bTraceAttempted && ApplyFallbackBoxAttackHit(FacingSign))
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

	bAttackHitWindowActive = true;
	bHasPreviousWeaponTrace = false;
	UpdateAttackHitWindow();
}

void ASideScrollingCharacter::EndAttackHitWindow()
{
	if (!bAttackHitWindowActive && !bAttackHitPending)
	{
		return;
	}

	bAttackHitWindowActive = false;
	bHasPreviousWeaponTrace = false;

	if (bAttackHitPending)
	{
		bAttackHitPending = false;
		if (!bAttackRegisteredHit && bShowGameplayDebugMessages && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Silver, TEXT("Attack missed"));
		}
	}
}

void ASideScrollingCharacter::UpdateAttackHitWindow()
{
	if (!bAttackHitPending || !bAttackHitWindowActive)
	{
		return;
	}

	bool bTraceAttempted = false;
	const float FacingSign = PendingAttackFacingSign >= 0.0f ? 1.0f : -1.0f;
	const bool bHit = bUseWeaponTraceForAttack && ApplyWeaponTraceAttackHit(FacingSign, bTraceAttempted);
	if (bHit)
	{
		bAttackRegisteredHit = true;
	}

	if (!bTraceAttempted)
	{
		bAttackHitWindowActive = false;
		bHasPreviousWeaponTrace = false;
		ApplyPendingAttackHit();
	}
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

bool ASideScrollingCharacter::ApplyWeaponTraceAttackHit(float FacingSign, bool& bOutTraceAttempted)
{
	bOutTraceAttempted = false;

	USceneComponent* TraceStartComponent = FindSceneComponentByName(WeaponTraceStartComponentName);
	USceneComponent* TraceEndComponent = FindSceneComponentByName(WeaponTraceEndComponentName);
	if (!TraceStartComponent || !TraceEndComponent)
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

	FCollisionShape TraceShape;
	TraceShape.SetSphere(WeaponTraceRadius);

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
			if (!Enemy || Enemy->IsDead() || HitEnemiesThisAttack.Contains(Enemy))
			{
				continue;
			}

			const float DistanceSq = FVector::DistSquared(DistanceFrom, Enemy->GetActorLocation());
			if (DistanceSq < ClosestDistanceSq)
			{
				ClosestDistanceSq = DistanceSq;
				ClosestEnemy = Enemy;
			}
		}
	};

	auto SweepWeaponSegment = [&](const FVector& Start, const FVector& End)
	{
		TArray<FHitResult> HitResults;
		GetWorld()->SweepMultiByObjectType(HitResults, Start, End, FQuat::Identity, ObjectParams, TraceShape, QueryParams);
		CollectClosestEnemy(HitResults, Start);
	};

	SweepWeaponSegment(TraceStart, TraceEnd);
	if (bHasPreviousWeaponTrace)
	{
		SweepWeaponSegment(PreviousWeaponTraceStart, TraceStart);
		SweepWeaponSegment(PreviousWeaponTraceEnd, TraceEnd);
	}

	PreviousWeaponTraceStart = TraceStart;
	PreviousWeaponTraceEnd = TraceEnd;
	bHasPreviousWeaponTrace = true;

	if (!ClosestEnemy)
	{
		return false;
	}

	HitEnemiesThisAttack.Add(ClosestEnemy);
	DamageEnemyFromAttack(ClosestEnemy, FacingSign);
	return true;
}

bool ASideScrollingCharacter::ApplyFallbackBoxAttackHit(float FacingSign)
{
	const float CurrentRange = GetCurrentAttackRange();
	const float CurrentRadius = GetCurrentAttackRadius();
	const FVector AttackCenter = GetActorLocation() + FVector(FacingSign * CurrentRange * 0.5f, 0.0f, 35.0f);

	FCollisionShape AttackBox;
	AttackBox.SetBox(FVector3f(CurrentRange * 0.5f, CurrentRadius, CurrentRadius));

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	TArray<FOverlapResult> OverlapResults;
	GetWorld()->OverlapMultiByObjectType(OverlapResults, AttackCenter, FQuat::Identity, ObjectParams, AttackBox, QueryParams);

	AModengEnemy* ClosestEnemy = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max();
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AModengEnemy* Enemy = Cast<AModengEnemy>(OverlapResult.GetActor());
		if (!Enemy || Enemy->IsDead())
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

	DamageEnemyFromAttack(ClosestEnemy, FacingSign);
	return true;
}

void ASideScrollingCharacter::DamageEnemyFromAttack(AModengEnemy* Enemy, float FacingSign)
{
	if (!Enemy || Enemy->IsDead())
	{
		return;
	}

	Enemy->ApplyDamageToEnemy(GetCurrentAttackDamage(), this);
	if (!Enemy->IsDead())
	{
		Enemy->AddActorWorldOffset(FVector(FacingSign * AttackKnockbackDistance, 0.0f, 0.0f), false);
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

float ASideScrollingCharacter::PlayAttackAnimation()
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!bPlayAttackAnimation || !AttackAnimation || !CharacterMesh)
	{
		return 0.0f;
	}

	GetWorld()->GetTimerManager().ClearTimer(AttackAnimationTimer);

	const float SafePlayRate = FMath::Max(AttackAnimationPlayRate, 0.1f);
	MeshTransformBeforeAttackAnimation = CharacterMesh->GetRelativeTransform();
	bAttackAnimationInProgress = true;
	CharacterMesh->PlayAnimation(AttackAnimation, false);
	CharacterMesh->GlobalAnimRateScale = SafePlayRate;

	const float Duration = AttackAnimation->GetPlayLength() / SafePlayRate;
	if (Duration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(AttackAnimationTimer, this, &ASideScrollingCharacter::FinishAttackAnimation, Duration, false);
	}

	return Duration;
}

void ASideScrollingCharacter::FinishAttackAnimation()
{
	if (bAttackHitWindowActive)
	{
		EndAttackHitWindow();
	}
	else if (bAttackHitPending)
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackHitTimer);
		ApplyPendingAttackHit();
	}

	RestorePlayerAnimationBlueprint();
}

void ASideScrollingCharacter::RestorePlayerAnimationBlueprint()
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh)
	{
		return;
	}

	CharacterMesh->GlobalAnimRateScale = 1.0f;
	if (bRestoreMeshTransformAfterAttackAnimation)
	{
		CharacterMesh->SetRelativeTransform(MeshTransformBeforeAttackAnimation);
	}

	bAttackAnimationInProgress = false;
	bAttackHitPending = false;

	if (PlayerAnimClass)
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
	bAttackHitWindowActive = false;
	bHasPreviousWeaponTrace = false;
	HitEnemiesThisAttack.Empty();
	bAttackHitPending = false;
	RestorePlayerAnimationBlueprint();
}

void ASideScrollingCharacter::UpdateFacingDirection(float FacingSign)
{
	if (!bLockFacingToSideScrollingAxis)
	{
		return;
	}

	SetActorRotation(FRotator(0.0f, FacingSign >= 0.0f ? FacingYawRight : FacingYawLeft, 0.0f));
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

				// let the CMC handle jump
				Jump();
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
