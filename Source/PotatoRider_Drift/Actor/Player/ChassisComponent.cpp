// Fill out your copyright notice in the Description page of Project Settings.


#include "ChassisComponent.h" 
#include "../../UI/BoosterUI.h"
#include "../../UI/CountDownUI.h"
#include "../RacingGameMode.h"
#include "../../UIManager.h"
#include "../../Utility.h" 
#include "Kismet/GameplayStatics.h"

UChassisComponent::UChassisComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UChassisComponent::BeginPlay()
{
	Super::BeginPlay(); 
	
	Steering.Correction_Min_Vel = Steering.Default_Min_Vel; 
	Steering.Correction_Max_Vel = Steering.Default_Max_Vel;

	Booster.Max_Additional_RPM = Booster.Default_Max_Additional_RPM;
	
	GameMode = GetWorld()->GetAuthGameMode<ARacingGameMode>(); 
	GameMode->UI()->GetWidget<UBoosterUI>(GetWorld(), EWidgetType::BoosterUI)->UpdateBooster(Booster.Count); 

	Drift.LastDriftDir = 1.0f; 
}

void UChassisComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle); 
	GetWorld()->GetTimerManager().ClearTimer(Drift.InertiaAngleTimerHandle); 
	GetWorld()->GetTimerManager().ClearTimer(Booster.BoosterTimerHandle); 
	GetWorld()->GetTimerManager().ClearTimer(Booster.MomentBoosterTimerHandle); 
	GetWorld()->GetTimerManager().ClearTimer(Booster.StartBoosterTimerHandle); 
}

void UChassisComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Deceleration();
	RevertHandle(); 
	RevertDrift();
	Boost(); 

	UpdateCameraAngleRate(); 
} 

