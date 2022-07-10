// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
		
public:
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	virtual void NativeInitializeAnimation() override;

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
};
