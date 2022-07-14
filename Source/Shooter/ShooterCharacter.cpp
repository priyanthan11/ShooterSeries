// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework\SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Components/WidgetComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :

	// Base Rate Turning/Lookingup
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),

	// Turn rates for aiming/not aiming
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	// Mouse Look sensitivity scale factors
	MouseHipTurnRate(1.0f),
	MouseHipLookupRate(1.0f),
	MouseAimingTurnRate(0.2f),
	MouseAimingLookUpRate(0.2f),

	// true when aiming weapon
	bAiming(false),

	// Camera Field ofView Values
	CameraDefaultFOV(0.f), // Set in beginplay
	CameraZoomedFOV(35.f),
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(20.f),

	// Crosshair SpreadFactors
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CorsshairShootingFactor(0.f),

	//BulletFire timer Variable
	ShootTimeDuration(0.05f),
	bFiringBullet(false),

	//Automatic  fire Variables
	AutomaticFireRate(0.1f),
	bShouldFire(true),
	bFireButtonPressed(false),

	// Item Trace Variables
	bShouldTraceForItems(false)

{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create camera boom(pulls in toweards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f; // Camera follow this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	// Create a FollowCamera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName); // Attach camera to end of boom
	FollowCamera->bUsePawnControlRotation = false; // Camera dose not rotate relative to arm

	//Dont rotate when the controller rotate. Let the controller onlyaffect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	//Configure character movement 
	GetCharacterMovement()->bOrientRotationToMovement = false;//Character moves in the directon of input
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// For ADS cameraZoom
	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
	}

	CameraCurrentFOV = CameraDefaultFOV; 
	//---------------------------------------------------------------------//
}

void AShooterCharacter::MoveForward(float Value)
{
	MovementInput(true, Value);
}

void AShooterCharacter::MoveRight(float Value)
{
	MovementInput(false, Value);
}

void AShooterCharacter::MovementInput(bool bIsforwardAxis, float Value)
{
	if ((Controller!=nullptr) && (Value != 0.f))
	{
		// find out which way is forward / Right
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0,Rotation.Yaw,0 };

		if (bIsforwardAxis == true)
		{
			const FVector Direction{ FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) };
			AddMovementInput(Direction, Value);
		}
		else
		{
			const FVector Direction{ FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) };
			AddMovementInput(Direction, Value);
		}
	}

}

void AShooterCharacter::TurnAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AShooterCharacter::LookUpRate(float Rate)
{

	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor;
	if (bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactor;
	if (bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookupRate;
	}
	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this,FireSound);
	}
	 // We need to store barrel socket location so
	// if the socket exist on the skeletal it will return in but not the actual location
	// just the socket name
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		// Get the actual  Transform (location, Rotation, Scale)  of the socket 
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;

		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);

		if (bBeamEnd)
		{
			// Spawn impact particles after updating beamEndpoint
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
					ImpactParticles,
					BeamEnd);
			}

			
			UParticleSystemComponent* Beam =
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
			BeamParticles, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
				
			}
		}

	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
	// StartBulletFireTImer for corsshair
	StartCrosshairBulletFire();
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	// Check for crosshair trace hit
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult,OutBeamLocation);

	if (bCrosshairHit)
	{
		//Tentative beam location - still need to trace from gun
		OutBeamLocation = CrosshairHitResult.Location;
	}
	else // No crosshair trace hit
	{
		// OutBeamLocation the end location for the line trace
	}
	
	//prefrom a second trace, this time from the gun barrel
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart = MuzzleSocketLocation;
	const FVector StartToEnd = OutBeamLocation - MuzzleSocketLocation;
	const FVector WeaponTraceEnd = MuzzleSocketLocation + StartToEnd *1.25f;

	GetWorld()->LineTraceSingleByChannel(
		WeaponTraceHit,
		WeaponTraceStart,
		WeaponTraceEnd,
		ECollisionChannel::ECC_Visibility);

	if (WeaponTraceHit.bBlockingHit) // Obj between barrel and BeamEndPoint?
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}
	
	return false;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	// Set Current CameraFOV
	if (bAiming)
	{
		// Introplate Zoomed FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);

	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);

}

void AShooterCharacter::SetLookRates()
{

	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
		
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
		
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{ 0.f,600.f };
	FVector2D VeclocityMultiplierRange{ 0.f,1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f;
	// Calculate crosshair in Velocity factor
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
	WalkSpeedRange, VeclocityMultiplierRange,
	Velocity.Size());

	// Calculate crosshair in air factor
	if (GetCharacterMovement()->IsFalling()) // is in air ?
	{
		// Spread the crosshair slowly while in air
		CrosshairInAirFactor = FMath::FInterpTo
		(CrosshairInAirFactor,2.25f, DeltaTime, 2.25f );
	}
	else // character on the ground
	{
		// Shrink the crosshairs rapidly while on the ground
		CrosshairInAirFactor = FMath::FInterpTo
		(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}
	// Calculate crosshair Aim Factor
	if (bAiming) // Are we aiming
	{
		// Shrink crosshair a small amount quickly
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.8f, DeltaTime, 30.f);
	}
	else
	{
		// Spread Crosshair back to very quickly
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}
	// True 0.05s after firing
	if (bAiming)
	{
		CorsshairShootingFactor = FMath::FInterpTo
		(CorsshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CorsshairShootingFactor = FMath::FInterpTo
		(CorsshairShootingFactor, 0.0f, DeltaTime, 60.f);
	}

	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CorsshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer
		(AutoFireTimer, this,
		&AShooterCharacter::AutoFireReset, AutomaticFireRate);
	}
}

void AShooterCharacter::AutoFireReset()
{
	bShouldFire = true;
	if (bFireButtonPressed)
	{
		StartFireTimer();
	}
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	// Get Current size of viewport
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get Screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPossition;
	FVector CrosshairWorldDirection;

	// Get world position and direction of crosshair
	bool bScreentoWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation, CrosshairWorldPossition,
		CrosshairWorldDirection);

	if (bScreentoWorld)
	{
		// Trace from crosshair world location outward 
		const FVector Start = CrosshairWorldPossition;
		const FVector End = Start + CrosshairWorldDirection * 50'000.f;
		OutHitLocation = End;

		GetWorld()->LineTraceSingleByChannel(
			OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}

	return false;
}

void AShooterCharacter::TraceForItems()
{

	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);
		if (ItemTraceResult.bBlockingHit)
		{
			AItem* HitItem = Cast<AItem>(ItemTraceResult.GetActor());
			if (HitItem && HitItem->GetPickupWidget())
			{
				// Show item's pickup widget
				HitItem->GetPickupWidget()->SetVisibility(true);
			}
			// We hi an AItem last frame
			if (TraceHitItemLastFrame)
			{
				if (HitItem != TraceHitItemLastFrame)
				{
					// we hitting different AItem this frame from last frame
					// or AItem is Null
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}

			// Store a reference to HitItem for nextframe
			TraceHitItemLastFrame = HitItem;
		}
	}
	else if (TraceHitItemLastFrame)
	{
		// No longaer overlapping any items 
		// item last from should not show widget
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle interpolation when camera zoomed
	CameraInterpZoom(DeltaTime);

	// Change Look sensitivity based on aiming
	SetLookRates();

	CalculateCrosshairSpread(DeltaTime);
	/*Check Overlapped item count then trace for item*/
	TraceForItems();
	
	
	
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpRate);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);

}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true; 
	}
}