void UChassisComponent::Accelerator(float Difference)
{ 
	float CountDownTime = GameMode->GetTimer(); 
	if (CountDownTime < 3.3f) 
	{ 
		return; 
	} 

	if (Difference)
	{ 
		if (!Engine.bPressedAccelerator)
		{ 
			Steering.Correction_Min_Vel = Steering.Deceleration_Max_Vel; 
			Steering.Correction_Max_Vel = Steering.Acceleration_Max_Vel;

			if (!Booster.bUsedStartBoost && Difference > 0.0f)
			{
				if (Utility::Between_EE(CountDownTime, 3.3f, 4.8f))
				{ 
					Booster.bStartBoost = true; 
					Booster.bUsedStartBoost = true; 
				 
					if (Utility::Between_EE(CountDownTime, 3.85f, 4.15f))
					{ 
						Booster.Max_Additional_RPM = Booster.PerfectStart_Max_Additional_RPM;
						GameMode->UI()->GetWidget<UCountDownUI>(GetWorld(), EWidgetType::CountDownUI)->ShowResult("Perfect"); 
					} 
					else
					{ 
						Booster.Max_Additional_RPM = Booster.GoodStart_Max_Additional_RPM; 
						GameMode->UI()->GetWidget<UCountDownUI>(GetWorld(), EWidgetType::CountDownUI)->ShowResult("Good"); 
					} 
	
					GetWorld()->GetTimerManager().ClearTimer(Booster.StartBoosterTimerHandle); 
					GetWorld()->GetTimerManager().SetTimer(Booster.StartBoosterTimerHandle, FTimerDelegate::CreateLambda([&]()
					{ 
						SetVelocityToEngineRPM(true);
						Booster.Additional_RPM = 0.0f; 
						Booster.bStartBoost = false; 
						Booster.Max_Additional_RPM = Booster.Default_Max_Additional_RPM; 
						GetWorld()->GetTimerManager().ClearTimer(Booster.StartBoosterTimerHandle); 
					}), 1.0f, false);

					PlaySound(ESoundType::StartBoost); 
				}
				else
				{
					GameMode->UI()->GetWidget<UCountDownUI>(GetWorld(), EWidgetType::CountDownUI)->ShowResult("Late"); 
				}
			}

			if (Booster.bMomentBoostTiming && !Booster.bUsedMomentBoost)
			{
				Booster.bMomentBoost = true;
				Booster.bUsedMomentBoost = true; 
				Booster.Max_Additional_RPM = Booster.Moment_Max_Additional_RPM;
				GetWorld()->GetTimerManager().ClearTimer(Booster.MomentBoosterTimerHandle); 
				GetWorld()->GetTimerManager().SetTimer(Booster.MomentBoosterTimerHandle, FTimerDelegate::CreateLambda([&]()
				{ 
					SetVelocityToEngineRPM(true); 
					Booster.Additional_RPM = 0.0f; 
					Booster.bMomentBoostTiming = false;
					Booster.bUsedMomentBoost = false; 
					Booster.bMomentBoost = false; 
					Booster.Max_Additional_RPM = Booster.Default_Max_Additional_RPM;
					GetWorld()->GetTimerManager().ClearTimer(Booster.MomentBoosterTimerHandle); 
				}), 0.5f, false);
				
				PlaySound(ESoundType::MomentBoost); 
			} 
		} 
		
		float Velocity = CalculateVelocity(true) * 0.036f;
		if (!IsDrift()) 
		{
			if (Steering.HandleAngle < 8.0f || CheckSteeringCorrection(Velocity) || FMath::Sign(Velocity) != FMath::Sign(Difference))
			{ 
				Engine.RPM = FMath::Lerp(Engine.RPM, (FMath::Sign(Difference) > 0 ? Engine.Default_Max_RPM : Engine.Reverse_Max_RPM), GetWorld()->GetDeltaSeconds() * 0.35f); 
			} 
		} 
		
		if (Steering.bPressedHandle && !Engine.bPressedAccelerator)
		{ 
			if (!CheckSteeringCorrection(Velocity))
			{
				SetVelocityToEngineRPM(false); 

				if (FMath::Sign(Velocity) == FMath::Sign(Difference))
				{
					Steering.TargetRotateDeceleration = (Velocity > 0 ? FMath::CeilToFloat(int(Velocity - 100.0f) * 0.375f) : Velocity - Steering.Deceleration_Max_Vel) * 27.78f;
				} 
			} 
		}
		
		Engine.bPressedAccelerator = true;  
	}
	else
	{
		Engine.bPressedAccelerator = false; 

		SetVelocityToEngineRPM(false); 

		Steering.Correction_Min_Vel = Steering.Default_Min_Vel; 
		Steering.Correction_Max_Vel = Steering.Default_Max_Vel;
		
		Booster.bBoost = false; 
		Booster.bMomentBoost = false; 
		Booster.bStartBoost = false; 
		Booster.Max_Additional_RPM = Booster.Default_Max_Additional_RPM; 
		GetWorld()->GetTimerManager().ClearTimer(Booster.BoosterTimerHandle); 
		GetWorld()->GetTimerManager().ClearTimer(Booster.MomentBoosterTimerHandle); 
		GetWorld()->GetTimerManager().ClearTimer(Booster.StartBoosterTimerHandle); 
	} 
} 

