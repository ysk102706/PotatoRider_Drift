// Fill out your copyright notice in the Description page of Project Settings.


#include "ChassisComponent.h" 
#include "BoosterUI.h"
#include "CountDownUI.h"
#include "RacingGameMode.h"
#include "UIManager.h"
#include "Utility.h" 
#include "Chaos/Deformable/Utilities.h"

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

void UChassisComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Deceleration();
	RevertHandle(); 
	RevertDrift();
	Boost(); 

	UpdateCameraAngleRate(); 

	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("%f"), Booster.Additional_RPM));
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("%f, %f"), Engine.RPM, Booster.Additional_RPM)); 
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
			
			if (Utility::Between_EE(CountDownTime, 3.3f, 4.8f))
			{ 
				Booster.bStartBoost = true; 
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
				}), 1.5f, false); 
			}
			else
			{
				GameMode->UI()->GetWidget<UCountDownUI>(GetWorld(), EWidgetType::CountDownUI)->ShowResult("Late"); 
			}

			if (Booster.bMomentBoostTiming)
			{
				Booster.bMomentBoost = true; 
				Booster.Max_Additional_RPM = Booster.Moment_Max_Additional_RPM;
				GetWorld()->GetTimerManager().ClearTimer(Booster.MomentBoosterTimerHandle); 
				GetWorld()->GetTimerManager().SetTimer(Booster.MomentBoosterTimerHandle, FTimerDelegate::CreateLambda([&]()
				{ 
					SetVelocityToEngineRPM(true); 
					Booster.Additional_RPM = 0.0f; 
					Booster.bMomentBoost = false; 
					Booster.bMomentBoostTiming = false; 
					Booster.Max_Additional_RPM = Booster.Default_Max_Additional_RPM;
					GetWorld()->GetTimerManager().ClearTimer(Booster.MomentBoosterTimerHandle); 
				}), 0.3f, false); 
			} 
		} 
		
		float Velocity = CalculateVelocity(true) * 0.036f;
		if (!IsDrift()) 
		{
			if (!Steering.bPressedHandle || CheckSteeringCorrection(Velocity) || FMath::Sign(Velocity) != FMath::Sign(Difference))
			{ 
				float IncreaseRPMRate = 1.0f - (Engine.RPM > 2300.0f ? (Engine.RPM - 2300.0f) / 2200.0f : 0.0f); 
				Engine.RPM += Difference * 0.75f * IncreaseRPMRate; 
			} 
		} 
		Engine.RPM = FMath::Clamp(Engine.RPM, Engine.Reverse_Max_RPM, Engine.Default_Max_RPM);

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
			Drift.LastDriftAngle = 0.0f; 
			Drift.bDoubleDrift = Steering.HandleAngle > Drift.Max_DriftAngle - 15.0f && FMath::Sign(Steering.HandleAngle) == Dir; 

			if (Drift.bConnectDrift) 
			{ 
				Drift.bIsConnectDrift = true; 
			} 
			Drift.bConnectDrift = false; 
			
			GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle); 
			GetWorld()->GetTimerManager().SetTimer(Drift.DriftTimerHandle, FTimerDelegate::CreateLambda([&]()
			{ 
				Drift.bDrift = false;
				Drift.LastDriftAngle = Steering.HandleAngle - GetMaxAngle(); 
				Drift.bRemainCentrifugalForce = true; 
				
				GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle); 
			}), 1.0f + Velocity / 200.0f, false); 
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
			} 

			if (Steering.TargetRotateDeceleration)
			{
				Steering.RotateDeceleration += 13.89f * (Steering.TargetRotateDeceleration > 0 ? 1.0f : -1.0f);
				Steering.RotateDeceleration = Steering.TargetRotateDeceleration > 0 ?
					FMath::Clamp(Steering.RotateDeceleration, 0, Steering.TargetRotateDeceleration) : 
					FMath::Clamp(Steering.RotateDeceleration, Steering.TargetRotateDeceleration, 0); 
			} 
		} 

		if (FMath::Abs(Velocity) > 3.0f)
		{ 
			Steering.HandleAngle += Dir * (Drift.bDrift ? 1.75f : (Drift.bRemainCentrifugalForce ? (Drift.LastDriftDir == Dir ? 0.15f : 0.5f) : 1.0f)); 
			Steering.bPressedHandle = true;
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
			Drift.LastDriftAngle = Steering.HandleAngle - GetMaxAngle(); 
			GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle);
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
			Drift.LastDriftAngle = Steering.HandleAngle - GetMaxAngle(); 
			GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle); 
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

		GetWorld()->GetTimerManager().SetTimer(Booster.BoosterTimerHandle, FTimerDelegate::CreateLambda([&]()
		{ 
			Booster.bBoost = false; 
			Booster.bRotatedBoost = false; 
			Booster.Max_Additional_RPM = Booster.Default_Max_Additional_RPM; 
			GetWorld()->GetTimerManager().ClearTimer(Booster.BoosterTimerHandle); 
		}), 5.0f, false); 
	} 
} 

