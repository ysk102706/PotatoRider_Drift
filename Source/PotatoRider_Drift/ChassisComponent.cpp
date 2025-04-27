// Fill out your copyright notice in the Description page of Project Settings.


#include "ChassisComponent.h"

#include "Utility.h"

UChassisComponent::UChassisComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UChassisComponent::BeginPlay()
{
	Super::BeginPlay(); 

	Steering.Correction_Min_Vel = Steering.Default_Min_Vel; 
	Steering.Correction_Max_Vel = Steering.Default_Max_Vel; 
}

void UChassisComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Deceleration();
	RevertHandle(); 
	RevertDrift(); 

	UpdateCameraAngleRate(); 
	
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("%f"), Steering.HandleAngle));
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("%f"), Drift.DriftAngle));
}

void UChassisComponent::Accelerator(float Difference)
{
	if (Difference)
	{
		if (!Engine.bPressedAccelerator)
		{ 
			Steering.Correction_Min_Vel = Steering.Deceleration_Max_Vel; 
			Steering.Correction_Max_Vel = Steering.Acceleration_Max_Vel; 
		} 
		
		float Velocity = CalculateVelocity() * 0.036f; 
		if (!Steering.bPressedHandle || CheckSteeringCorrection(Velocity) || FMath::Sign(Velocity) != FMath::Sign(Difference))
		{
			Engine.RPM += Difference * 0.5f; 
			Engine.RPM = FMath::Clamp(Engine.RPM, Engine.Reverse_Max_RPM, Engine.Default_Max_RPM); 
		} 

		if (Steering.bPressedHandle && !Engine.bPressedAccelerator)
		{
			if (!CheckSteeringCorrection(Velocity))
			{
				SetVelocityToEngineRPM(); 

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

		SetVelocityToEngineRPM(); 

		Steering.Correction_Min_Vel = Steering.Default_Min_Vel; 
		Steering.Correction_Max_Vel = Steering.Default_Max_Vel; 
	} 
} 

void UChassisComponent::Handle(float Dir)
{ 
	float Velocity = CalculateVelocity() * 0.036f; 
	if (Dir)
	{ 
		if (Drift.bPressedDrift && !Drift.bDrift)
		{
			Drift.bDrift = true; 
			GetWorld()->GetTimerManager().SetTimer(Drift.DriftTimerHandle, FTimerDelegate::CreateLambda([&]()
			{
				Drift.bDrift = false; 
				
				GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle); 
			}), 1.0f + Velocity / 200.0f, false); 
		} 
		
		if (Engine.bPressedAccelerator)
		{
			if (!Steering.bPressedHandle && !CheckSteeringCorrection(Velocity))
			{
				Steering.TargetRotateDeceleration = (Velocity > 0 ? FMath::CeilToFloat(int(Velocity - 100.0f) * 0.375f) : Velocity - Steering.Deceleration_Max_Vel) * 27.78f;
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
			Steering.HandleAngle += Dir * (Drift.bDrift ? 3.0f : 1.0f);
			Steering.bPressedHandle = true;
		} 

		float MaxAngle = Steering.Max_Angle * Steering.CameraAngleRate; 
		if (Drift.bDrift) 
		{ 
			Drift.DriftAngle = 100.0f - MaxAngle; 
		} 
		
		MaxAngle += Drift.DriftAngle; 
		Steering.HandleAngle = FMath::Clamp(Steering.HandleAngle, -MaxAngle, MaxAngle);

		Steering.HoldTime = FMath::Lerp(Steering.HoldTime, 0.5f, 0.02f); 
	} 
	else
	{
		Steering.bPressedHandle = false;

		if (Steering.RotateDeceleration)
		{
			SetVelocityToEngineRPM(); 
		} 
	}
}

void UChassisComponent::DriftDevice(bool bPressed)
{ 
	Drift.bPressedDrift = bPressed; 

	if (!bPressed) 
	{ 
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Release")); 
		if (Drift.bDrift)
		{ 
			Drift.bDrift = false; 
			GetWorld()->GetTimerManager().ClearTimer(Drift.DriftTimerHandle); 
		}
	}
} 

float UChassisComponent::CalculateVelocity()
{ 
	float FinalRPM = Engine.RPM / (PowerTrain.GearRatioOfTransmission * PowerTrain.GearRatioOfFinalReductionGear);
	return Steering.TireCircumference * FinalRPM * 1.67f - Steering.RotateDeceleration; 
}

FQuat UChassisComponent::CalculateQuat()
{
	float Angle = 0.0f; 
	float Velocity = CalculateVelocity() * 0.036f;
	
	Angle = FMath::DegreesToRadians(Steering.HandleAngle * GetWorld()->GetDeltaSeconds() * (Velocity >= 0 ? 1 : -1)); 
	
	return FQuat(FVector(0, 0, 1), Angle); 
} 

void UChassisComponent::Deceleration()
{
	if (!Engine.bPressedAccelerator && Engine.RPM) 
	{ 
		float Velocity = CalculateVelocity() * 0.036f; 
		if (Steering.bPressedHandle && Utility::Between_EI(Velocity, 3.0f, Steering.Default_Max_Vel))
		{
			Engine.RPM += 0.5f; 
		}
		else
		{
			Engine.RPM += 10.0f * (Engine.RPM > 0 ? -1 : 1);
		} 
	}
}

void UChassisComponent::RevertHandle()
{
	if (!Steering.bPressedHandle)
	{
		if (Steering.HandleAngle)
		{
			Steering.HandleAngle += 1.0f * (Steering.HandleAngle > 0 ? -1 : 1); 
		} 
		
		Steering.HoldTime = FMath::Lerp(Steering.HoldTime, 0.0f, 0.02f); 
	}
}

void UChassisComponent::RevertDrift()
{ 
	if (!Drift.bDrift)
	{ 
		Drift.DriftAngle = FMath::Lerp(Drift.DriftAngle, 0.0f, 0.05f); 
	}
}

void UChassisComponent::SetVelocityToEngineRPM()
{
	float CurVelocity = CalculateVelocity();
	CurVelocity = CurVelocity / (Steering.TireCircumference * 1.67f); 
	Engine.RPM = CurVelocity * PowerTrain.GearRatioOfTransmission * PowerTrain.GearRatioOfFinalReductionGear; 
	Steering.RotateDeceleration = 0.0f;
	Steering.TargetRotateDeceleration = 0.0f; 
} 

void UChassisComponent::UpdateCameraAngleRate()
{
	float Velocity = CalculateVelocity() * 0.036f; 

	Steering.CameraAngleRate = 1.5;
	if (Velocity >= 15.0f) 
	{
		Steering.CameraAngleRate = 1.0f - (FMath::Clamp(Velocity, 30.0f, 170.0f) - 30.0f) / 140.0f; 
	} 
	else if (Velocity >= 0.0f)
	{ 
		Steering.CameraAngleRate = FMath::Clamp(Velocity, 0.0f, 15.0f) / 15.0f; 
	} 
	else
	{
		Steering.CameraAngleRate = FMath::Clamp(Velocity, -30.0f, 0.0f) / -30.0f * 1.5f; 
	}
} 

bool UChassisComponent::CheckSteeringCorrection(float CurVelocity)
{
	return Utility::Between_EI(CurVelocity, Steering.Correction_Min_Vel, Steering.Correction_Max_Vel); 
} 

float UChassisComponent::GetHandleHoldTime()
{
	return Steering.HoldTime; 
}
