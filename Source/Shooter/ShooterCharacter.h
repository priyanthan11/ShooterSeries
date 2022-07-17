  // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_9mm UMETA(DisplayName = "9mm"),
	EAT_AR	UMETA(DisplayName = "AR"),

	EAT_Max UMETA(DisplayName = "DefaultMax")
};


UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called Forward/Backward input
	void MoveForward(float Value);
	// Called side to side input
	void MoveRight(float Value);
	// Actual implementation of moveforward/ backwards
	void MovementInput(bool bIsforwardAxis, float Value);

	/**
	* Called via inpt turn at rate at a given rate.
	* @param Rate this is a normalized rate, i.e. 1.0 means 100% of desried turn rate
	**/
	void TurnAtRate(float Rate);
	/**
	* called via input look up/down at given rate
	* @param Rate this is a normalized rate, i.e. 1.0 means 100% of desried turn rate
	**/
	void LookUpRate(float Rate);

	/**
	* Rotate Controller Based on mouse X Movement
	* @param Value the input Value from mouse movement
	**/
	void Turn(float Value);
	/**
	* Rotate Controller Based on mouse Y Movement
	* @param Value the input Value from mouse movement
	**/
	void LookUp(float Value);

	//Called when firebutton pressed or called
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);
	/** Set bAiming true or flase with button pressed**/
	void AimingButtonPressed();
	void AimingButtonReleased();

	/* Set Zooming Shoothly when aiming*/
	void CameraInterpZoom(float DeltaTime);

	/*Set BaseTurnRate and BaseLookUpRate Based on Aiming*/
	void SetLookRates();


	void CalculateCrosshairSpread(float DeltaTime);

	void StartCrosshairBulletFire();

	UFUNCTION()
	void FinishCrosshairBulletFire();


	void FireButtonPressed();

	void FireButtonReleased();

	void StartFireTimer();

	UFUNCTION()
	void AutoFireReset();

	// Line trace for items under the crosshair
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);

	/* Trace For Items if OverLappedItemCount > 0*/
	void TraceForItems();

	/*Spawn a default weapon and equips it*/
	class AWeapon* SpawnDefaultWeapon();

	/*Takes a weapon and attaches it the mesh*/
	void EquipWeapon( AWeapon* WeaponToEquipped);

	/*Detach weapon from mesh and let it fall to the ground*/
	void DropWeapon();

	void SelectButtonPressed();
	void SelectButtonReleased();

	// Drop currently equiped weapon and equipe TraceHitItems
	void SwapWeapon(AWeapon* WeaponToSwap);

	// Initialize ammoMap with ammo Values
	void InitializeAmmoMap();



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	// CameraBoom Positioning Camera Behind the Charactor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	// 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	// Base trun rate, in deg/sec. other scaling may affect final turn rate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;
	// Base Look up/down rate, in deg/sec. other scaling may affect final rate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;
	  
	/* Turn rate while not aiming*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;
	/*Look up rate when not aiming*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	/* Turn rate when aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;
	/* lookup rate when aiming*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	/* Scale factor for mouse look sensitivity. Lookup rate when not aiming*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;
	/* Scale factor for mouse look sensitivity. Lookup rate when not aiming*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookupRate;
	/* Scale factor for mouse look sensitivity. Turn rate when not aiming*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;
	/* Scale factor for mouse look sensitivity. Turn rate when  aiming*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;


	// Randomize fire sound cue
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;
	//  Flash spawned at muzzel location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;
	/* Montage for firing weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;
	/*Particle Spawn on bullet impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;
	/* Smoke Trail For Bullets*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;
	/** True when aiming **/
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bAiming;
	 /* Default camera FOV value*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float CameraDefaultFOV;

	/** FOV Value when zoomed **/
	float CameraZoomedFOV;

	/*Current FOV this frame*/
	float CameraCurrentFOV;
	/*InterpSpeed for zooming when aiming*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	/*Determines the spread of the crosshairs */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshair", meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;
	/* Velocity component for crosshair spread*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshair", meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	/* InAir component for crosshair spread*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshair", meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	/* Aim component for crosshair spread*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshair", meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	/* Sooting component for crosshair spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshair", meta = (AllowPrivateAccess = "true"))
	float CorsshairShootingFactor;


	float ShootTimeDuration;

	bool bFiringBullet;

	FTimerHandle CrosshairShootTimer;

	/** Left Mouse button or console trigger pressed*/
	bool bFireButtonPressed;

	/** True when we can fire false when waiting for the timer **/
	bool bShouldFire;

	/** Rate of automatioc Gun fire**/
	float AutomaticFireRate;

	/* Set timer between gun shots*/
	FTimerHandle AutoFireTimer;

	/* True we should trace for iteam */
	bool bShouldTraceForItems;
	/* Number of overlaped AItems */
	int8 OverlappedItemCount;

	/** The AItem We hit Last Frame **/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	class AItem* TraceHitItemLastFrame;

	/*Currently Equiped Weapon*/
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class AWeapon* EquippedWeapon;

	/*Set this in blueprints for the default weapon class*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon>DefaultWeaponClass;

	/*The item currently hitby out trace in trace for items (could be null)*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "combat", meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItem;

	//Distance outward from the camera for interp destance
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	float CameraInterpDistance;

	// Distance upward from the camerafor the interp destination
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	float CameraInterpElevation;

	// Map to keep track of ammo of the different ammo type
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshair", meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32>AmmoMap;

	//Starting 9mm ammo amount
	int32 Starting9mmAmmo;
	// Starting AR amount Ammo
	int32 StartingARAmmo;






	
public:
	// Returns camera boom subobject
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	//Returns FollowCamera Subobject
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	FORCEINLINE bool GetAiming() const { return bAiming; }

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;

	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }
	/** Adds / Subtracts to/from overlappeditem and updates bShouldTrace For Items **/
	void IncrementOverlappedItemCount(int8 Amount);

	FVector GetCameraInterpLocation();

	void GetPickupItem(AItem* Item);

};
