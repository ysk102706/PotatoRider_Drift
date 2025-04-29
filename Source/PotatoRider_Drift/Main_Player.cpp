// Fill out your copyright notice in the Description page of Project Settings.


#include "Main_Player.h"
#include "EnhancedInputComponent.h" 
#include "EnhancedInputSubsystems.h" 
#include "ChassisComponent.h"
#include "Utility.h" 
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"

AMain_Player::AMain_Player()
{
 	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root")); 
	SetRootComponent(Root); 

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	BoxComp->SetupAttachment(Root); 
	BoxComp->SetBoxExtent(FVector(50.0f));
	BoxComp->SetRelativeScale3D(FVector(2.0f, 1.5f, 0.75f)); 

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp")); 
	MeshComp->SetupAttachment(BoxComp); 
	
	AimPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AimPoint")); 
	AimPoint->SetupAttachment(Root);
	AimPoint->SetRelativeLocation(FVector(200.0f, 0.0f, 0.0f));

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(AimPoint); 
	SpringArmComp->TargetArmLength = 700.0f;
	SpringArmComp->TargetOffset = FVector(0, 0, 200.0f); 
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);
	CameraComp->SetRelativeRotation(FRotator(-25.0f, 0.0f, 0.0f));
	
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

	LastPositionData = { GetActorLocation(), GetActorForwardVector(), GetActorRightVector()}; 
}

void AMain_Player::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); 

	float Velocity = ChassisComp->CalculateVelocity();
	FQuat q = ChassisComp->CalculateQuat(); 
	FVector Forward = GetActorForwardVector(); 
	
	FVector RotatedForward = q.RotateVector(Forward); 
	if (!HandleInputStack.Num() || !ChassisComp->IsDrift() || GetActorRightVector() * -HandleInputStack.Top() != LastPositionData.Right)
	{ 
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("%f"), RotatedForward.Rotation().Yaw)); 
		SetActorLocation(GetActorLocation() + RotatedForward * Velocity * DeltaTime);
	}
	else
	{ 
		FVector CentrifugalForceDir = GetActorRightVector() * -HandleInputStack.Top(); 
		FPositionData CurPositionData({GetActorLocation(), GetActorForwardVector(), CentrifugalForceDir}); 
		float CentrifugalForce = Utility::CalculateCentrifugalForce(Velocity, LastPositionData, CurPositionData);
		CentrifugalForceDir *= CentrifugalForce * DeltaTime * ChassisComp->GetDriftAngleRate(); 
		LastPositionData = CurPositionData;
		
		SetActorLocation(GetActorLocation() + RotatedForward * Velocity * DeltaTime + CentrifugalForceDir); 
	}
	SetActorRotation(RotatedForward.Rotation()); 

	FVector MeshForward = FVector(1.0f, 0.0f, 0.0f);
	MeshRot = (HandleInputStack.Num() ? q.RotateVector(MeshForward).Rotation() : MeshForward.Rotation()); 
	MeshRot.Yaw /= DeltaTime; 
	BoxComp->SetRelativeRotation(MeshRot); 
	
	if (SpringArmComp->GetRelativeRotation().Yaw != 0.0f)
	{ 
		FVector CameraForward = FMath::Lerp(AimPoint->GetRelativeLocation().GetSafeNormal(), FVector(1.0f, 0.0f, 0.0f), 0.035f);
		FRotator rot = CameraForward.Rotation(); 
		AimPoint->SetRelativeLocation((rot).Vector() * 200.0f); 
		SpringArmComp->SetRelativeRotation(rot); 
	}

	FVector Start = GetActorLocation();
	FVector End = GetActorLocation() + BoxComp->GetForwardVector() * 500.0f; 
	DrawDebugLine(GetWorld(), Start, End, FColor::Red);
	
	GetWorld()->SpawnActor<AActor>(TempSkidMark, GetActorTransform());
} 

void AMain_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (auto* IMC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		IMC->BindAction(IA[int(EInputAction::Accelerator)], ETriggerEvent::Triggered, this, &AMain_Player::OnPressAccelerator); 
		IMC->BindAction(IA[int(EInputAction::Accelerator)], ETriggerEvent::Completed, this, &AMain_Player::OnReleaseAccelerator); 

		IMC->BindAction(IA[int(EInputAction::Decelerator)], ETriggerEvent::Triggered, this, &AMain_Player::OnPressDecelerator); 
		IMC->BindAction(IA[int(EInputAction::Decelerator)], ETriggerEvent::Completed, this, &AMain_Player::OnReleaseDecelerator); 

		IMC->BindAction(IA[int(EInputAction::LeftHandle)], ETriggerEvent::Triggered, this, &AMain_Player::OnPressLeftHandle); 
		IMC->BindAction(IA[int(EInputAction::LeftHandle)], ETriggerEvent::Completed, this, &AMain_Player::OnReleaseLeftHandle); 

		IMC->BindAction(IA[int(EInputAction::RightHandle)], ETriggerEvent::Triggered, this, &AMain_Player::OnPressRightHandle); 
		IMC->BindAction(IA[int(EInputAction::RightHandle)], ETriggerEvent::Completed, this, &AMain_Player::OnReleaseRightHandle); 
		
		IMC->BindAction(IA[int(EInputAction::Drift)], ETriggerEvent::Started, this, &AMain_Player::OnPressStartDrift); 
		IMC->BindAction(IA[int(EInputAction::Drift)], ETriggerEvent::Completed, this, &AMain_Player::OnReleaseDrift); 
	}
}

