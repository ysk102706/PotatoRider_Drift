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

	PowerPlantComp = CreateDefaultSubobject<UChassisComponent>(TEXT("PowerPlantComp"));
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

	float Velocity = PowerPlantComp->CalculateVelocity();  
	
	SetActorLocation(GetActorLocation() + GetActorForwardVector() * Velocity * DeltaTime, true); 
}

void AMain_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (auto* IMC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		IMC->BindAction(IA[int(EInputAction::Accelerator)], ETriggerEvent::Triggered, this, &AMain_Player::OnPressAccelerator); 
		IMC->BindAction(IA[int(EInputAction::Accelerator)], ETriggerEvent::Completed, this, &AMain_Player::OnReleaseAccelerator); 
	}
}

void AMain_Player::OnPressAccelerator(const FInputActionValue& Value)
{
	FVector2D v = Value.Get<FVector2D>(); 
	PowerPlantComp->Accelerator(v.X * 10); 
}

void AMain_Player::OnReleaseAccelerator(const FInputActionValue& Value)
{
	PowerPlantComp->Accelerator(0); 
}

