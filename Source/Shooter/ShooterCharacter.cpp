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
#include "Weapon.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Ammo.h"

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
	MouseAimingLookUpRate(0.6f),
	MouseAimingTurnRate(0.6f),
	

	// true when aiming weapon
	bAiming(false),

	// Camera Field ofView Values
	CameraDefaultFOV(0.f), // Set in beginplay
	CameraZoomedFOV(25.f),
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
	bShouldFire(true),
	AutomaticFireRate(0.1f),
	bFireButtonPressed(false),

	// Item Trace Variables
	bShouldTraceForItems(false),

	//Camera interpLocation 
	CameraInterpDistance(250.f),
	CameraInterpElevation(65.f),

	// Ammo Count
	Starting9mmAmmo(85),
	StartingARAmmo(120),

	//Combat Variables
	CombatState(ECombatState::ECS_Unoccupied),

	//Crouch
	bCrouching(false),

	//MovementSpeed
	BaseMovementSpeed(600.f),
	CrouchedMoveSpeed(300.f),

	// Capsule Half Height sized
	StandingCapsuleHalfHeight(88.f),
	CrouchedCapsuleHalfHeight(44.f),

	//Groun Frection Stanidng/ Crouching
	BaseGroundFriction(2.f),
	CrouchedGroundFriction(100.f),

	//AimingBottonPressed
	bAimingButtonPressed(false)



{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create camera boom(pulls in toweards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 240.f; // Camera follow this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on controller
	CameraBoom->SocketOffset = FVector(0.f, 40.f, 70.f);

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
	
	// Create Hand Scean Component
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));

	// Create Interoplation Component
	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interp Component"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());

	InterpCom1 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component1"));
	InterpCom1->SetupAttachment(GetFollowCamera());

	InterpCom2 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component2"));
	InterpCom2->SetupAttachment(GetFollowCamera());

	InterpCom3 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component3"));
	InterpCom3->SetupAttachment(GetFollowCamera());

	InterpCom4 = CreateDefaultSubobject<USceneComponent>(TEXT(" Interp Component4"));
	InterpCom4->SetupAttachment(GetFollowCamera());

	InterpCom5 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component5"));
	InterpCom5->SetupAttachment(GetFollowCamera());

	InterpCom6 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component6"));
	InterpCom6->SetupAttachment(GetFollowCamera());


}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// For ADS cameraZoom
	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	// Spawn the default weapon and attach to the mesh
	EquipWeapon(SpawnDefaultWeapon());
	

	InitializeAmmoMap();

	// Create FinterpLocation structs for each interp location. add to arry
	InitializedInterpLocation();
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
	if (EquippedWeapon == nullptr) return;

	if (CombatState != ECombatState::ECS_Unoccupied) return;
	
	if (WeaponHasAmmo())
	{
		PlayFireSound();
		SentBullet();
		PlayGunFireMontage();

		// Subtract one from the weapon Ammo
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();

	}
	
	
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
	bAimingButtonPressed = true;
	if (CombatState != ECombatState::ECS_Reload)
	{
		Aim();
	}
}

void AShooterCharacter::AimingButtonReleased()
{
	bAimingButtonPressed = false;
	StopAim();
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
	FireWeapon();
	
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (EquippedWeapon)
	{
		CombatState = ECombatState::ECS_FireTimerInPrograss;

		GetWorldTimerManager().SetTimer
		(AutoFireTimer, this,
			&AShooterCharacter::AutoFireReset, EquippedWeapon->GetRateOfFire());


		//AutomaticFireRate
	}
	
}

void AShooterCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed)
		{
			FireWeapon();
		}
	}
	else
	{
		// Auto Reload Weapon
		//ReloadWeapon();
	}
}

void AShooterCharacter::FinishReloading()
{
	// Update Combat State
	CombatState = ECombatState::ECS_Unoccupied;

	if (bAimingButtonPressed)
	{
		Aim();
	}

	if (EquippedWeapon == nullptr) return;
	
	const auto AmmoType{ EquippedWeapon->GetAmmoType() };

	//Update Ammo Map
	if (AmmoMap.Contains(AmmoType))
	{
		// Ammount of ammo the character carring of the equippedWeapon
		int32 CarriedAmmo = AmmoMap[AmmoType];

		// Space left in the magazine of equippedWeapon
		const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapasity() - EquippedWeapon->GetAmmo();

		if (MagEmptySpace > CarriedAmmo)
		{
			// Reload the magazine with all the ammo we are carring
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			// Fill the magazine
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
	

}

bool AShooterCharacter::CarringAmmo()
{
	if (EquippedWeapon == nullptr) return false;
	
	auto AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType]>0;
	}

	return false;
}

void AShooterCharacter::GrabClip()
{
	if (EquippedWeapon == nullptr) return;
	if (HandSceneComponent == nullptr) return;
	
	//Index for the clip bone on the equipped weapon
	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };

	// Store the transform of the clip
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);

	HandSceneComponent->AttachToComponent(GetMesh(),AttachmentRules,FName(TEXT("Hand_L")));

	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetMovingClip(true);

}

void AShooterCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}

void AShooterCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}
	if (bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchedMoveSpeed;
		GetCharacterMovement()->GroundFriction = CrouchedGroundFriction;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}

}



void AShooterCharacter::Jump()
{
	if (bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
	else
	{
		ACharacter::Jump();
	}
}

void AShooterCharacter::IntepCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight;
	if (bCrouching)
	{
		TargetCapsuleHalfHeight = CrouchedCapsuleHalfHeight;
	}
	else
	{
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;
	}

	const float IntepHalfHeight = FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaTime, 20.f);

	

	// Evelation character mesh from inside the floor
	// - Value if crouching + Value if standing
	const float DeltaCapsuleHalfHeight = IntepHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const FVector MeshOffset = FVector(0.f, 0.f, -DeltaCapsuleHalfHeight);
	
	GetMesh()->AddLocalOffset(MeshOffset);

	GetCapsuleComponent()->SetCapsuleHalfHeight(IntepHalfHeight);
}

void AShooterCharacter::Aim()
{
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchedMoveSpeed;
}

void AShooterCharacter::StopAim()
{
	bAiming = false;
	if (!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		// Get amount of ammo in our AmmoMap for Ammo's type
		int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] };
		AmmoCount += Ammo->GetItemCount();
		//Set the amount of ammo in the map for this type 
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}
	if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		// Check to see if the gun is empty
		if (EquippedWeapon->GetAmmo()==0)
		{
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

void AShooterCharacter::InitializedInterpLocation()
{
	FIntepLocation WeaponLocation{ WeaponInterpComp,0 };
	InterpLocations.Add(WeaponLocation);

	FIntepLocation InterpLoc1{ InterpCom1,0 };
	InterpLocations.Add(InterpLoc1);

	FIntepLocation InterpLoc2{ InterpCom2,0 };
	InterpLocations.Add(InterpLoc2);

	FIntepLocation InterpLoc3{ InterpCom3,0 };
	InterpLocations.Add(InterpLoc3);

	FIntepLocation InterpLoc4{ InterpCom4,0 };
	InterpLocations.Add(InterpLoc4);

	FIntepLocation InterpLoc5{ InterpCom5,0 };
	InterpLocations.Add(InterpLoc5);

	FIntepLocation InterpLoc6{ InterpCom6,0 };
	InterpLocations.Add(InterpLoc6);

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
			TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				// Show item's pickup widget
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
			}
			// We hi an AItem last frame
			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)
				{
					// we hitting different AItem this frame from last frame
					// or AItem is Null
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}

			// Store a reference to HitItem for nextframe
			TraceHitItemLastFrame = TraceHitItem;
		}
	}
	else if (TraceHitItemLastFrame)
	{
		// No longaer overlapping any items 
		// item last from should not show widget
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	// Chech the TSubclassOf Variable
	if (DefaultWeaponClass)
	{
		// SPawn the wapon
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
		
	}
	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquipped)
{
	if (WeaponToEquipped)
	{
		// Get the hand Socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			// Attach the wapont to the hand socket RightHandSocket
			HandSocket->AttachActor(WeaponToEquipped, GetMesh());
		}
		EquippedWeapon = WeaponToEquipped;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}

}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if (TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this);
		
	}
	
}

void AShooterCharacter::SelectButtonReleased()
{
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);

}

bool AShooterCharacter::WeaponHasAmmo()
{

	if (EquippedWeapon == nullptr) return false;

	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AShooterCharacter::SentBullet()
{

	// We need to store barrel socket location so
   // if the socket exist on the skeletal it will return in but not the actual location
   // just the socket name
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		// Get the actual  Transform (location, Rotation, Scale)  of the socket 
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
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
}

void AShooterCharacter::PlayGunFireMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}


	// StartBulletFireTImer for corsshair
	StartCrosshairBulletFire();
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (EquippedWeapon == nullptr) return;
	

	// Do we have currnet ammo of type weapon?
	if (CarringAmmo() && !EquippedWeapon->ClipIsFull()) // Replace with carrying Ammo
	{
		if (bAiming)
		{
			StopAim();
		}
		CombatState = ECombatState::ECS_Reload;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (ReloadMontage && AnimInstance)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}

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
	
	IntepCapsuleHalfHeight(DeltaTime);
	
	
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

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	// Reload Weapon
	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);

	//Crouch
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);


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

FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
	const FVector CameraForward{ FollowCamera->GetForwardVector() };

	//Desired Location = CameraWorldLocation + CameraForward*A +Up * B
	return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	if (Item->GetEquipSound())
	{
		UGameplayStatics::PlaySound2D(this, Item->GetEquipSound());
	}

	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		SwapWeapon(Weapon);
	}
	auto Ammo = Cast<AAmmo>(Item);
	if (Ammo)
	{
		PickupAmmo(Ammo);
	}
}

FIntepLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
	if (Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}

	return FIntepLocation();
}

