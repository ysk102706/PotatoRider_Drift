// Fill out your copyright notice in the Description page of Project Settings.


#include "Main_Player.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ChassisComponent.h"
#include "EngineUtils.h"
#include "../RacingGameMode.h"
#include "../../UIManager.h"
#include "../../Utility.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "../../UI/SpeedometerUI.h"
#include "NiagaraComponent.h" 
#include "../ResetPoint.h"
#include "NiagaraFunctionLibrary.h" 
#include "Components/SceneCaptureComponent2D.h"

AMain_Player::AMain_Player()
{
	PrimaryActorTick.bCanEverTick = true;
	
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	SetRootComponent(BoxComp);
	BoxComp->SetBoxExtent(FVector(135.0f, 75.0f, 50.0f));

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(BoxComp);
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -50.0f)); 
	MeshComp->SetRelativeScale3D(FVector(0.65f, 0.85f, 0.75f));

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

	MinimapCaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MinimapCaptureComp")); 
	MinimapCaptureComp->SetupAttachment(SpringArmComp); 
	MinimapCaptureComp->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f));
	
	L_SkidMark = CreateDefaultSubobject<UNiagaraComponent>(TEXT("L_SkidMark"));
	R_SkidMark = CreateDefaultSubobject<UNiagaraComponent>(TEXT("R_SkidMark")); 
	L_SkidMark->SetupAttachment(MeshComp, TEXT("LB_SkidMark")); 
	R_SkidMark->SetupAttachment(MeshComp, TEXT("RB_SkidMark")); 

	L_Booster = CreateDefaultSubobject<UNiagaraComponent>(TEXT("L_Booster"));
	R_Booster = CreateDefaultSubobject<UNiagaraComponent>(TEXT("R_Booster"));
	L_Booster->SetupAttachment(MeshComp, TEXT("L_Booster"));
	R_Booster->SetupAttachment(MeshComp, TEXT("R_Booster"));

	L_RedLight = CreateDefaultSubobject<UNiagaraComponent>(TEXT("L_RedLight"));
	R_RedLight = CreateDefaultSubobject<UNiagaraComponent>(TEXT("R_RedLight"));
	L_RedLight->SetupAttachment(MeshComp, TEXT("L_RedLight"));
	R_RedLight->SetupAttachment(MeshComp, TEXT("R_RedLight"));

	AirResistance = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AirResistance"));
	AirResistance->SetupAttachment(MeshComp, TEXT("AirResistance"));

	L_DriftSpark = CreateDefaultSubobject<UNiagaraComponent>(TEXT("L_DriftSpark"));
	R_DriftSpark = CreateDefaultSubobject<UNiagaraComponent>(TEXT("R_DriftSpark"));
	L_DriftSpark->SetupAttachment(MeshComp, TEXT("L_DriftSpark"));
	R_DriftSpark->SetupAttachment(MeshComp, TEXT("R_DriftSpark"));

	Collision_VFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Collision_VFX"));
	Collision_VFX->SetupAttachment(MeshComp, TEXT("Collision_VFX"));
	
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
	LastPositionData = {GetActorLocation(), Forward, FVector::UpVector};

	GameMode = GetWorld()->GetAuthGameMode<ARacingGameMode>();
	if (GameMode)
	{
		GameMode->UI()->ShowWidget(GetWorld(), EWidgetType::BoosterUI);
		GameMode->UI()->ShowWidget(GetWorld(), EWidgetType::SpeedometerUI);
	}

	for (TActorIterator<AResetPoint> It(GetWorld()); It; ++It)
	{
		AResetPoint* Point = *It; 
		if (Point->ActorHasTag("Start"))
		{
			ResetPoint = Point;
			break; 
		}
	}
}