void UChassisComponent::Handle(float Dir) 
{ 
	float Velocity = CalculateVelocity(true) * 0.036f;
	if (Dir)
	{ 
		if (Drift.bPressedDrift && !Drift.bDrift && (!Drift.bUsedDrift || Drift.LastDriftDir != Dir))
		{
			Drift.bDrift = true; 
			Drift.bUsedDrift = true; 
			Drift.LastDriftDir = Dir; 
			Drift.bRemainCentrifugalForce = false; 
			Drift.bDoubleDrift = Steering.HandleAngle > Drift.Max_DriftAngle - 10.0f && FMath::Sign(Steering.HandleAngle) == Dir; 

			if (Drift.bConnectDrift) 
			{ 
				Drift.bIsConnectDrift = true; 
			} 
			Drift.bConnectDrift = false; 
			
			GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle); 
			GetWorld()->GetTimerManager().SetTimer(Drift.DriftTimerHandle, FTimerDelegate::CreateLambda([&]()
			{ 
				Drift.bDrift = false;
				Drift.bRemainCentrifugalForce = true; 
				
				GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle); 
				GetWorld()->GetTimerManager().SetTimer(Drift.DriftTimerHandle, FTimerDelegate::CreateLambda([&]()
				{
					Drift.bRemainCentrifugalForce = false; 
					Drift.bDoubleDrift = false; 
					SetVelocityToEngineRPM(false);

					GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle); 
				}), 0.5f, false); 
			}), 1.0f + Velocity / 200.0f, false); 

			PlaySound(ESoundType::Drift); 
		} 
		
		if (Engine.bPressedAccelerator)
		{ 
			if (!Steering.bPressedHandle && !CheckSteeringCorrection(Velocity))
			{
				Steering.TargetRotateDeceleration = (Velocity > 0 ? 
					FMath::CeilToFloat(int(Velocity - 100.0f) * 0.375f) : 
					Velocity - Steering.Deceleration_Max_Vel) * 27.78f; 

				if (Booster.bBoost) 
				{ 
					float Rate = 1.0f - (FMath::Max(Engine.Default_Max_RPM, Engine.RPM + Booster.Additional_RPM + Drift.Deceleration_RPM) - Engine.Default_Max_RPM) / 
						(Booster.Normal_Max_Additional_RPM - Engine.Default_Max_RPM); 
					Steering.TargetRotateDeceleration *= Rate * 0.9f + 0.1f; 
				}

				if (Drift.bDoubleDrift)
				{
					Steering.TargetRotateDeceleration = 0.0f;
					Steering.RotateDeceleration = 0.0f; 
				}
			} 

			if (Steering.TargetRotateDeceleration && Steering.HandleAngle > 8.0f)
			{
				Steering.RotateDeceleration = FMath::Lerp(Steering.RotateDeceleration, Steering.TargetRotateDeceleration, GetWorld()->GetDeltaSeconds()); 
			} 
		} 

		if (FMath::Abs(Velocity) > 3.0f)
		{ 
			float RotateHandleRate = (Drift.bDrift ? (IsBoost() ? 0.8f : 0.85f) : (Drift.bRemainCentrifugalForce ? (Drift.LastDriftDir == Dir ? 0.15f : 0.5f) : (IsBoost() ? 0.5f : 0.65f))); 
			Steering.HandleAngle += Dir * RotateHandleRate * GetWorld()->GetDeltaSeconds() * 120.0f;
			Steering.bPressedHandle = FMath::Abs(Steering.HandleAngle) > 8.0f; 
		}
		
		float MaxAngle = GetMaxAngle();
		if (IsDrift())  
		{ 
			Drift.DriftAngle = Drift.Max_DriftAngle - MaxAngle; 
		}  
		MaxAngle += Drift.DriftAngle; 
		Steering.HandleAngle = FMath::Clamp(Steering.HandleAngle, -MaxAngle, MaxAngle); 
	} 
	else
	{ 
		Steering.bPressedHandle = false;
		Drift.bUsedDrift = false; 
		
		if (Steering.RotateDeceleration) 
		{ 
			SetVelocityToEngineRPM(false); 
		} 

		if (Drift.bDrift)
		{
			Drift.bDrift = false;
			Drift.bRemainCentrifugalForce = true;
			Drift.InertiaAngle = -1.0f;
			GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle);
			GetWorld()->GetTimerManager().ClearTimer(Drift.InertiaAngleTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(Drift.InertiaAngleTimerHandle, FTimerDelegate::CreateLambda([&]()
			{
				Drift.InertiaAngle = 1.0f; 
				
				GetWorld()->GetTimerManager().ClearTimer(Drift.InertiaAngleTimerHandle);
			}), 0.5f, false); 
		} 

		if (Booster.bRotatedBoost) 
		{ 
			Booster.bRotatedBoost = false; 
			Booster.Max_Additional_RPM = Booster.Normal_Max_Additional_RPM; 
		} 
	}
}

