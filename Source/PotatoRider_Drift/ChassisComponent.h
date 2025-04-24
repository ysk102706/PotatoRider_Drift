// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChassisComponent.generated.h"

USTRUCT(BlueprintType)
struct FEngineDevice
{
	GENERATED_BODY()

public: 
	float RPM; 
	float Default_Max_RPM = 5000.0f;
	float Boost_Max_RPM = 7000.0f;
	float Reverse_Max_RPM = -2000.0f; 
	
	bool bPressedAccelerator; 
}; 

USTRUCT(BlueprintType)
struct FPowerTrainDevice
{
	GENERATED_BODY()

public: 
	float GearRatioOfTransmission = 0.913f; 
	float GearRatioOfFinalReductionGear = 3.357f; 
};

USTRUCT(BlueprintType)
struct FSteeringDevice
{
	GENERATED_BODY()

public: 
	float HandleAngle;
	float Max_Angle = 45.0f;
	float TireCircumference = PI * 0.71; 

	float RotateDeceleration;
	float TargetRotateDeceleration;

	float DefaultRotation_Max_Velocity = 30.0f; 
	float AccelerationRotation_Max_Velocity = 105.0f; 
	
	bool bPressedHandle; 
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class POTATORIDER_DRIFT_API UChassisComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UChassisComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Accelerator(float Difference);
	void RotateHandle(float Dir);

	UFUNCTION(BlueprintCallable)
	float CalculateVelocity();
	FVector CalculateForwardDirection(const FVector& CurForward); 
	
private:
	void Deceleration();
	void RevertHandle();
	
	FEngineDevice Engine; 
	FPowerTrainDevice PowerTrain;
	FSteeringDevice Steering; 
	
};