void AMain_Player::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Velocity = ChassisComp->CalculateVelocity(true) / (GameMode->IsRaceEnd() ? 2.5f : 1.0f);
	float CorrectedVelocity = Velocity / (HandleInputStack.Num() && ChassisComp->IsFullDrift() && ChassisComp->IsBoost() ? 15.0f : 1.0f);
	
	FQuat Q = ChassisComp->CalculateHandleQuat();
	float Dir = FMath::Sign(Forward.Cross(Q.RotateVector(Forward)).Z); 
	float QuatCorrectedRate = 1.0f + (FMath::Max(FMath::Abs(Velocity * 0.036f), 45.0f) - 45.0f) / 235.0f * (HandleInputStack.Num() && ChassisComp->IsFullDrift() ? 10.0f : 1.0f);
	FQuat CorrectedQ = FQuat(FVector(0, 0, 1), Q.GetAngle() * Dir * QuatCorrectedRate);
	
	FVector RotatedForward = (ChassisComp->IsDrift() && !ChassisComp->IsFullDrift() || ChassisComp->IsRemainDrift() ? Q : CorrectedQ).RotateVector(Forward);
	CentrifugalForceDir = FMath::Lerp(CentrifugalForceDir, FVector(0), 0.25f);  
	FPositionData CurPositionData = {GetActorLocation(), Forward, Right * -ChassisComp->GetDriftDir()}; 
	if (Utility::Between_EE(CorrectedVelocity * 0.036f, 20.0f, 350.0f) && ChassisComp->IsDrift() && LastPositionData.Right.Dot(Right * -ChassisComp->GetDriftDir()) > 0.0f && Dir == ChassisComp->GetDriftDir())
	{
		CentrifugalForceDir = Right * -ChassisComp->GetDriftDir(); 
		CurPositionData.Right = CentrifugalForceDir; 
		float CentrifugalForce = Utility::CalculateCentrifugalForce(Velocity, LastPositionData, CurPositionData);
		CentrifugalForceDir *= CentrifugalForce * DeltaTime * (ChassisComp->IsFullDrift() ? 0.0f : ChassisComp->GetDriftAngleRate()); 
		//CentrifugalForceDir *= CentrifugalForce * DeltaTime * ChassisComp->GetDriftAngleRate() * (HandleInputStack.Num() && ChassisComp->IsFullDrift() ? 0.0f : 1.0f); 
	} 
	LastPositionData = CurPositionData; 
	SetActorLocation(GetActorLocation() + RotatedForward * Velocity * DeltaTime + CentrifugalForceDir, true);
	
	if (InelasticForce.Length() > 0.0f) 
	{ 
		InelasticForce = FMath::Lerp(InelasticForce, FVector(0), 0.05f);
		
	} 
	SetActorLocation(GetActorLocation() + InelasticForce * DeltaTime * 6.0f); 
	
	FHitResult GroundHit;  
	FHitResult RunwayHit;  
	FVector Start = GetActorLocation() + FVector(0, 0, -1) * 40.0f; 
	FVector End = Start + FVector(0, 0, -1) * 30.0f; 
	FCollisionQueryParams CQP; 
	CQP.AddIgnoredActor(this); 
	bool GroundRet = GetWorld()->LineTraceSingleByChannel(GroundHit, Start, End, ECollisionChannel::ECC_GameTraceChannel5, CQP); 
	bool GroundCheck = GroundRet && GroundHit.Distance < 25.0f; 
	//DrawDebugLine(GetWorld(), Start, End, GroundCheck ? FColor::Blue : FColor::Red, false, 10000.0f); 
	Start = GetActorLocation() + Forward * 150.0f + FVector(0, 0, -1) * 20.0f; 
	End = Start + Forward * 100.0f; 
	bool RunwayRet = GetWorld()->LineTraceSingleByChannel(RunwayHit, Start, End, ECollisionChannel::ECC_GameTraceChannel5, CQP); 
	//DrawDebugLine(GetWorld(), Start, End, RunwayRet ? FColor::Blue : FColor::Red, false, 10000.0f); 
	//if (RunwayRet) DrawDebugLine(GetWorld(), RunwayHit.ImpactPoint, RunwayHit.ImpactPoint + RunwayHit.ImpactNormal * 500.0f,  FColor::Black, false, 10000.0f); 
	SetActorLocation(GetActorLocation() + (GroundCheck ? FVector(0) : FVector(0, 0, -1) * GravityForce * DeltaTime)); 
	
	float MeshAngle = Q.GetAngle() * Dir / DeltaTime;
	FQuat MeshQ(FVector(0, 0, 1), MeshAngle);
	FRotator MeshRot = MeshQ.RotateVector(Forward).Rotation(); 
	if (RunwayRet || GroundCheck)
	{ 
		PitchAngle = FMath::Acos(FVector(0, 0, 1).Dot((RunwayRet ? RunwayHit : GroundHit).ImpactNormal)); 
		float PitchAngleDir = FMath::Sign(Forward.GetSafeNormal2D().Dot((RunwayRet ? RunwayHit : GroundHit).ImpactNormal));
		MeshRot.Pitch = FMath::RadiansToDegrees(PitchAngle * -PitchAngleDir); 
		FVector X = (RunwayRet ? RunwayHit : GroundHit).ImpactNormal; 
		if (GroundCheck)
        {
			SetActorLocation(GroundHit.ImpactPoint + X * 50.0f); 
		}
		
		//GEngine->AddOnScreenDebugMessage(-1, 1000.0f, FColor::Red, FString::Printf(TEXT("%f, %f"), PitchAngle, PitchAngleDir)); 
	}
	else 
	{
		PitchAngle = FMath::Lerp(PitchAngle, 0.0f, DeltaTime * 6.0f); 
		//PitchAngle = FMath::Abs(Forward.Z) ? (Forward.Z > 0 ? -5.0f : 5.0f) : 0.0f; 
		//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("%f"), PitchAngle)); 
	}
	FVector v = MeshRot.Vector();
	FVector Axis = FQuat(FVector(0, 0, 1), ChassisComp->GetSuspensionAxisAngle()).RotateVector(Forward);
	FQuat SuspensionQ = ChassisComp->CalculateSuspensionQuat(Axis); 
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("%f"), SuspensionQ.GetAngle())); 
	//SetActorRotation(MeshRot); 
	FVector x = SuspensionQ.RotateVector(Forward); 
	SetActorRotation(SuspensionQ.RotateVector(v).Rotation()); 
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + x * 500.0f, FColor::Blue, false, 10000.0f);
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + Axis * 500.0f, FColor::Red, false, 10000.0f);
	
	if (!ChassisComp->IsFullDrift())
	{ 
		CameraRot = MeshQ.UnrotateVector(SuspensionQ.UnrotateVector(FVector(1.0f, 0.0f, 0.0f))).Rotation();
		if (ChassisComp->IsRemainDrift())
		{ 
			CameraRot = FMath::Lerp(SpringArmComp->GetRelativeRotation(), FRotator(0), 0.0065f * (ChassisComp->IsBoost() ? 1.5f : 1.0f)); 
		} 
		else if (!HandleInputStack.Num() || HandleInputStack.Top() != Dir * FMath::Sign(Velocity))
		{ 
			CameraRot = FMath::Lerp(SpringArmComp->GetRelativeRotation(), FRotator(0), 0.025f); 
		} 
		else if ((HandleInputStack.Num() || ChassisComp->IsDrift()) && FMath::Abs(CameraRot.Yaw - FMath::RadiansToDegrees(MeshAngle)) > 0.25f)
		{ 
			CameraRot = FMath::Lerp(SpringArmComp->GetRelativeRotation(), CameraRot, (ChassisComp->IsFullDrift() ? 1.0f : 0.1f)); 
		}
		//CameraRot = SuspensionQ.RotateVector(CameraRot.Vector()).Rotation(); 
		AimPoint->SetRelativeLocation(CameraRot.Vector() * 200.0f);
		SpringArmComp->SetRelativeRotation(CameraRot); 
		SpringArmComp->TargetArmLength = FMath::Lerp(SpringArmComp->TargetArmLength, ChassisComp->IsBoost() ? 800.0f : 650.0f, 0.025f);
	} 
	Forward = FQuat(Right, PitchAngle).UnrotateVector(RotatedForward.GetSafeNormal2D());
	if (FMath::Abs(Forward.Z) < 0.1f)
	{ 
		Forward.Z = 0.0f; 
	}
 	Right = FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(90.0f)).RotateVector(Forward.GetSafeNormal2D()); 

	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("%f, %f"), MeshQ.UnrotateVector(FVector(1.0f, 0.0f, 0.0f)).Rotation().Yaw, CameraRot.Yaw)); 
	
	auto* SpeedometerUI = GameMode->UI()->GetWidget<USpeedometerUI>(GetWorld(), EWidgetType::SpeedometerUI);
	SpeedometerUI->UpdateSpeed(Velocity * 0.036f);

	if (ChassisComp->IsDrift()) 
	{
		L_SkidMark->Activate();
		R_SkidMark->Activate();

		L_DriftSpark->Activate();
		R_DriftSpark->Activate();
	}
	else
	{ 
		L_SkidMark->Deactivate();
		R_SkidMark->Deactivate(); 

		L_DriftSpark->Deactivate();
		R_DriftSpark->Deactivate();
	}

	if (ChassisComp->IsBoost())
	{
		L_Booster->Activate(); 
		R_Booster->Activate(); 

		L_RedLight->Activate(); 
		R_RedLight->Activate();

		AirResistance->Activate(); 
	} 
	else
	{ 
		L_Booster->Deactivate(); 
		R_Booster->Deactivate();
		
		L_RedLight->Deactivate(); 
		R_RedLight->Deactivate();
		
		AirResistance->Deactivate(); 
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

		IMC->BindAction(IA[int(EInputAction::Reset)], ETriggerEvent::Started, this, &AMain_Player::OnActionReset); 
	}
}