void UChassisComponent::DriftDevice(bool bPressed)
{ 
	Drift.bPressedDrift = bPressed; 

	if (!bPressed) 
	{
		if (Drift.bDrift)
		{ 
			Drift.bDrift = false;
			Drift.bRemainCentrifugalForce = true;
			Drift.InertiaAngle = -1.0f;
			GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle);
			GetWorld()->GetTimerManager().ClearTimer(Drift.InertiaAngleTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(Drift.InertiaAngleTimerHandle, FTimerDelegate::CreateLambda([&]()
			{
				Drift.InertiaAngle = 1.0f; 
				
				GetWorld()->GetTimerManager().ClearTimer(Drift.InertiaAngleTimerHandle);
			}), 0.5f, false); 
		} 

		Drift.bUsedDrift = false; 
	} 
} 

void UChassisComponent::BoosterDevice() 
{
	float Velocity = CalculateVelocity(true) * 0.036f;
	if (Booster.Count && !Booster.bBoost && Velocity > 0.0f)  
	{
		Booster.Count--;
		Booster.bBoost = true; 
		Booster.bRotatedBoost = Steering.bPressedHandle; 
		Booster.Max_Additional_RPM = Booster.bRotatedBoost ? Booster.Rotated_Normal_Max_Additional_RPM : Booster.Normal_Max_Additional_RPM; 
		GameMode->UI()->GetWidget<UBoosterUI>(GetWorld(), EWidgetType::BoosterUI)->UpdateBooster(Booster.Count);

		GetWorld()->GetTimerManager().ClearTimer(Booster.BoosterTimerHandle); 
		GetWorld()->GetTimerManager().ClearTimer(Booster.MomentBoosterTimerHandle); 
		GetWorld()->GetTimerManager().ClearTimer(Booster.StartBoosterTimerHandle); 
		GetWorld()->GetTimerManager().SetTimer(Booster.BoosterTimerHandle, FTimerDelegate::CreateLambda([&]()
		{ 
			Booster.bBoost = false; 
			Booster.bRotatedBoost = false; 
			Booster.Max_Additional_RPM = Booster.Default_Max_Additional_RPM; 
			GetWorld()->GetTimerManager().ClearTimer(Booster.BoosterTimerHandle); 
		}), 5.0f, false); 

		PlaySound(ESoundType::NormalBoost); 
	} 
} 

float UChassisComponent::CalculateVelocity(bool bIncludeBooster)
{ 
	float NewRPM = (Engine.RPM + (bIncludeBooster ? Booster.Additional_RPM : 0.0f) + Drift.Deceleration_RPM); 
	float FinalRPM = NewRPM / (PowerTrain.GearRatioOfTransmission * PowerTrain.GearRatioOfFinalReductionGear);
	return Steering.TireCircumference * FinalRPM * 1.67f - Steering.RotateDeceleration; 
} 

FQuat UChassisComponent::CalculateHandleQuat()
{ 
	float Dir = FMath::Sign(CalculateVelocity(true) * 0.036f); 
	float Angle = FMath::DegreesToRadians(Steering.HandleAngle * GetWorld()->GetDeltaSeconds() * Dir); 
	return FQuat(FVector(0, 0, 1), Angle); 
}

FQuat UChassisComponent::CalculateSuspensionQuat(FVector Axis)
{
	float DeltaTime = GetWorld()->GetDeltaSeconds() * 120.0f; 
	float Dir = FMath::Sign(CalculateVelocity(true) * 0.036f); 
	float Angle = FMath::DegreesToRadians(DeltaTime * 3.0f * (1.0 - GetDriftAngleRate()) * FMath::Sign(Steering.HandleAngle) * Dir); 
	return FQuat(Axis, Angle); 
}