float UChassisComponent::CalculateVelocity(bool bIncludeBooster)
{ 
	float NewRPM = (Engine.RPM + (bIncludeBooster ? Booster.Additional_RPM : 0.0f) + Drift.Deceleration_RPM); 
	float FinalRPM = NewRPM / (PowerTrain.GearRatioOfTransmission * PowerTrain.GearRatioOfFinalReductionGear);
	return Steering.TireCircumference * FinalRPM * 1.67f - Steering.RotateDeceleration; 
} 

FQuat UChassisComponent::CalculateQuat()
{
	float Angle = 0.0f; 
	float Velocity = CalculateVelocity(true) * 0.036f;
	
	Angle = FMath::DegreesToRadians(Steering.HandleAngle * GetWorld()->GetDeltaSeconds() * (Velocity >= 0 ? 1.0f : -1.0f)); 
	
	return FQuat(FVector(0, 0, 1), Angle); 
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
	return Drift.LastDriftAngle ? FMath::Abs((FMath::Max(GetMaxAngle(), FMath::Abs(Steering.HandleAngle)) - GetMaxAngle()) / Drift.LastDriftAngle) : 1.0f;
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
			Engine.RPM = FMath::Lerp(Engine.RPM, 0.0f, 0.005f); 
		} 
	}
}

