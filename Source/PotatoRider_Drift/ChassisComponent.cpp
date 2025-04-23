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

float UChassisComponent::CalculateVelocity()
{
	float tireCircumference = PI * 0.4572f;
	float FinalRPM = Engine.RPM / (PowerTrain.GearRatioOfTransmission * PowerTrain.GearRatioOfFinalReductionGear);
	return tireCircumference * FinalRPM * 1.67f; 
}

void UChassisComponent::Deceleration()
{
	if (!Engine.bPressedAccelerator && Engine.RPM)
	{
		Engine.RPM += 10.0f * (Engine.RPM > 0 ? -1 : 1); 
	}
}