bool UChassisComponent::IsDrift()
{
	return Drift.bDrift || Drift.bRemainCentrifugalForce; 
}

bool UChassisComponent::IsRemainDrift()
{
	return Drift.bRemainCentrifugalForce;
}

float UChassisComponent::GetDriftAngleRate() 
{ 
	return (1.0f - FMath::Abs(Steering.HandleAngle / Drift.Max_DriftAngle));
} 

float UChassisComponent::GetDriftDir()
{ 
	return Drift.LastDriftDir; 
}

bool UChassisComponent::IsBreakDrift()
{
	return Drift.bBreakDrift;
}

bool UChassisComponent::IsFullDrift()
{
	return FMath::Abs(Steering.HandleAngle) >= Drift.Max_DriftAngle - 1.0f; 
} 

bool UChassisComponent::IsBoost()
{ 
	return Booster.bBoost || Booster.bMomentBoost || Booster.bStartBoost;
}

float UChassisComponent::GetSuspensionAxisAngle()
{
	return FMath::Atan(Suspension.OverallLength / Suspension.OverallWidth) * FMath::Sign(Steering.HandleAngle); 
}

bool UChassisComponent::IsRedLight()
{
	return Booster.bStartBoost || Booster.bBoost; 
}

void UChassisComponent::Deceleration()
{
	if (!Engine.bPressedAccelerator && Engine.RPM) 
	{ 
		float Velocity = CalculateVelocity(true) * 0.036f;
		if (Steering.bPressedHandle && Utility::Between_EI(Velocity, 3.0f, Steering.Default_Max_Vel))
		{
			Engine.RPM += 0.5f; 
		}
		else 
		{ 
			Engine.RPM = FMath::Lerp(Engine.RPM, 0.0f, GetWorld()->GetDeltaSeconds() * 0.18f); 
		} 
	}
}

void UChassisComponent::RevertHandle()
{
	if (!Steering.bPressedHandle)
	{
		if (Steering.HandleAngle) 
		{
			float DeltaTime = GetWorld()->GetDeltaSeconds(); 
			Steering.HandleAngle = FMath::Lerp(Steering.HandleAngle, 0.0f, DeltaTime * (Drift.bRemainCentrifugalForce ? 0.6f * Drift.InertiaAngle : 1.8f)); 
		} 
	} 
	else if (Drift.bRemainCentrifugalForce && Drift.DriftAngle < 8.0f) 
	{ 
		Drift.bRemainCentrifugalForce = false; 
		Drift.bDoubleDrift = false; 
		SetVelocityToEngineRPM(false); 
	}
}

