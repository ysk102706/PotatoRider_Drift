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
			Engine.RPM += Difference * 0.75f; 
			Engine.RPM = FMath::Clamp(Engine.RPM, Engine.Reverse_Max_RPM, Engine.Default_Max_RPM); 
		} 

		if (Steering.bPressedHandle && !Engine.bPressedAccelerator)
		{
			if (!CheckSteeringCorrection(Velocity))
			{
				SetCurVelocityToEngineRPM(); 
				Steering.TargetRotateDeceleration = 0.0f; 

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

		Steering.Correction_Min_Vel = Steering.Default_Min_Vel; 
		Steering.Correction_Max_Vel = Steering.Default_Max_Vel; 
	}
} 

void UChassisComponent::RotateHandle(float Dir)
{
	if (Dir)
	{
		Steering.HandleAngle += Dir * 0.65f;
		Steering.HandleAngle = FMath::Clamp(Steering.HandleAngle, -Steering.Max_Angle, Steering.Max_Angle);
		
		float Velocity = CalculateVelocity() * 0.036f;
		if (Engine.bPressedAccelerator)
		{
			if (!Steering.bPressedHandle && !CheckSteeringCorrection(Velocity))
			{
				Steering.TargetRotateDeceleration = (Velocity > 0 ? FMath::CeilToFloat(int(Velocity - 100.0f) * 0.375f) : Velocity - Steering.Deceleration_Max_Vel) * 27.78f;
			} 

			if (Steering.TargetRotateDeceleration)
			{
				Steering.RotateDeceleration += 30.0f * Steering.TargetRotateDeceleration > 0 ? 1.0f : -1.0f;
				Steering.RotateDeceleration = Steering.TargetRotateDeceleration > 0 ?
					FMath::Clamp(Steering.RotateDeceleration, 0, Steering.TargetRotateDeceleration) : 
					FMath::Clamp(Steering.RotateDeceleration, Steering.TargetRotateDeceleration, 0);
			} 
		} 
		
		Steering.bPressedHandle = true; 
	} 
	else
	{
		Steering.bPressedHandle = false;

		if (Steering.RotateDeceleration)
		{
			SetCurVelocityToEngineRPM(); 
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
	
	if (FMath::Abs(Velocity) > 3.0f) 
	{
		Angle = FMath::DegreesToRadians(Steering.HandleAngle * GetWorld()->GetDeltaSeconds() * (Velocity >= 0 ? 1 : -1)); 
	}

	return FQuat(FVector(0, 0, 1), Angle); 
}

void UChassisComponent::Deceleration()
{
	if (!Engine.bPressedAccelerator && Engine.RPM) 
	{
		float Velocity = CalculateVelocity() * 0.036f; 
		if (Steering.bPressedHandle && Utility::Between_EI(Velocity, 3.0f, Steering.Default_Max_Vel))
		{
			Engine.RPM += 2.0f; 
		}
		else
		{
			Engine.RPM += 2.0f * (Engine.RPM > 0 ? -1 : 1);
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
	}
}

bool UChassisComponent::CheckSteeringCorrection(float CurVelocity)
{
	return Utility::Between_EI(CurVelocity, Steering.Correction_Min_Vel, Steering.Correction_Max_Vel); 
}

void UChassisComponent::SetCurVelocityToEngineRPM()
{
	float CurVelocity = CalculateVelocity();
	CurVelocity = CurVelocity / (Steering.TireCircumference * 1.67f); 
	Engine.RPM = CurVelocity * PowerTrain.GearRatioOfTransmission * PowerTrain.GearRatioOfFinalReductionGear; 
	Steering.RotateDeceleration = 0.0f;
	Steering.TargetRotateDeceleration = 0.0f; 
}
