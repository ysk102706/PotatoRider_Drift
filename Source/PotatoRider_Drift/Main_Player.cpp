// Fill out your copyright notice in the Description page of Project Settings.


#include "Main_Player.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ChassisComponent.h"
#include "RacingGameMode.h"
#include "UIManager.h"
#include "Utility.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "SpeedometerUI.h"

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
	CameraComp->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f));

	ChassisComp = CreateDefaultSubobject<UChassisComponent>(TEXT("ChassisComp"));
}

void AMain_Player::BeginPlay()
{
	Super::BeginPlay();

	if (auto* pc = Cast<APlayerController>(Controller))
	{
		auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());
		Subsystem->AddMappingContext(IMC_Player, 0);
	}

	LastPositionData = {GetActorLocation(), GetActorForwardVector(), FVector::ZeroVector};

	GameMode = GetWorld()->GetAuthGameMode<ARacingGameMode>();
	if (GameMode)
	{
		GameMode->UI()->ShowWidget(GetWorld(), EWidgetType::BoosterUI);
		GameMode->UI()->ShowWidget(GetWorld(), EWidgetType::SpeedometerUI);
	}
}

void AMain_Player::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ChassisComp->IsBreakDrift())
	{
		SetCameraAndMeshRotation();
		ChassisComp->BreakDrift();
	}

	float Velocity = ChassisComp->CalculateVelocity(true) / (GameMode->IsRaceEnd() ? 2.5f : 1.0f);
	FQuat Q = ChassisComp->CalculateQuat();
	float CorrectedVelocity = Velocity / (ChassisComp->IsFullDrift() ? 5.0f : 2.0f);
	FVector Forward = GetActorForwardVector();
	float Dir = FMath::Sign(Forward.Cross(Q.RotateVector(Forward)).Z);
	float QuatCorrectedRate = 1.0f + (FMath::Max(FMath::Abs(CorrectedVelocity), 45.0f) - 45.0f) * 0.5f / 235.0f;
	FQuat CorrectedQ = FQuat(FVector(0, 0, 1), Q.GetAngle() * Dir * QuatCorrectedRate);

	if (!HandleInputStack.Num() && FMath::RadiansToDegrees(Q.GetAngle()) < 0.001f && FMath::Abs(
		SpringArmComp->GetRelativeRotation().Yaw) < 0.001f)
	{
		SetCameraAndMeshRotation();
	}

	FVector RotatedForward = (ChassisComp->IsDrift() && !ChassisComp->IsFullDrift() ? Q : CorrectedQ).
		RotateVector(Forward);
	FVector CentrifugalForceDir(0);
	FPositionData CurPositionData = {GetActorLocation(), GetActorForwardVector(), GetActorRightVector() * -ChassisComp->GetDriftDir()};
	if (Utility::Between_EE(CorrectedVelocity * 0.036f, 20.0f, 350.0f) && ChassisComp->IsDrift() && LastPositionData.Right.Dot(GetActorRightVector() * -ChassisComp->GetDriftDir()) > 0.0f)
	{
		CentrifugalForceDir = GetActorRightVector() * -ChassisComp->GetDriftDir();
		CurPositionData.Right = CentrifugalForceDir;
		float CentrifugalForce = Utility::CalculateCentrifugalForce(CorrectedVelocity, LastPositionData, CurPositionData);
		CentrifugalForceDir *= CentrifugalForce * DeltaTime * ChassisComp->GetDriftAngleRate();

		// FString str = FString::Printf(
		// 	TEXT(
		// 		"Drift.LastDriftAngle : %f, GetDriftAngleRate : %f\nVelocity : %f, RotatedForward.Length : %f\nPre : %f, %f, %f\nCur : %f, %f, %f\nCentrifugalForceDir : %f, %f, %f\nCentrifugalForce : %f"),
		// 	ChassisComp->Drift.LastDriftAngle, ChassisComp->GetDriftAngleRate(), Velocity, RotatedForward.Length(),
		// 	LastPositionData.Position.X, LastPositionData.Position.Y,
		// 	LastPositionData.Position.Z,
		// 	CurPositionData.Position.X, CurPositionData.Position.Y, CurPositionData.Position.Z,
		// 	CentrifugalForceDir.X, CentrifugalForceDir.Y, CentrifugalForceDir.Z, CentrifugalForce);
		// DrawDebugString(GetWorld(), GetActorLocation(), str, nullptr, FColor::Yellow, 0, true, 1);
		// UE_LOG(LogTemp, Warning, TEXT("%s"), *str);
	} 
	LastPositionData = CurPositionData;
	MySetActorLocation(GetActorLocation() + RotatedForward * Velocity * DeltaTime + CentrifugalForceDir.GetClampedToSize2D(-1000, 1000), true);
	SetActorRotation(RotatedForward.Rotation());

	FVector MeshForward = FVector(1.0f, 0.0f, 0.0f);
	MeshRot = Q.RotateVector(MeshForward).Rotation();
	MeshRot.Yaw /= DeltaTime;
	BoxComp->SetRelativeRotation(MeshRot);

	if (SpringArmComp->GetRelativeRotation().Yaw != 0.0f)
	{
		FVector CameraForward = FMath::Lerp(AimPoint->GetRelativeLocation().GetSafeNormal(), FVector(1.0f, 0.0f, 0.0f), 0.02f);
		FRotator rot = CameraForward.Rotation();
		AimPoint->SetRelativeLocation((rot).Vector() * 200.0f);
		SpringArmComp->SetRelativeRotation(rot);
	}

	FVector Start = GetActorLocation();
	FVector End = GetActorLocation() + BoxComp->GetForwardVector() * 500.0f;
	//DrawDebugLine(GetWorld(), Start, End, FColor::Red);

	//GetWorld()->SpawnActor<AActor>(TempSkidMark, GetActorTransform());

	auto* SpeedometerUI = GameMode->UI()->GetWidget<USpeedometerUI>(GetWorld(), EWidgetType::SpeedometerUI);
	SpeedometerUI->UpdateSpeed(Velocity * 0.036f);

	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("%f"), Velocity * 0.036f)); 
}