void UChassisComponent::RevertDrift()
{ 
	float Velocity = CalculateVelocity(true) * 0.036f; 
	
	bool bCondition1 = FMath::Abs(Velocity) < 35.0f && Engine.bPressedAccelerator; 
	bool bCondition2 = FMath::Abs(Velocity) < 2.0f; 
	if (IsDrift())
	{ 
		if (bCondition1 || bCondition2)
		{ 
			GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle);
			if (bCondition1) 
			{
				Drift.Deceleration_RPM += 500.0f * (Velocity > 0.0f ? -1.0f : 1.0f);
			}
			SetVelocityToEngineRPM(false); 

			Drift.bDrift = false;
			Drift.bRemainCentrifugalForce = false;
			Drift.bBreakDrift = true; 
			Drift.bDoubleDrift = false; 
			Drift.bIsConnectDrift = false; 
			Drift.bConnectDrift = true; 
			GetWorld()->GetTimerManager().SetTimer(Drift.DriftTimerHandle, FTimerDelegate::CreateLambda([&]()
			{ 
				Drift.bConnectDrift = false; 

				GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle); 
			}), 0.5f, false); 
		} 
	} 
	
	float DeltaTime = GetWorld()->GetDeltaSeconds(); 
	if (!Drift.bDrift) 
	{ 
		Drift.DriftAngle = FMath::Lerp(Drift.DriftAngle, 0.0f, DeltaTime * (Steering.HandleAngle ? 4.2f : 1.2f)); 

		if (Drift.bRemainCentrifugalForce) 
		{ 
			Booster.BoosterGauge += DeltaTime * 120.0f * FMath::Min(0.35f, FMath::Abs(Velocity) / 450.0f) * (Drift.bIsConnectDrift ? 1.5f : 1.0f);
			Booster.BoosterGauge = FMath::Min(Booster.BoosterGauge, Booster.Max_BoosterGauge); 

			float DriftAngle = FMath::Clamp(Steering.HandleAngle - 55.0f, -12.0f, 12.0f) * 0.1f; 
			Drift.Deceleration_RPM -= DeltaTime * 120.0f * 7.5f * (Engine.RPM + Booster.Additional_RPM + Drift.Deceleration_RPM > 0 ? 1.0f : -1.0f) * 
				(DriftAngle > 0.0f ? FMath::Max(0.0f, DriftAngle - 0.2f) : FMath::Min(0.0f, DriftAngle + 0.2f)); 
		} 
		else if (!IsBoost() && Velocity > 100.0f) 
		{ 
			Booster.BoosterGauge += DeltaTime * 9.0f; 
			Booster.BoosterGauge = FMath::Min(Booster.BoosterGauge, Booster.Max_BoosterGauge); 

			if (Booster.BoosterGauge >= Booster.Max_BoosterGauge) {
				Booster.BoosterGauge = 0.0f; 
				Booster.Count = FMath::Min(2, Booster.Count + 1);
				GameMode->UI()->GetWidget<UBoosterUI>(GetWorld(), EWidgetType::BoosterUI)->UpdateBooster(Booster.Count); 
			}
		}
		
		float HandleAngle = FMath::Abs(Steering.HandleAngle); 
		if (Drift.bRemainCentrifugalForce)
		{
			if (HandleAngle <= Steering.EndDriftAngle) 
			{ 
				Drift.bRemainCentrifugalForce = false;
				Drift.bDoubleDrift = false; 
				SetVelocityToEngineRPM(false); 

				Drift.bIsConnectDrift = false; 
				Drift.bConnectDrift = true;
				GetWorld()->GetTimerManager().SetTimer(Drift.DriftTimerHandle, FTimerDelegate::CreateLambda([&]()
				{
					Drift.bConnectDrift = false; 

					GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle);
				}), 0.5f, false);

				if (Booster.BoosterGauge >= Booster.Max_BoosterGauge)
				{ 
					Booster.BoosterGauge = 0.0f; 
					Booster.Count = FMath::Min(2, Booster.Count + 1);
					GameMode->UI()->GetWidget<UBoosterUI>(GetWorld(), EWidgetType::BoosterUI)->UpdateBooster(Booster.Count); 
				}
			}
		}  

		if (HandleAngle <= Steering.Max_Angle + 8.0f)
		{
			if (!Booster.bMomentBoostTiming && Drift.bRemainCentrifugalForce) 
			{ 
				Booster.bMomentBoostTiming = true; 
				GetWorld()->GetTimerManager().ClearTimer(Booster.MomentBoosterTimerHandle); 
				GetWorld()->GetTimerManager().SetTimer(Booster.MomentBoosterTimerHandle, FTimerDelegate::CreateLambda([&]()
				{ 
					Booster.bMomentBoostTiming = false; 
					GetWorld()->GetTimerManager().ClearTimer(Booster.MomentBoosterTimerHandle);
				}), 1.5f, false);
			} 
		} 
		else if (!Booster.bMomentBoost) 
		{ 
			Booster.bMomentBoostTiming = false; 
			GetWorld()->GetTimerManager().ClearTimer(Booster.MomentBoosterTimerHandle); 
		}
	} 
	else 
	{ 
		Booster.BoosterGauge += DeltaTime * 120.0f * FMath::Min(0.35f, FMath::Abs(Velocity) / 450.0f) * (Drift.bIsConnectDrift ? 1.5f : 1.0f) * (IsBoost() ? 2.5f : 1.0f); 
		Booster.BoosterGauge = FMath::Min(Booster.BoosterGauge, Booster.Max_BoosterGauge); 

		if (FMath::Abs(Steering.HandleAngle) > 8.0f && FMath::Sign(Steering.HandleAngle) == Drift.LastDriftDir)
		{
			Drift.Deceleration_RPM -= DeltaTime * 120.0f * (Drift.bDoubleDrift ? 5.0f : 15.0f) * (Engine.RPM + Booster.Additional_RPM + Drift.Deceleration_RPM > 0 ? 1.0f : -1.0f) * (IsBoost() ? 2.0f : 1.0f);
		} 
	} 

	auto* BoosterUI = GameMode->UI()->GetWidget<UBoosterUI>(GetWorld(), EWidgetType::BoosterUI);
	BoosterUI->UpdateBoosterGauge(Booster.BoosterGauge / Booster.Max_BoosterGauge); 
} 

