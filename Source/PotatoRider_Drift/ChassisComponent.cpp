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
		Engine.RPM += Difference; 
		Engine.RPM = FMath::Clamp(Engine.RPM, Engine.Max_Reverse_RPM, Engine.Max_RPM);

		Engine.bPressedAccelerator = true; 
	}
	else
	{
		Engine.bPressedAccelerator = false; 
	}
}

void UChassisComponent::Deceleration()
{
	if (!Engine.bPressedAccelerator && Engine.RPM)
	{
		Engine.RPM += 10.0f * (Engine.RPM > 0 ? -1 : 1); 
	}
}

void UChassisComponent::RotateHandle(float Dir)
{
	if (Dir)
	{
		Steering.HandleAngle += Dir;
		Steering.HandleAngle = FMath::Clamp(Steering.HandleAngle, -Steering.Max_Angle, Steering.Max_Angle);

		Steering.bPressedHandle = true; 
	}
	else
	{
		Steering.bPressedHandle = false; 
	}
}

void UChassisComponent::RevertHandle()
{
	if (!Steering.bPressedHandle && Steering.HandleAngle)
	{
		Steering.HandleAngle += 1.0f * (Steering.HandleAngle > 0 ? -1 : 1); 
	}
}

float UChassisComponent::CalculateVelocity()
{
	float tireCircumference = PI * 0.4572f;
	float FinalRPM = Engine.RPM / (PowerTrain.GearRatioOfTransmission * PowerTrain.GearRatioOfFinalReductionGear);
	return tireCircumference * FinalRPM * 1.67f; 
}

FVector UChassisComponent::CalculateForwardDirection(const FVector& CurForward)
{
	return 0; 
}
