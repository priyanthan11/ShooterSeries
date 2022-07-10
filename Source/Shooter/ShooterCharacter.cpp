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

// Sets default values
AShooterCharacter::AShooterCharacter():
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create camera boom(pulls in toweards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f; // Camera follow this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 50.f);

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

		// Get Current size of viewport
		FVector2D ViewportSize;
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);
		}

		// Get Screen space location of crosshairs
		FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		CrosshairLocation.Y -= 50.f;
		FVector CrosshairWorldPossition;
		FVector CrosshairWorldDirection;

		// Get world position and direction of crosshair
		bool bScreentoWorld = UGameplayStatics::DeprojectScreenToWorld(
			UGameplayStatics::GetPlayerController(this, 0), 
			CrosshairLocation, CrosshairWorldPossition, 
			CrosshairWorldDirection);
		 
		// was deprojection sucessful?
		if (bScreentoWorld)
		{
			FHitResult ScreenTraceHit;
			const FVector Start = CrosshairWorldPossition;
			const FVector End = CrosshairWorldPossition + CrosshairWorldDirection * 50'000.f;

			// set beamednpoint to line trace end point
			FVector BeamEndpoint = End;

			// Trace outward from crosshairs world location
			GetWorld()->LineTraceSingleByChannel(
			ScreenTraceHit,
			Start,
			End, 
			ECollisionChannel::ECC_Visibility);

			// was thre a trace hit?
			if (ScreenTraceHit.bBlockingHit)
			{
				// BeamEndPoint is now trace hit location
				BeamEndpoint = ScreenTraceHit.Location;
			} 

			//prefrom a second trace, this time from the gun barrel
			FHitResult WeaponTraceHit;
			const FVector WeaponTraceStart = SocketTransform.GetLocation();
			const FVector WeaponTraceEnd = BeamEndpoint;

			GetWorld()->LineTraceSingleByChannel(
			WeaponTraceHit,
			WeaponTraceStart,
			WeaponTraceEnd,
			ECollisionChannel::ECC_Visibility);

			if (WeaponTraceHit.bBlockingHit) // Obj between bareel and BeamEndPoint?
			{
				BeamEndpoint = WeaponTraceHit.Location;
			}

			// Spawn impact particles after updating beamEndpoint
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
					ImpactParticles,
					BeamEndpoint);
			}



			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = 
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
				BeamParticles, SocketTransform);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEndpoint);
				}
			}
		}

		/*
		FHitResult FireHit;
		const FVector Start = SocketTransform.GetLocation();
		const FQuat Rotation = SocketTransform.GetRotation();
		const FVector RoationAxis = Rotation.GetAxisX();
		const FVector End = Start + RoationAxis * 50'000.f;

		FVector BeamEndPoint = End;

		GetWorld()->LineTraceSingleByChannel(FireHit, Start, End, ECollisionChannel::ECC_Visibility);
		if (FireHit.bBlockingHit)
		{
			//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f);
			//DrawDebugPoint(GetWorld(), FireHit.Location, 5.f, FColor::Red, false, 2.f);

			BeamEndPoint = FireHit.Location; 

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.Location);
			}
		} 
		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
			}
		}
		*/
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireWeapon);
}

