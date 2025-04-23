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
	float Max_RPM = 7000.0f;
	float Max_Reverse_RPM = -2000.0f;

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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class POTATORIDER_DRIFT_API UChassisComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UChassisComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Accelerator(float Difference);
	float CalculateVelocity();
	void Deceleration(); 
	
private:
	FEngineDevice Engine; 
	FPowerTrainDevice PowerTrain;
	
};
