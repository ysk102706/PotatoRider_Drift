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
#include "NiagaraComponent.h" 

AMain_Player::AMain_Player()
{
	PrimaryActorTick.bCanEverTick = true;
	
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	SetRootComponent(BoxComp);
	BoxComp->SetBoxExtent(FVector(50.0f));
	BoxComp->SetRelativeScale3D(FVector(0.65f, 0.85f, 0.85f));

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(BoxComp);

	AimPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AimPoint"));
	AimPoint->SetupAttachment(BoxComp);
	AimPoint->SetRelativeLocation(FVector(200.0f, 0.0f, 0.0f));

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(AimPoint);
	SpringArmComp->TargetArmLength = 650.0f;
	SpringArmComp->TargetOffset = FVector(0, 0, 200.0f);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);
	CameraComp->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f));

	LeftSkidMark = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LeftSkidMark"));
	RightSkidMark = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RightSkidMark")); 
	LeftSkidMark->SetupAttachment(MeshComp, TEXT("LB_SkidMark")); 
	RightSkidMark->SetupAttachment(MeshComp, TEXT("RB_SkidMark")); 

	LeftBooster = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LeftBooster"));
	RightBooster = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RightBooster"));
	LeftBooster->SetupAttachment(MeshComp, TEXT("L_Booster"));
	RightBooster->SetupAttachment(MeshComp, TEXT("R_Booster"));
	
	ChassisComp = CreateDefaultSubobject<UChassisComponent>(TEXT("ChassisComp"));
}

void AMain_Player::BeginPlay() 
{
	Super::BeginPlay();

	BoxComp->OnComponentHit.AddDynamic(this, &AMain_Player::OnBoxCompHit); 
	
	if (auto* pc = Cast<APlayerController>(Controller))
	{ 
		auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());
		Subsystem->AddMappingContext(IMC_Player, 0);
	}

	Forward = GetActorForwardVector();
	Right = GetActorRightVector(); 
	LastPositionData = {GetActorLocation(), Forward, Right};

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

	float Velocity = ChassisComp->CalculateVelocity(true) / (GameMode->IsRaceEnd() ? 2.5f : 1.0f);
	float CorrectedVelocity = Velocity / 1.5f; 
	
	FQuat Q = ChassisComp->CalculateQuat();
	float Dir = FMath::Sign(Forward.Cross(Q.RotateVector(Forward)).Z);
	float QuatCorrectedRate = 1.0f + (FMath::Max(FMath::Abs(CorrectedVelocity), 45.0f) - 45.0f) * 0.5f / 235.0f;
	FQuat CorrectedQ = FQuat(FVector(0, 0, 1), Q.GetAngle() * Dir * QuatCorrectedRate);
	
	FVector RotatedForward = (ChassisComp->IsDrift() && !ChassisComp->IsFullDrift() ? Q : CorrectedQ).RotateVector(Forward);
	CentrifugalForceDir = FMath::Lerp(CentrifugalForceDir, FVector(0), 0.01f); 
	FPositionData CurPositionData = {GetActorLocation(), Forward, Right * -ChassisComp->GetDriftDir()};
	if (Utility::Between_EE(CorrectedVelocity * 0.036f, 20.0f, 350.0f) && ChassisComp->IsDrift() && LastPositionData.Right.Dot(Right * -ChassisComp->GetDriftDir()) > 0.0f)
	{
		CentrifugalForceDir = Right * -ChassisComp->GetDriftDir();
		CurPositionData.Right = CentrifugalForceDir;
		float CentrifugalForce = Utility::CalculateCentrifugalForce(CorrectedVelocity, LastPositionData, CurPositionData);
		CentrifugalForceDir *= CentrifugalForce * DeltaTime * ChassisComp->GetDriftAngleRate(); 
	} 
	LastPositionData = CurPositionData;
	
	if (InelasticForce.Length() > 0.0f) 
	{
		InelasticForce = FMath::Lerp(InelasticForce, FVector(0), 0.25f); 
	} 
	
	MySetActorLocation(GetActorLocation() + RotatedForward * Velocity * DeltaTime + CentrifugalForceDir.GetClampedToSize2D(-1000, 1000) + InelasticForce, true);

	float MeshAngle = Q.GetAngle() * Dir / DeltaTime; 
	FQuat MeshQ(FVector(0, 0, 1), MeshAngle);
	FRotator MeshRot = MeshQ.RotateVector(Forward).Rotation(); 
	SetActorRotation(MeshRot);
	
	CameraRot = MeshQ.UnrotateVector(FVector(1.0f, 0.0f, 0.0f)).Rotation(); 
	if (!HandleInputStack.Num() || ChassisComp->IsRemainDrift())
	{
		CameraRot = FMath::Lerp(SpringArmComp->GetRelativeRotation(), FRotator(0), 0.005f);
	} 
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("%f"), CameraRot.Yaw)); 
	AimPoint->SetRelativeLocation(CameraRot.Vector() * 200.0f);
	SpringArmComp->SetRelativeRotation(CameraRot); 
	SpringArmComp->TargetArmLength = FMath::Lerp(SpringArmComp->TargetArmLength, ChassisComp->IsBoost() ? 1000.0f : 650.0f, 0.025f);

	Forward = RotatedForward;
	Right = Forward.RightVector; 

	auto* SpeedometerUI = GameMode->UI()->GetWidget<USpeedometerUI>(GetWorld(), EWidgetType::SpeedometerUI);
	SpeedometerUI->UpdateSpeed(Velocity * 0.036f);

	if (ChassisComp->IsDrift())
	{
		LeftSkidMark->Activate();
		RightSkidMark->Activate();
	}
	else
	{ 
		LeftSkidMark->Deactivate();
		RightSkidMark->Deactivate(); 
	}

	if (ChassisComp->IsBoost())
	{
		LeftBooster->Activate(); 
		RightBooster->Activate(); 
	} 
	else
	{
		LeftBooster->Deactivate();
		RightBooster->Deactivate(); 
	}
	
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("%f"), Velocity * 0.036f)); 
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

void AMain_Player::MySetActorLocation(const FVector& NewLocation, bool bSweep)
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

void AMain_Player::OnBoxCompHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{ 
	float Velocity = ChassisComp->CalculateVelocity(true); 
	FVector Dir = Utility::CalculateReflectionVector(Forward, NormalImpulse); 
	InelasticForce = Utility::CalculateInelasticCollision(Dir, Velocity) * GetWorld()->GetDeltaSeconds();
	UE_LOG(LogTemp, Warning, TEXT("%f"), InelasticForce.Length()); 
}
