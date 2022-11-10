 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Engine/DataTable.h"
#include "WeaponType.h"
#include "Weapon.generated.h"

USTRUCT(BlueprintType)
struct FWeaponDataTable :public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EAmmoType AmmoType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 WeaponAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MagazineCapacity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class USoundCue* PickupSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundCue* EquipSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USkeletalMesh* ItemMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString ItemName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* InventoryIcon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* AmmoIcon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterialInstance* MaterialInstance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MaterialIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ClipBoneName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ReloadMontageSection;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UAnimInstance>AnimBP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* CrosshairsMiddle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* CrosshairsLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* CrosshairsRight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* CrosshairsBottom;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* CrosshairTop;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AutoFireRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UParticleSystem* MuzzleFlash;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundCue* FireSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName BoneToHide;
};
/**
 * 
 */
UCLASS()
class SHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();

	virtual void Tick(float DeltaTime) override;

	void ThrowWeapon();


protected:
		
	void StopFalling();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	
	
private:

	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;
	
	// Ammo Count for this weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;

	//The type of weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	//Type of Ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	//FName for ReloadMontageSection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSection;

	//Maximum Ammo that our weapon hold
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 MagazineCapacity;

	// True when Clip Moving while Reloading
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	bool bMovingClip;

	// Name of clip bone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ClipBoneName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UDataTable* WeaponDataTable;

	int32 PreviousMaterialIndex;

	/*Textures for weapon Crosshairs*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
		UTexture2D* CrosshairsMiddle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
		UTexture2D* CrosshairsLeft;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
		UTexture2D* CrosshairsRight;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
		UTexture2D* CrosshairsBottom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
		UTexture2D* CrosshairTop;
	/* The speed at which automatic fire happens*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
		float AutoFireRate;/* Particle system spawnd at the barrel*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
		UParticleSystem* MuzzleFlash;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
		USoundCue* FireSound;
	/*Name of the bone in the weapon to hide*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
		FName BoneToHide;

	/*Amount that slide push back while firing*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pistol", meta = (AllowPrivateAccess = "true"))
		float SlideDisplacement;
	/*Curve for the slide displacement*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol", meta = (AllowPrivateAccess = "true"))
		UCurveFloat* SlideDisplacementCurve;
public:

	FORCEINLINE int32 GetAmmo() const { return Ammo; }

	//Called From Character class when FiringWeapon
	void DecrementAmmo();
	void ReloadAmmo(int32 Ammount);
	bool ClipIsFull();

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
	FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSection; }
	FORCEINLINE int32 GetMagazineCapasity() const { return MagazineCapacity; }
	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }
	FORCEINLINE float GetAutoFireRate() const { return AutoFireRate; }
	FORCEINLINE UParticleSystem* GetMuzzleFlash() const { return MuzzleFlash; }
	FORCEINLINE USoundCue* GetFireSound() const { return FireSound; }

	FORCEINLINE void SetMovingClip(bool Move) { bMovingClip = Move; }
	FORCEINLINE void SetBoneClipName(FName Name) { ClipBoneName = Name; }
	FORCEINLINE void SetReloadMontageSection(FName Name) { ReloadMontageSection = Name; }


	

};
