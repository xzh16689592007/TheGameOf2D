#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ModengPotionPickup.generated.h"

class ASideScrollingCharacter;
class UMaterialInterface;
class USphereComponent;
class UStaticMesh;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EModengPotionType : uint8
{
	Health,
	Mana
};

UCLASS()
class THEGAMEOF2D_API AModengPotionPickup : public AActor
{
	GENERATED_BODY()

public:
	AModengPotionPickup();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Potion")
	void InitializePotion(EModengPotionType InPotionType, float InRestoreAmount);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Potion")
	TObjectPtr<USphereComponent> PickupSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Potion")
	TObjectPtr<UStaticMeshComponent> PotionMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion")
	EModengPotionType PotionType = EModengPotionType::Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion", meta = (ClampMin = "0.0"))
	float RestoreAmount = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion", meta = (ClampMin = "1.0"))
	float PickupRadius = 48.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion", meta = (ClampMin = "0.0"))
	float Lifetime = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Visual")
	FVector PotionMeshScale = FVector(0.18f, 0.18f, 0.18f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Visual")
	float PotionVisualGroundOffset = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Visual", meta = (ClampMin = "0.0"))
	float BobHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Visual", meta = (ClampMin = "0.0"))
	float BobSpeed = 2.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Visual")
	TObjectPtr<UStaticMesh> PotionStaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Visual")
	TObjectPtr<UMaterialInterface> HealthPotionMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Visual")
	TObjectPtr<UMaterialInterface> ManaPotionMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Visual")
	TObjectPtr<UMaterialInterface> HiddenPotionMaterial = nullptr;

	UFUNCTION()
	void HandlePickupOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent, Category = "Potion", meta = (DisplayName = "On Picked Up"))
	void ReceivePickedUp(ASideScrollingCharacter* PlayerCharacter);

private:
	FVector InitialVisualRelativeLocation = FVector::ZeroVector;
	float LifeTimeSeconds = 0.0f;
	bool bPickedUp = false;

	void RefreshVisuals();
	void ApplyPotionMaterials();
	void AlignMeshToGround();
	UMaterialInterface* GetPotionMaterial() const;
	int32 GetVisiblePotionMaterialIndex() const;
	FName GetVisiblePotionMaterialSlotName() const;
};