void AMain_Player::OnPressAccelerator(const FInputActionValue& Value)
{
	float v = Value.Get<float>(); 
	Accelerator(v); 
}

void AMain_Player::OnReleaseAccelerator(const FInputActionValue& Value)
{ 
	AccelerationInputStack.Remove(1);
	if (!AccelerationInputStack.Num())
	{
		ChassisComp->Accelerator(0);
	} 
}

void AMain_Player::OnPressDecelerator(const FInputActionValue& Value)
{
	float v = Value.Get<float>(); 
	Accelerator(v); 
}

void AMain_Player::OnReleaseDecelerator(const FInputActionValue& Value)
{
	AccelerationInputStack.Remove(-1);
	if (!AccelerationInputStack.Num())
	{
		ChassisComp->Accelerator(0);
	} 
}

void AMain_Player::OnPressLeftHandle(const FInputActionValue& Value)
{
	float v = Value.Get<float>(); 
	Handle(v); 
}

void AMain_Player::OnReleaseLeftHandle(const FInputActionValue& Value)
{ 
	HandleInputStack.Remove(-1);
	if (!HandleInputStack.Num())
	{
		ChassisComp->Handle(0); 
		ChassisComp->ResetHandleForce(); 

		FQuat q = FQuat(FVector(0.0f, 0.0f, 1.0f), FMath::DegreesToRadians(MeshRot.Yaw)); 
		FRotator Rot = q.RotateVector(GetActorForwardVector()).Rotation(); 
		SetActorRotation(Rot); 
		
		Rot = MeshRot; 
		Rot.Yaw *= -1.0f;
		Rot.Yaw += SpringArmComp->GetRelativeRotation().Yaw; 
		AimPoint->SetRelativeLocation(Rot.Vector() * 200.0f);
		SpringArmComp->SetRelativeRotation(Rot);
	} 
	else
	{ 
		LastPositionData = {GetActorLocation(), GetActorForwardVector(), GetActorRightVector()};
	}
}

void AMain_Player::OnPressRightHandle(const FInputActionValue& Value)
{
	float v = Value.Get<float>(); 
	Handle(v);
}

void AMain_Player::OnReleaseRightHandle(const FInputActionValue& Value)
{ 
	HandleInputStack.Remove(1);
	if (!HandleInputStack.Num())
	{
		ChassisComp->Handle(0); 
		ChassisComp->ResetHandleForce(); 

		FQuat q = FQuat(FVector(0.0f, 0.0f, 1.0f), FMath::DegreesToRadians(MeshRot.Yaw)); 
		FRotator Rot = q.RotateVector(GetActorForwardVector()).Rotation(); 
		SetActorRotation(Rot); 
		
		Rot = MeshRot; 
		Rot.Yaw *= -1.0f; 
		Rot.Yaw += SpringArmComp->GetRelativeRotation().Yaw; 
		AimPoint->SetRelativeLocation(Rot.Vector() * 200.0f);
		SpringArmComp->SetRelativeRotation(Rot);
	}
	else
	{ 
		LastPositionData = {GetActorLocation(), GetActorForwardVector(), GetActorRightVector() * -1};
	}
}

void AMain_Player::OnPressStartDrift(const FInputActionValue& Value)
{ 
	ChassisComp->DriftDevice(true); 
}

void AMain_Player::OnReleaseDrift(const FInputActionValue& Value)
{ 
	ChassisComp->DriftDevice(false); 
}

void AMain_Player::Accelerator(float Value)
{
	int sz = AccelerationInputStack.Num(); 
	if (!sz || (sz == 1 && AccelerationInputStack[0] != Value))
	{
		AccelerationInputStack.Add(Value);
	} 
	
	if (AccelerationInputStack.Top() == Value)
	{
		ChassisComp->Accelerator(Value * 20);
	} 
}

void AMain_Player::Handle(float Value)
{
	int sz = HandleInputStack.Num(); 
	float Velocity = ChassisComp->CalculateVelocity() * 0.036f;

	if (!sz || (sz == 1 && HandleInputStack[0] != Value))
	{
		if (FMath::Abs(Velocity) < 100.0f)
		{
			HandleInputStack.Add(Value);
			LastPositionData = {GetActorLocation(), GetActorForwardVector(), GetActorRightVector() * Value};
		}
		else 
		{
			HandleInputStack.Insert(Value, 0); 
		} 
	} 
	
	if (HandleInputStack.Top() == Value)
	{
		ChassisComp->Handle(Value);
	} 
}
