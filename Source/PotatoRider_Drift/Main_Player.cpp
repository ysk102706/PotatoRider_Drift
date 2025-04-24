// Fill out your copyright notice in the Description page of Project Settings.


#include "Main_Player.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h" 
#include "EnhancedInputSubsystems.h" 
#include "ChassisComponent.h"
#include "Utility.h"

AMain_Player::AMain_Player()
{
 	PrimaryActorTick.bCanEverTick = true;

	ChassisComp = CreateDefaultSubobject<UChassisComponent>(TEXT("PowerPlantComp"));
}

void AMain_Player::BeginPlay()
{
	Super::BeginPlay();

	if (auto* pc = Cast<APlayerController>(Controller))
	{
		auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());
		Subsystem->AddMappingContext(IMC_Player, 0); 
	}
}

void AMain_Player::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Velocity = ChassisComp->CalculateVelocity();
	FVector Forward = ChassisComp->CalculateForwardDirection(GetActorForwardVector()); 

	SetActorLocation(GetActorLocation() + Forward * Velocity * DeltaTime, true); 
	SetActorRotation(Forward.Rotation()); 

	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("%f"), Velocity * 0.036f));
}

void AMain_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (auto* IMC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		IMC->BindAction(IA[int(EInputAction::Accelerator)], ETriggerEvent::Triggered, this, &AMain_Player::OnPressAccelerator); 
		IMC->BindAction(IA[int(EInputAction::Accelerator)], ETriggerEvent::Completed, this, &AMain_Player::OnReleaseAccelerator); 

		IMC->BindAction(IA[int(EInputAction::Handle)], ETriggerEvent::Triggered, this, &AMain_Player::OnPressHandle); 
		IMC->BindAction(IA[int(EInputAction::Handle)], ETriggerEvent::Completed, this, &AMain_Player::OnReleaseHandle); 
	}
}

void AMain_Player::OnPressAccelerator(const FInputActionValue& Value)
{
	float v = Value.Get<float>(); 
	ChassisComp->Accelerator(v * 10); 
}

void AMain_Player::OnReleaseAccelerator(const FInputActionValue& Value)
{
	ChassisComp->Accelerator(0); 
}

void AMain_Player::OnPressHandle(const FInputActionValue& Value)
{
	float v = Value.Get<float>(); 
	ChassisComp->RotateHandle(v); 
}

void AMain_Player::OnReleaseHandle(const FInputActionValue& Value)
{
	ChassisComp->RotateHandle(0); 
}
