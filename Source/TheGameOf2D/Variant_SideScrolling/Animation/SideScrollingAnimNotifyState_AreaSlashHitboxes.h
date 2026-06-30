// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SideScrollingAnimNotifyState_AreaSlashHitboxes.generated.h"

class AModengEnemy;

UENUM(BlueprintType)
enum class ESideScrollingAreaSlashShape : uint8
{
	Box,
	Sphere,
	Capsule
};

UENUM(BlueprintType)
enum class ESideScrollingAreaSlashHitPolicy : uint8
{
	OncePerNotify,
	OncePerShape,
	EveryTick
};

USTRUCT(BlueprintType)
struct FSideScrollingAreaSlashHitbox
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hitbox")
	ESideScrollingAreaSlashShape Shape = ESideScrollingAreaSlashShape::Box;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Timing", meta=(ClampMin="0.0", Units="s"))
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Timing", meta=(ClampMin="0.0", Units="s"))
	float EndTime = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transform")
	FVector LocalOffset = FVector(160.0f, 0.0f, 80.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transform")
	FRotator LocalRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shape", meta=(EditCondition="Shape == ESideScrollingAreaSlashShape::Box", EditConditionHides))
	FVector BoxExtent = FVector(180.0f, 80.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shape", meta=(EditCondition="Shape == ESideScrollingAreaSlashShape::Sphere", EditConditionHides, ClampMin="1.0"))
	float SphereRadius = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shape", meta=(EditCondition="Shape == ESideScrollingAreaSlashShape::Capsule", EditConditionHides, ClampMin="1.0"))
	float CapsuleRadius = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shape", meta=(EditCondition="Shape == ESideScrollingAreaSlashShape::Capsule", EditConditionHides, ClampMin="1.0"))
	float CapsuleHalfHeight = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage", meta=(ClampMin="0.0"))
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage", meta=(ClampMin="0.0"))
	float KnockbackDistance = 30.0f;
};

struct FSideScrollingAreaSlashRuntimeState
{
	float ElapsedTime = 0.0f;
	float Duration = 0.0f;
	TSet<TObjectKey<AModengEnemy>> HitEnemies;
	TMap<int32, TSet<TObjectKey<AModengEnemy>>> HitEnemiesByShape;
};

UCLASS(meta=(DisplayName="Area Slash Hitboxes"))
class USideScrollingAnimNotifyState_AreaSlashHitboxes : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Area Slash")
	TArray<FSideScrollingAreaSlashHitbox> Hitboxes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Area Slash")
	ESideScrollingAreaSlashHitPolicy HitPolicy = ESideScrollingAreaSlashHitPolicy::OncePerNotify;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Area Slash")
	FName ReferenceBoneOrSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Area Slash")
	bool bUseReferenceRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Area Slash")
	bool bMirrorWithCharacterFacing = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug", meta=(ClampMin="0.0", Units="s"))
	float DebugDrawDuration = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
	FColor DebugColor = FColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
	FColor DebugHitColor = FColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug", meta=(ClampMin="0.0"))
	float DebugLineThickness = 2.0f;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

private:
	TMap<TObjectKey<USkeletalMeshComponent>, FSideScrollingAreaSlashRuntimeState> RuntimeStates;

	void EvaluateHitboxes(USkeletalMeshComponent* MeshComp, FSideScrollingAreaSlashRuntimeState& RuntimeState);
	bool IsHitboxActive(const FSideScrollingAreaSlashHitbox& Hitbox, const FSideScrollingAreaSlashRuntimeState& RuntimeState) const;
	FTransform BuildReferenceTransform(USkeletalMeshComponent* MeshComp, float FacingSign) const;
	FTransform BuildHitboxTransform(const FSideScrollingAreaSlashHitbox& Hitbox, const FTransform& ReferenceTransform, float FacingSign) const;
	void CollectHitboxEnemies(USkeletalMeshComponent* MeshComp, const FSideScrollingAreaSlashHitbox& Hitbox, const FTransform& ReferenceTransform, float FacingSign, TArray<AModengEnemy*>& OutEnemies, bool& bOutHadAnyOverlap) const;
	bool ShouldDamageEnemy(AModengEnemy* Enemy, int32 HitboxIndex, FSideScrollingAreaSlashRuntimeState& RuntimeState) const;
	void MarkEnemyDamaged(AModengEnemy* Enemy, int32 HitboxIndex, FSideScrollingAreaSlashRuntimeState& RuntimeState);
	void DrawHitboxDebug(UWorld* World, const FSideScrollingAreaSlashHitbox& Hitbox, const FTransform& HitboxTransform, bool bHit) const;
};
