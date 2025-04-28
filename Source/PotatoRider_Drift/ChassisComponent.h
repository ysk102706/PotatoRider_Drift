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
	float Reverse_Max_RPM = -3500.0f; 
	
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
	float Max_Angle = 40.0f;
	float TireCircumference = PI * 0.71; 

	float Correction_Min_Vel; 
	float Correction_Max_Vel; 

	float Default_Min_Vel = 0.0f; 
	float Default_Max_Vel = 30.0f; 
	float Acceleration_Max_Vel = 105.0f;
	float Deceleration_Max_Vel = -70.0f; 

	float RotateDeceleration;
	float TargetRotateDeceleration;

	float HoldTime; 
	float CameraAngleRate; 
	
	bool bPressedHandle; 
}; 

USTRUCT(BlueprintType) 
struct FDriftDevice
{
	GENERATED_BODY() 

public: 
	float DriftAngle; 

	bool bDrift; 
	bool bPressedDrift; 

	FTimerHandle DriftTimerHandle; 
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
	void Handle(float Dir); 
	void DriftDevice(bool bPressed); 

	float CalculateVelocity();
	FQuat CalculateQuat(); 

	float GetHandleHoldTime();

	void ResetHandleForce(); 
	
private:
	void Deceleration();
	void RevertHandle(); 
	void RevertDrift(); 

	void SetVelocityToEngineRPM(); 
	void UpdateCameraAngleRate(); 

	bool CheckSteeringCorrection(float CurVelocity); 

	FEngineDevice Engine; 
	FPowerTrainDevice PowerTrain; 
	FSteeringDevice Steering; 
	FDriftDevice Drift; 
	
};
