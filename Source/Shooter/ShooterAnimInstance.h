// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
	EOS_Aiming UMETA(DisplayName="Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_InAir UMETA(DisplayName = "InAir"),

	EOS_Max UMETA(DisplayName = "DefaultMax")
};
/**
 * 
 */
UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
		
public:

	UShooterAnimInstance();

	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	virtual void NativeInitializeAnimation() override;

protected:

	// handle turn in place varoables
	void TurnInPlace();

	// Handle Calculation for leaning while running
	void Lean(float DeltaTime);

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
		class AShooterCharacter* ShooterCharacter;

	//Speed of the Character
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Speed;
	// Whether or not the character in the air
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;
	// Whether or not the character is Moving
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	/** offset  Yaw Used for strafing*/
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw;

	/** Offset Yaw the frame before we moving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	//yaw of the character this fram;
	float TIPCharacterYaw;

	// Yaw at the characeter previous frame
	float TIPCharacterYawLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = " Turn In place", meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	//Rotation CurveValue
	float RotationCurve;

	//Rotation CurveValueLastFrame
	float RotationCurveValueLastFrame;

	//The pitch of the AimRotation, used in AimOffset
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = " Turn In place", meta = (AllowPrivateAccess = "true"))
	float Pitch;

	//True when reloading, used prevent to AimOffset when Reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = " Turn In place", meta = (AllowPrivateAccess = "true"))
	bool bReloading;

	// Offset that used to detemint which animoffst to use
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = " Turn In place", meta = (AllowPrivateAccess = "true"))
	EOffsetState OffsetState;

	//Character Yaw this frame
	FRotator CharacterRotation;

	// Character Yaw Last Frame
	FRotator CharacterRotationLastFrame;

	//YawDelta Used for running to lean in blenspace
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = " Lean", meta = (AllowPrivateAccess = "true"))
	float YawDelta;
};