void AMain_Player::SetResetPoint(AResetPoint* NewResetPoint)
{
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("SetResetPoint Try")); 
	if (!ResetPoint || NewResetPoint->Idx == ResetPoint->Idx + 1 || NewResetPoint->Idx == ResetPoint->Idx - 1)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("SetResetPoint Success")); 
		ResetPoint = NewResetPoint; 
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

void AMain_Player::OnActionReset(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Reset!!")); 
	SetActorLocationAndRotation(ResetPoint->GetActorLocation(), ResetPoint->GetActorRotation()); 
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

// void AMain_Player::MySetActorLocation(const FVector& NewLocation, bool bSweep)
// {
// 	if (NewLocation.X > 999999999.f || NewLocation.X < -999999999.0f ||
// 		NewLocation.Y > 999999999.f || NewLocation.Y < -999999999.0f ||
// 		NewLocation.Z > 999999999.f || NewLocation.Z < -999999999.0f)
// 	{
// 		UE_LOG(LogTemp, Display, TEXT("MySetActorLocation")); 
// 	}
// 	else 
// 	{
// 		double x = FMath::Clamp(NewLocation.X, -999999999.0f, 999999999.0f);
// 		double y = FMath::Clamp(NewLocation.Y, -999999999.0f, 999999999.0f);
// 		double z = FMath::Clamp(NewLocation.Z, -999999999.0f, 999999999.0f);
// 		FVector n(x, y, z);
// 		SetActorLocation(n, bSweep); 
// 	}
// } 

void AMain_Player::OnBoxCompHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{ 
	float Velocity = ChassisComp->CalculateVelocity(true); 
	FVector Dir = Utility::CalculateReflectionVector(Forward, Hit.ImpactNormal); 
	InelasticForce = Utility::CalculateInelasticCollision(Dir, Velocity); 
	ChassisComp->OnCollisionDetection();

	Collision_VFX->Deactivate(); 
	Collision_VFX->Activate();
	GetWorldTimerManager().ClearTimer(CollisionParticleTimerHandle); 
	GetWorldTimerManager().SetTimer(CollisionParticleTimerHandle, FTimerDelegate::CreateLambda([&]()
	{
		Collision_VFX->Deactivate();
		GetWorldTimerManager().ClearTimer(CollisionParticleTimerHandle); 
	}), 1.0f, false); 
}
