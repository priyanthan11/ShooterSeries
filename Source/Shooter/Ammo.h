// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Ammo.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AAmmo : public AItem
{
	GENERATED_BODY()
public:
	AAmmo();

	virtual void Tick(float DeltaTime) override;
protected:

	virtual void BeginPlay() override;
	

	/*Override of set item propertise so we can set AmmoMesh Propertise*/
	virtual void SetItemProperties(EItemState State) override;

private:
	//Ammo Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* AmmoMesh;

	// Ammo Type for Ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
		EAmmoType AmmoType;

public:

	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
};
