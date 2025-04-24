// Fill out your copyright notice in the Description page of Project Settings.


#include "ChassisComponent.h"

UChassisComponent::UChassisComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UChassisComponent::BeginPlay()
{
	Super::BeginPlay(); 

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
		if (!Steering.bPressedHandle)
		{
			Engine.RPM += Difference * 0.5f;
		
			Engine.RPM = FMath::Clamp(Engine.RPM, Engine.Reverse_Max_RPM, Engine.Default_Max_RPM);
		} 

		if (Steering.bPressedHandle && !Engine.bPressedAccelerator)
		{
			float CurVelocity = CalculateVelocity();
			CurVelocity = CurVelocity / (Steering.TireCircumference * 1.67f); 
			Engine.RPM = CurVelocity * PowerTrain.GearRatioOfTransmission * PowerTrain.GearRatioOfFinalReductionGear; 
			Steering.RotateDeceleration = 0.0f;
			
			float Velocity = CalculateVelocity() * 0.036f; 
			Steering.TargetRotateDeceleration = (Velocity > Steering.AccelerationRotation_Max_Velocity + 5.0f ? FMath::CeilToFloat(int(Velocity - 100.0f) * 0.375f) : Velocity - Steering.AccelerationRotation_Max_Velocity) * 27.78f; 
		}

		Engine.bPressedAccelerator = true;  
	}
	else
	{
		Engine.bPressedAccelerator = false;

		if (Steering.bPressedHandle)
		{
			float CurVelocity = CalculateVelocity();
			CurVelocity = CurVelocity / (Steering.TireCircumference * 1.67f); 
			Engine.RPM = CurVelocity * PowerTrain.GearRatioOfTransmission * PowerTrain.GearRatioOfFinalReductionGear; 
			Steering.RotateDeceleration = 0.0f; 
		}
	}
} 

void UChassisComponent::RotateHandle(float Dir)
{
	if (Dir)
	{
		Steering.HandleAngle += Dir;
		Steering.HandleAngle = FMath::Clamp(Steering.HandleAngle, -Steering.Max_Angle, Steering.Max_Angle);
		
		float Velocity = CalculateVelocity() * 0.036f;
		if (Engine.bPressedAccelerator)
		{
			if (!Steering.bPressedHandle)
			{ 
				Steering.TargetRotateDeceleration = (Velocity > Steering.AccelerationRotation_Max_Velocity + 5.0f ? FMath::CeilToFloat(int(Velocity - 100.0f) * 0.375f) : Velocity - Steering.AccelerationRotation_Max_Velocity) * 27.78f; 
			}
			
			Steering.RotateDeceleration = FMath::Lerp(Steering.RotateDeceleration, Steering.TargetRotateDeceleration, 0.05f);
			Steering.RotateDeceleration = Steering.TargetRotateDeceleration >= 0 ?
				FMath::Clamp(Steering.RotateDeceleration, 0, Steering.TargetRotateDeceleration) : 
				FMath::Clamp(Steering.RotateDeceleration, Steering.TargetRotateDeceleration, 0);
		} 
		else
		{
			if (Velocity < Steering.DefaultRotation_Max_Velocity)
			{
				if (!Steering.bPressedHandle)
				{
					Steering.TargetRotateDeceleration = (Velocity - Steering.DefaultRotation_Max_Velocity) * 27.78f;
				}
				
				Steering.RotateDeceleration = FMath::Lerp(Steering.RotateDeceleration, Steering.TargetRotateDeceleration, 0.05f);
				Steering.RotateDeceleration = Steering.TargetRotateDeceleration >= 0 ?
					FMath::Clamp(Steering.RotateDeceleration, 0, Steering.TargetRotateDeceleration) : 
					FMath::Clamp(Steering.RotateDeceleration, Steering.TargetRotateDeceleration, 0);
		
			}
		}
		
		
		Steering.bPressedHandle = true; 
	} 
	else
	{
		Steering.bPressedHandle = false;

		float CurVelocity = CalculateVelocity();
		CurVelocity = CurVelocity / (Steering.TireCircumference * 1.67f); 
		Engine.RPM = CurVelocity * PowerTrain.GearRatioOfTransmission * PowerTrain.GearRatioOfFinalReductionGear; 
		Steering.RotateDeceleration = 0.0f; 
	}
}

float UChassisComponent::CalculateVelocity()
{ 
	float FinalRPM = Engine.RPM / (PowerTrain.GearRatioOfTransmission * PowerTrain.GearRatioOfFinalReductionGear);
	return Steering.TireCircumference * FinalRPM * 1.67f - Steering.RotateDeceleration; 
}

FVector UChassisComponent::CalculateForwardDirection(const FVector& CurForward)
{
	float Velocity = CalculateVelocity() * 0.036f;

	if (Velocity > 3.0f) 
	{
		FQuat q(FVector(0, 0, 1), FMath::DegreesToRadians(Steering.HandleAngle * GetWorld()->GetDeltaSeconds() * (Engine.RPM > 0 ? 1 : -1))); 
		return q.RotateVector(CurForward);
	}

	return CurForward; 
}

void UChassisComponent::Deceleration()
{
	if (!Engine.bPressedAccelerator && Engine.RPM)
	{
		float Velocity = CalculateVelocity() * 0.036f; 
		if (!Steering.bPressedHandle || (Steering.bPressedHandle && Velocity > Steering.DefaultRotation_Max_Velocity)) 
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