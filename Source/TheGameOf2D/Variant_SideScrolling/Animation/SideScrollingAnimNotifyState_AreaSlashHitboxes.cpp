// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotifyState_AreaSlashHitboxes.h"
#include "DrawDebugHelpers.h"
#include "ModengEnemy.h"
#include "SideScrollingCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

void USideScrollingAnimNotifyState_AreaSlashHitboxes::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}

	FSideScrollingAreaSlashRuntimeState& RuntimeState = RuntimeStates.FindOrAdd(MeshComp);
	RuntimeState.ElapsedTime = 0.0f;
	RuntimeState.Duration = TotalDuration;
	RuntimeState.HitEnemies.Reset();
	RuntimeState.HitEnemiesByShape.Reset();

	EvaluateHitboxes(MeshComp, RuntimeState);
}

void USideScrollingAnimNotifyState_AreaSlashHitboxes::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp)
	{
		return;
	}

	FSideScrollingAreaSlashRuntimeState* RuntimeState = RuntimeStates.Find(MeshComp);
	if (!RuntimeState)
	{
		return;
	}

	RuntimeState->ElapsedTime += FMath::Max(FrameDeltaTime, 0.0f);
	EvaluateHitboxes(MeshComp, *RuntimeState);
}

void USideScrollingAnimNotifyState_AreaSlashHitboxes::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		RuntimeStates.Remove(MeshComp);
	}
}

FString USideScrollingAnimNotifyState_AreaSlashHitboxes::GetNotifyName_Implementation() const
{
	return TEXT("Area Slash Hitboxes");
}

void USideScrollingAnimNotifyState_AreaSlashHitboxes::EvaluateHitboxes(USkeletalMeshComponent* MeshComp, FSideScrollingAreaSlashRuntimeState& RuntimeState)
{
	ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr;
	if (!Character || !Character->GetWorld())
	{
		return;
	}

	const float FacingSign = bMirrorWithCharacterFacing && Character->GetSideScrollingFacingSign() < 0.0f ? -1.0f : 1.0f;
	const FTransform ReferenceTransform = BuildReferenceTransform(MeshComp, FacingSign);
	for (int32 HitboxIndex = 0; HitboxIndex < Hitboxes.Num(); ++HitboxIndex)
	{
		const FSideScrollingAreaSlashHitbox& Hitbox = Hitboxes[HitboxIndex];
		if (!IsHitboxActive(Hitbox, RuntimeState))
		{
			continue;
		}

		TArray<AModengEnemy*> HitEnemies;
		bool bHadAnyOverlap = false;
		CollectHitboxEnemies(MeshComp, Hitbox, ReferenceTransform, FacingSign, HitEnemies, bHadAnyOverlap);

		for (AModengEnemy* Enemy : HitEnemies)
		{
			if (!ShouldDamageEnemy(Enemy, HitboxIndex, RuntimeState))
			{
				continue;
			}

			const float DamageAmount = Character->GetCurrentAttackDamage() * Hitbox.DamageMultiplier;
			Enemy->ApplyDamageToEnemy(DamageAmount, Character);
			MarkEnemyDamaged(Enemy, HitboxIndex, RuntimeState);

			if (!Enemy->IsDead() && Hitbox.KnockbackDistance > 0.0f)
			{
				Enemy->AddActorWorldOffset(FVector(FacingSign * Hitbox.KnockbackDistance, 0.0f, 0.0f), false);
			}
		}

		if (bDrawDebug)
		{
			const FTransform HitboxTransform = BuildHitboxTransform(Hitbox, ReferenceTransform, FacingSign);
			DrawHitboxDebug(Character->GetWorld(), Hitbox, HitboxTransform, bHadAnyOverlap);
		}
	}
}

bool USideScrollingAnimNotifyState_AreaSlashHitboxes::IsHitboxActive(const FSideScrollingAreaSlashHitbox& Hitbox, const FSideScrollingAreaSlashRuntimeState& RuntimeState) const
{
	const float StartTime = FMath::Max(Hitbox.StartTime, 0.0f);
	const float EndTime = Hitbox.EndTime > StartTime ? Hitbox.EndTime : RuntimeState.Duration;
	return RuntimeState.ElapsedTime >= StartTime && RuntimeState.ElapsedTime <= EndTime;
}

FTransform USideScrollingAnimNotifyState_AreaSlashHitboxes::BuildReferenceTransform(USkeletalMeshComponent* MeshComp, float FacingSign) const
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner)
	{
		return FTransform::Identity;
	}

	FTransform ReferenceTransform = Owner->GetActorTransform();
	if (!ReferenceBoneOrSocketName.IsNone() && (MeshComp->DoesSocketExist(ReferenceBoneOrSocketName) || MeshComp->GetBoneIndex(ReferenceBoneOrSocketName) != INDEX_NONE))
	{
		ReferenceTransform = MeshComp->GetSocketTransform(ReferenceBoneOrSocketName, RTS_World);
		if (!bUseReferenceRotation)
		{
			ReferenceTransform.SetRotation(Owner->GetActorQuat());
		}
	}

	return ReferenceTransform;
}

