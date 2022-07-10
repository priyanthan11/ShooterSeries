// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

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

	//Called when firebutton pressed or called
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);


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
public:
	// Returns camera boom subobject
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	//Returns FollowCamera Subobject
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

};
