#include "ModengPotionPickup.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Variant_SideScrolling/SideScrollingCharacter.h"

AModengPotionPickup::AModengPotionPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	SetRootComponent(PickupSphere);
	PickupSphere->SetSphereRadius(PickupRadius);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionObjectType(ECC_WorldDynamic);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	PotionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PotionMesh"));
	PotionMesh->SetupAttachment(PickupSphere);
	PotionMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PotionMesh->SetGenerateOverlapEvents(false);
	PotionMesh->SetCastShadow(true);
	PotionMesh->SetRelativeScale3D(PotionMeshScale);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PotionMeshAsset(TEXT("/Game/Fab/4_Colour_Alchemist_Sphere_Like_Potions/4_colour_alchemist_sphere_like_potions/StaticMeshes/4_colour_alchemist_sphere_like_potions.4_colour_alchemist_sphere_like_potions"));
	if (PotionMeshAsset.Succeeded())
	{
		PotionStaticMesh = PotionMeshAsset.Object;
		PotionMesh->SetStaticMesh(PotionStaticMesh);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> HealthMaterial(TEXT("/Game/Fab/4_Colour_Alchemist_Sphere_Like_Potions/4_colour_alchemist_sphere_like_potions/Materials/material.material"));
	if (HealthMaterial.Succeeded())
	{
		HealthPotionMaterial = HealthMaterial.Object;
		PotionMesh->SetMaterial(0, HealthPotionMaterial);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ManaMaterial(TEXT("/Game/Fab/4_Colour_Alchemist_Sphere_Like_Potions/4_colour_alchemist_sphere_like_potions/Materials/blue.blue"));
	if (ManaMaterial.Succeeded())
	{
		ManaPotionMaterial = ManaMaterial.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> HiddenMaterial(TEXT("/Engine/Engine_MI_Shaders/Instances/M_Shader_SimpleTranslucent_Invis.M_Shader_SimpleTranslucent_Invis"));
	if (HiddenMaterial.Succeeded())
	{
		HiddenPotionMaterial = HiddenMaterial.Object;
	}

	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AModengPotionPickup::HandlePickupOverlap);
}

void AModengPotionPickup::BeginPlay()
{
	Super::BeginPlay();

	PickupSphere->SetSphereRadius(PickupRadius);
	RefreshVisuals();
	InitialVisualRelativeLocation = PotionMesh ? PotionMesh->GetRelativeLocation() : FVector::ZeroVector;

	if (Lifetime > 0.0f)
	{
		SetLifeSpan(Lifetime);
	}
}

void AModengPotionPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	LifeTimeSeconds += DeltaSeconds;
	if (!PotionMesh)
	{
		return;
	}

	const float BobOffset = BobHeight > 0.0f ? FMath::Sin(LifeTimeSeconds * BobSpeed) * BobHeight : 0.0f;
	PotionMesh->SetRelativeLocation(InitialVisualRelativeLocation + FVector(0.0f, 0.0f, BobOffset));
}

void AModengPotionPickup::InitializePotion(EModengPotionType InPotionType, float InRestoreAmount)
{
	PotionType = InPotionType;
	RestoreAmount = FMath::Max(0.0f, InRestoreAmount);
	RefreshVisuals();
}

void AModengPotionPickup::HandlePickupOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bPickedUp)
	{
		return;
	}

	ASideScrollingCharacter* PlayerCharacter = Cast<ASideScrollingCharacter>(OtherActor);
	if (!PlayerCharacter || PlayerCharacter->IsPlayerDefeated())
	{
		return;
	}

	const float RestoredAmount = PotionType == EModengPotionType::Health
		? PlayerCharacter->RestoreHealth(RestoreAmount)
		: PlayerCharacter->RestoreMana(RestoreAmount);

	if (RestoredAmount <= UE_KINDA_SMALL_NUMBER)
	{
		return;
	}

	bPickedUp = true;
	ReceivePickedUp(PlayerCharacter);
	Destroy();
}

void AModengPotionPickup::RefreshVisuals()
{
	if (!PotionMesh)
	{
		return;
	}

	if (PotionStaticMesh && PotionMesh->GetStaticMesh() != PotionStaticMesh)
	{
		PotionMesh->SetStaticMesh(PotionStaticMesh);
	}

	ApplyPotionMaterials();
	PotionMesh->SetRelativeScale3D(PotionMeshScale);
	AlignMeshToGround();
	InitialVisualRelativeLocation = PotionMesh->GetRelativeLocation();
}

void AModengPotionPickup::ApplyPotionMaterials()
{
	if (!PotionMesh)
	{
		return;
	}

	UMaterialInterface* VisibleMaterial = GetPotionMaterial();
	if (!VisibleMaterial)
	{
		return;
	}

	const int32 VisibleMaterialIndex = GetVisiblePotionMaterialIndex();
	if (!HiddenPotionMaterial)
	{
		PotionMesh->SetMaterial(VisibleMaterialIndex, VisibleMaterial);
		return;
	}

	const int32 MaterialCount = FMath::Max(1, PotionMesh->GetNumMaterials());
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		UMaterialInterface* MaterialToApply = MaterialIndex == VisibleMaterialIndex
			? VisibleMaterial
			: HiddenPotionMaterial.Get();
		PotionMesh->SetMaterial(MaterialIndex, MaterialToApply);
	}
}

void AModengPotionPickup::AlignMeshToGround()
{
	if (!PotionMesh)
	{
		return;
	}

	PotionMesh->SetRelativeLocation(FVector::ZeroVector);
	const FBoxSphereBounds LocalBounds = PotionMesh->CalcLocalBounds();
	const float MeshBottomOffset = (LocalBounds.Origin.Z - LocalBounds.BoxExtent.Z) * PotionMeshScale.Z;
	const float GroundOffset = -MeshBottomOffset + PotionVisualGroundOffset;
	PotionMesh->SetRelativeLocation(FVector(0.0f, 0.0f, GroundOffset));
}

UMaterialInterface* AModengPotionPickup::GetPotionMaterial() const
{
	return PotionType == EModengPotionType::Health ? HealthPotionMaterial : ManaPotionMaterial;
}

int32 AModengPotionPickup::GetVisiblePotionMaterialIndex() const
{
	const UStaticMesh* StaticMesh = PotionMesh ? PotionMesh->GetStaticMesh() : nullptr;
	if (!StaticMesh)
	{
		return 0;
	}

	const FName VisibleSlotName = GetVisiblePotionMaterialSlotName();
	int32 VisibleMaterialIndex = StaticMesh->GetMaterialIndex(VisibleSlotName);
	if (VisibleMaterialIndex == INDEX_NONE)
	{
		VisibleMaterialIndex = StaticMesh->GetMaterialIndexFromImportedMaterialSlotName(VisibleSlotName);
	}

	if (VisibleMaterialIndex != INDEX_NONE)
	{
		return VisibleMaterialIndex;
	}

	UMaterialInterface* VisibleMaterial = GetPotionMaterial();
	const TArray<FStaticMaterial>& StaticMaterials = StaticMesh->GetStaticMaterials();
	for (int32 MaterialIndex = 0; MaterialIndex < StaticMaterials.Num(); ++MaterialIndex)
	{
		if (StaticMaterials[MaterialIndex].MaterialInterface == VisibleMaterial)
		{
			return MaterialIndex;
		}
	}

	return 0;
}

FName AModengPotionPickup::GetVisiblePotionMaterialSlotName() const
{
	return PotionType == EModengPotionType::Health ? TEXT("material") : TEXT("blue");
}