void UChassisComponent::Boost()
{ 
	float Max_Additional = 0.0f; 
	if (Booster.bBoost) 
	{ 
		Max_Additional = Booster.Max_Additional_RPM - (!Booster.bRotatedBoost ? Engine.RPM : 0.0f); 
	} 
	else if (Booster.bMomentBoost || Booster.bStartBoost) 
	{ 
		Max_Additional = Booster.Max_Additional_RPM; 
	} 

	Booster.Additional_RPM = FMath::Lerp(Booster.Additional_RPM, Max_Additional, GetWorld()->GetDeltaSeconds() * (Booster.bMomentBoost || Booster.bStartBoost ? 6.0f : 1.2f));
}

void UChassisComponent::SetVelocityToEngineRPM(bool bIncludeBooster)
{
	float CurVelocity = CalculateVelocity(bIncludeBooster);
	CurVelocity = CurVelocity / (Steering.TireCircumference * 1.67f); 
	Engine.RPM = CurVelocity * PowerTrain.GearRatioOfTransmission * PowerTrain.GearRatioOfFinalReductionGear; 
	Steering.RotateDeceleration = 0.0f;
	Steering.TargetRotateDeceleration = 0.0f;
	Drift.Deceleration_RPM = 0.0f; 
} 

void UChassisComponent::UpdateCameraAngleRate()
{
	float Velocity = CalculateVelocity(true) * 0.036f;
	
	if (Velocity >= 40.0f) 
	{ 
		Steering.CameraAngleRate = 1.0f - (FMath::Clamp(Velocity, 40.0f, 240.0f) - 40.0f) / 280.0f; 
	} 
	else if (Velocity >= 0.0f)
	{ 
		Steering.CameraAngleRate = FMath::Clamp(Velocity, 0.0f, 40.0f) / 40.0f; 
	} 
	else 
	{
		Steering.CameraAngleRate = FMath::Clamp(Velocity, -30.0f, 0.0f) / -30.0f; 
	}
} 

bool UChassisComponent::CheckSteeringCorrection(float CurVelocity)
{
	return Utility::Between_EI(CurVelocity, Steering.Correction_Min_Vel, Steering.Correction_Max_Vel); 
} 

void UChassisComponent::ResetHandleForce()
{
	Steering.HandleAngle = 0.0f; 
}

void UChassisComponent::BreakDrift()
{ 
	Drift.bBreakDrift = false; 
	Drift.DriftAngle = 0.0f; 
	Steering.HandleAngle = 0.0f; 
}

void UChassisComponent::OnCollisionDetection()
{
	Engine.RPM  *= 0.35f;
	PlaySound(ESoundType::Collision); 
}

float UChassisComponent::GetMaxAngle()
{ 
	return Steering.Max_Angle * Steering.CameraAngleRate; 
}

void UChassisComponent::PlaySound(ESoundType Type)
{
	UGameplayStatics::PlaySound2D(GetWorld(), Sound[int(Type)]); 
}