void AMain_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (auto* IMC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		IMC->BindAction(IA[int(EInputAction::Accelerator)], ETriggerEvent::Triggered, this,
		                &AMain_Player::OnPressAccelerator);
		IMC->BindAction(IA[int(EInputAction::Accelerator)], ETriggerEvent::Completed, this,
		                &AMain_Player::OnReleaseAccelerator);

		IMC->BindAction(IA[int(EInputAction::Decelerator)], ETriggerEvent::Triggered, this,
		                &AMain_Player::OnPressDecelerator);
		IMC->BindAction(IA[int(EInputAction::Decelerator)], ETriggerEvent::Completed, this,
		                &AMain_Player::OnReleaseDecelerator);

		IMC->BindAction(IA[int(EInputAction::LeftHandle)], ETriggerEvent::Triggered, this,
		                &AMain_Player::OnPressLeftHandle);
		IMC->BindAction(IA[int(EInputAction::LeftHandle)], ETriggerEvent::Completed, this,
		                &AMain_Player::OnReleaseLeftHandle);

		IMC->BindAction(IA[int(EInputAction::RightHandle)], ETriggerEvent::Triggered, this,
		                &AMain_Player::OnPressRightHandle);
		IMC->BindAction(IA[int(EInputAction::RightHandle)], ETriggerEvent::Completed, this,
		                &AMain_Player::OnReleaseRightHandle);

		IMC->BindAction(IA[int(EInputAction::Drift)], ETriggerEvent::Started, this, &AMain_Player::OnPressStartDrift);
		IMC->BindAction(IA[int(EInputAction::Drift)], ETriggerEvent::Completed, this, &AMain_Player::OnReleaseDrift);

		IMC->BindAction(IA[int(EInputAction::Booster)], ETriggerEvent::Started, this, &AMain_Player::OnActionBooster);
	}
}

void AMain_Player::OnPressAccelerator(const FInputActionValue& Value)
{
	if (GameMode->IsRaceEnd())
	{
		ChassisComp->Accelerator(0);
	}
	else
	{
		float v = Value.Get<float>();
		Accelerator(v);
	}
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
	if (GameMode->IsRaceEnd())
	{
		ChassisComp->Accelerator(0);
	}
	else
	{
		float v = Value.Get<float>();
		Accelerator(v);
	}
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
	if (GameMode->IsRaceEnd())
	{
		ChassisComp->Handle(0);
	}
	else
	{
		float v = Value.Get<float>();
		Handle(v);
	}
}

void AMain_Player::OnReleaseLeftHandle(const FInputActionValue& Value)
{
	HandleInputStack.Remove(-1);
	if (!HandleInputStack.Num())
	{
		ChassisComp->Handle(0);
	}
}

void AMain_Player::OnPressRightHandle(const FInputActionValue& Value)
{
	if (GameMode->IsRaceEnd())
	{
		ChassisComp->Handle(0);
	}
	else
	{
		float v = Value.Get<float>();
		Handle(v);
	}
}

void AMain_Player::OnReleaseRightHandle(const FInputActionValue& Value)
{
	HandleInputStack.Remove(1);
	if (!HandleInputStack.Num())
	{
		ChassisComp->Handle(0);
	}
}

void AMain_Player::OnPressStartDrift(const FInputActionValue& Value)
{
	if (!GameMode->IsRaceEnd())
	{
		ChassisComp->DriftDevice(true);
	}
}

void AMain_Player::OnReleaseDrift(const FInputActionValue& Value)
{
	ChassisComp->DriftDevice(false);
}

void AMain_Player::OnActionBooster(const FInputActionValue& Value)
{
	if (!GameMode->IsRaceEnd())
	{
		ChassisComp->BoosterDevice();
	}
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
	float Velocity = ChassisComp->CalculateVelocity(true) * 0.036f;

	if (!sz || (sz == 1 && HandleInputStack[0] != Value))
	{
		if (FMath::Abs(Velocity) < 100.0f)
		{
			HandleInputStack.Add(Value);
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

void AMain_Player::SetCameraAndMeshRotation()
{
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

void AMain_Player::MySetActorLocation(const FVector& NewLocation, bool bSweep = false)
{
	if (NewLocation.X > 999999999.f || NewLocation.X < -999999999.0f ||
		NewLocation.Y > 999999999.f || NewLocation.Y < -999999999.0f ||
		NewLocation.Z > 999999999.f || NewLocation.Z < -999999999.0f)
	{
		UE_LOG(LogTemp, Display, TEXT("MySetActorLocation"));
	}
	else 
	{
		double x = FMath::Clamp(NewLocation.X, -999999999.0f, 999999999.0f);
		double y = FMath::Clamp(NewLocation.Y, -999999999.0f, 999999999.0f);
		double z = FMath::Clamp(NewLocation.Z, -999999999.0f, 999999999.0f);
		FVector n(x, y, z);
		SetActorLocation(n, bSweep);
	}
} 
