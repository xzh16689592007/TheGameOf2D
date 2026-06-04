// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SideScrollingCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UStaticMeshComponent;
struct FInputActionValue;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Combat", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* AttackVisual;

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

	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat", meta = (ClampMin = "0.01"))
	float AttackVisualDuration = 0.12f;

	/** Extra damage gained for each weapon level after level 1 */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat", meta = (ClampMin = "0.0"))
	float DamageGainPerWeaponLevel = 10.0f;

	/** Extra attack range gained for each weapon level after level 1 */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat", meta = (ClampMin = "0.0"))
	float RangeGainPerWeaponLevel = 35.0f;

	/** Extra attack radius gained for each weapon level after level 1 */
	UPROPERTY(EditAnywhere, Category="Side Scrolling|Combat", meta = (ClampMin = "0.0"))
	float RadiusGainPerWeaponLevel = 12.0f;

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

	FTimerHandle AttackVisualTimer;

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

public:
	
	/** Constructor */
	ASideScrollingCharacter();

protected:

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

	/** Handles basic attack inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAttack();

	UFUNCTION(BlueprintCallable, Category="Side Scrolling|Progression")
	void AddInk(int32 Amount);

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Progression")
	int32 GetWeaponLevel() const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Progression")
	int32 GetCurrentInk() const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Combat")
	float GetCurrentAttackDamage() const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Combat")
	float GetCurrentAttackRange() const;

	UFUNCTION(BlueprintPure, Category="Side Scrolling|Combat")
	float GetCurrentAttackRadius() const;

protected:

	/** Handles advanced jump logic */
	void MultiJump();

	/** Checks for soft collision with platforms */
	void CheckForSoftCollision();

	/** Resets wall jump lockout. Called from timer after a wall jump */
	void ResetWallJump();

	void TryUpgradeWeapon();
	void ShowAttackVisual(float FacingSign);
	void HideAttackVisual();

public:

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