void UChassisComponent::RevertHandle()
{
	if (!Steering.bPressedHandle)
	{
		if (Steering.HandleAngle) 
		{ 
			Steering.HandleAngle = FMath::Lerp(Steering.HandleAngle, 0.0f, Drift.bRemainCentrifugalForce ? 0.005f : 0.035f); 
		} 
	} 
	else if (Drift.bRemainCentrifugalForce && Drift.DriftAngle < 5.0f) 
	{ 
		Drift.bRemainCentrifugalForce = false; 
		Drift.LastDriftAngle = 0.0f; 
		Drift.bDoubleDrift = false; 
		ApplyDriftDeceleration(); 
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
			ApplyDriftDeceleration(); 

			Drift.bDrift = false;
			Drift.bRemainCentrifugalForce = false;
			Drift.LastDriftAngle = 0.0f; 
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
	
	if (!Drift.bDrift) 
	{ 
		Drift.DriftAngle = FMath::Lerp(Drift.DriftAngle, 0.0f, Steering.HandleAngle ? 0.035f : 0.01f); 

		if (Drift.bRemainCentrifugalForce) 
		{ 
			float DriftAngle = FMath::Clamp(Steering.HandleAngle - 48.0f, -12.0f, 12.0f) * 0.1f; 
			Booster.BoosterGauge += FMath::Min(0.8f, FMath::Abs(Velocity) / 500.0f) * (Drift.bIsConnectDrift ? 1.5f : 1.0f);
			Booster.BoosterGauge = FMath::Min(Booster.BoosterGauge, Booster.Max_BoosterGauge); 
			Drift.Deceleration_RPM -= (Drift.bDoubleDrift ? 2.5f : 10.0f) * 
				(Engine.RPM + Booster.Additional_RPM + Drift.Deceleration_RPM > 0 ? 1.0f : -1.0f) * 
				(DriftAngle > 0.0f ? FMath::Max(0.0f, DriftAngle - 0.2f) : FMath::Min(0.0f, DriftAngle + 0.6f));
			float Max_Deceleration = (Drift.bDoubleDrift ? 0.0f : 60.0f); 
			Drift.Deceleration_RPM = FMath::Clamp(Drift.Deceleration_RPM, -Max_Deceleration, Max_Deceleration); 
		}
		else if (Velocity > 100.0f)
		{ 
			Booster.BoosterGauge += 0.05f; 
			Booster.BoosterGauge = FMath::Min(Booster.BoosterGauge, Booster.Max_BoosterGauge); 
		}
		
		float HandleAngle = FMath::Abs(Steering.HandleAngle); 
		if (Drift.bRemainCentrifugalForce) 
		{ 
			if (HandleAngle <= Steering.EndDriftAngle) 
			{ 
				Drift.bRemainCentrifugalForce = false;
				Drift.LastDriftAngle = 0.0f;
				Drift.bDoubleDrift = false; 
				ApplyDriftDeceleration(); 

				Drift.bIsConnectDrift = false; 
				Drift.bConnectDrift = true;
				GetWorld()->GetTimerManager().SetTimer(Drift.DriftTimerHandle, FTimerDelegate::CreateLambda([&]()
				{
					Drift.bConnectDrift = false; 

					GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle);
				}), 0.5f, false);
			} 
		} 

		if (HandleAngle <= Steering.EndDriftAngle + 2.0f) 
		{ 
			if (Booster.BoosterGauge >= Booster.Max_BoosterGauge) {
				Booster.BoosterGauge = 0.0f; 
				Booster.Count = FMath::Min(2, Booster.Count + 1);
				GameMode->UI()->GetWidget<UBoosterUI>(GetWorld(), EWidgetType::BoosterUI)->UpdateBooster(Booster.Count); 
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
		Booster.BoosterGauge += FMath::Min(0.8f, FMath::Abs(Velocity) / 500.0f) * (Drift.bIsConnectDrift ? 1.5f : 1.0f); 
		Booster.BoosterGauge = FMath::Min(Booster.BoosterGauge, Booster.Max_BoosterGauge); 

		Drift.Deceleration_RPM -= (Drift.bDoubleDrift ? 2.5f : 10.0f) * (Engine.RPM + Booster.Additional_RPM + Drift.Deceleration_RPM > 0 ? 1.0f : -1.0f); 
		float Max_Deceleration = (Drift.bDoubleDrift ? 0.0f : 800.0f); 
		Drift.Deceleration_RPM = FMath::Clamp(Drift.Deceleration_RPM, -Max_Deceleration, Max_Deceleration); 
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
	
	Booster.Additional_RPM = FMath::Lerp(Booster.Additional_RPM, Max_Additional, Booster.bMomentBoost || Booster.bStartBoost ? 0.075f : 0.01f);
}

void UChassisComponent::SetVelocityToEngineRPM(bool bIncludeBooster)
{
	float CurVelocity = CalculateVelocity(bIncludeBooster);
	CurVelocity = CurVelocity / (Steering.TireCircumference * 1.67f); 
	Engine.RPM = CurVelocity * PowerTrain.GearRatioOfTransmission * PowerTrain.GearRatioOfFinalReductionGear; 
	Steering.RotateDeceleration = 0.0f;
	Steering.TargetRotateDeceleration = 0.0f; 
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

void UChassisComponent::ApplyDriftDeceleration()
{ 
	if (Drift.Deceleration_RPM)
	{ 
		float NewRPM = Engine.RPM + Drift.Deceleration_RPM;
		Engine.RPM = Engine.RPM > 0.0f ? FMath::Max(0, NewRPM) : FMath::Min(0, NewRPM);
		Drift.Deceleration_RPM = 0.0f; 
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

float UChassisComponent::GetMaxAngle()
{ 
	return Steering.Max_Angle * Steering.CameraAngleRate; 
}
