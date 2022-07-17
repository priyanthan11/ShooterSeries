// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AShooterPlayerController();


private:
	//Reference to overall HUDOverlay BP class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget", meta = (AllowPrivateAccess = true))
	TSubclassOf<UUserWidget>HUDOverlayClass;

	// Variable to Hold the HUDOverlay Widget after Created
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widget", meta = (AllowPrivateAccess = true))
	UUserWidget* HUDOverlay;

protected:
	virtual void BeginPlay() override;

};	