FTransform USideScrollingAnimNotifyState_AreaSlashHitboxes::BuildHitboxTransform(const FSideScrollingAreaSlashHitbox& Hitbox, const FTransform& ReferenceTransform, float FacingSign) const
{
	FVector MirroredOffset = Hitbox.LocalOffset;
	MirroredOffset.X *= FacingSign;

	FRotator MirroredRotation = Hitbox.LocalRotation;
	if (FacingSign < 0.0f)
	{
		MirroredRotation.Yaw = 180.0f - MirroredRotation.Yaw;
	}

	return FTransform(MirroredRotation, MirroredOffset) * ReferenceTransform;
}

void USideScrollingAnimNotifyState_AreaSlashHitboxes::CollectHitboxEnemies(USkeletalMeshComponent* MeshComp, const FSideScrollingAreaSlashHitbox& Hitbox, const FTransform& ReferenceTransform, float FacingSign, TArray<AModengEnemy*>& OutEnemies, bool& bOutHadAnyOverlap) const
{
	OutEnemies.Reset();
	bOutHadAnyOverlap = false;

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	UWorld* World = Owner ? Owner->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	FCollisionShape Shape;
	switch (Hitbox.Shape)
	{
	case ESideScrollingAreaSlashShape::Sphere:
		Shape.SetSphere(FMath::Max(Hitbox.SphereRadius, 1.0f));
		break;
	case ESideScrollingAreaSlashShape::Capsule:
		Shape.SetCapsule(FMath::Max(Hitbox.CapsuleRadius, 1.0f), FMath::Max(Hitbox.CapsuleHalfHeight, Hitbox.CapsuleRadius));
		break;
	case ESideScrollingAreaSlashShape::Box:
	default:
		Shape.SetBox(FVector3f(Hitbox.BoxExtent.ComponentMax(FVector(1.0f))));
		break;
	}

	const FTransform HitboxTransform = BuildHitboxTransform(Hitbox, ReferenceTransform, FacingSign);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByObjectType(OverlapResults, HitboxTransform.GetLocation(), HitboxTransform.GetRotation(), ObjectParams, Shape, QueryParams);
	bOutHadAnyOverlap = OverlapResults.Num() > 0;

	TSet<TObjectKey<AModengEnemy>> UniqueEnemies;
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AModengEnemy* Enemy = Cast<AModengEnemy>(OverlapResult.GetActor());
		if (!Enemy || Enemy->IsDead())
		{
			continue;
		}

		const TObjectKey<AModengEnemy> EnemyKey(Enemy);
		if (UniqueEnemies.Contains(EnemyKey))
		{
			continue;
		}

		UniqueEnemies.Add(EnemyKey);
		OutEnemies.Add(Enemy);
	}
}

bool USideScrollingAnimNotifyState_AreaSlashHitboxes::ShouldDamageEnemy(AModengEnemy* Enemy, int32 HitboxIndex, FSideScrollingAreaSlashRuntimeState& RuntimeState) const
{
	if (!Enemy || Enemy->IsDead())
	{
		return false;
	}

	const TObjectKey<AModengEnemy> EnemyKey(Enemy);
	switch (HitPolicy)
	{
	case ESideScrollingAreaSlashHitPolicy::EveryTick:
		return true;
	case ESideScrollingAreaSlashHitPolicy::OncePerShape:
		return !RuntimeState.HitEnemiesByShape.FindOrAdd(HitboxIndex).Contains(EnemyKey);
	case ESideScrollingAreaSlashHitPolicy::OncePerNotify:
	default:
		return !RuntimeState.HitEnemies.Contains(EnemyKey);
	}
}

void USideScrollingAnimNotifyState_AreaSlashHitboxes::MarkEnemyDamaged(AModengEnemy* Enemy, int32 HitboxIndex, FSideScrollingAreaSlashRuntimeState& RuntimeState)
{
	if (!Enemy)
	{
		return;
	}

	const TObjectKey<AModengEnemy> EnemyKey(Enemy);
	RuntimeState.HitEnemies.Add(EnemyKey);
	RuntimeState.HitEnemiesByShape.FindOrAdd(HitboxIndex).Add(EnemyKey);
}

void USideScrollingAnimNotifyState_AreaSlashHitboxes::DrawHitboxDebug(UWorld* World, const FSideScrollingAreaSlashHitbox& Hitbox, const FTransform& HitboxTransform, bool bHit) const
{
	if (!World)
	{
		return;
	}

	const FColor DrawColor = bHit ? DebugHitColor : DebugColor;
	switch (Hitbox.Shape)
	{
	case ESideScrollingAreaSlashShape::Sphere:
		DrawDebugSphere(World, HitboxTransform.GetLocation(), FMath::Max(Hitbox.SphereRadius, 1.0f), 24, DrawColor, false, DebugDrawDuration, 0, DebugLineThickness);
		break;
	case ESideScrollingAreaSlashShape::Capsule:
		DrawDebugCapsule(World, HitboxTransform.GetLocation(), FMath::Max(Hitbox.CapsuleHalfHeight, Hitbox.CapsuleRadius), FMath::Max(Hitbox.CapsuleRadius, 1.0f), HitboxTransform.GetRotation(), DrawColor, false, DebugDrawDuration, 0, DebugLineThickness);
		break;
	case ESideScrollingAreaSlashShape::Box:
	default:
		DrawDebugBox(World, HitboxTransform.GetLocation(), Hitbox.BoxExtent.ComponentMax(FVector(1.0f)), HitboxTransform.GetRotation(), DrawColor, false, DebugDrawDuration, 0, DebugLineThickness);
		break;
	}
}
