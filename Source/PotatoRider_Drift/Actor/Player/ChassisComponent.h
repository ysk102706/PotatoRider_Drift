// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Utility.h"
#include "ChassisComponent.generated.h"

USTRUCT(BlueprintType)
struct FEngineDevice
{
	GENERATED_BODY()

public: 
	float RPM; 
	float Default_Max_RPM = 4500.0f; 
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
	float Max_Angle = 35.0f; 
	float EndDriftAngle = 26.5f; 
	float TireCircumference = PI * 0.71f; 

	float Correction_Min_Vel; 
	float Correction_Max_Vel; 

	float Default_Min_Vel = 0.0f; 
	float Default_Max_Vel = 40.0f; 
	float Acceleration_Max_Vel = 105.0f;
	float Deceleration_Max_Vel = -70.0f; 

	float RotateDeceleration;
	float TargetRotateDeceleration;
	
	float CameraAngleRate; 
	
	bool bPressedHandle; 
};

USTRUCT(BlueprintType)
struct FSuspensionDevice
{
	GENERATED_BODY()

public:
	float Angle;
	float OverallLength = 135.0f;
	float OverallWidth = 75.0f; 
};

USTRUCT(BlueprintType) 
struct FDriftDevice
{
	GENERATED_BODY() 

public: 
	float DriftAngle; 
	float LastDriftDir; 
	float Max_DriftAngle = 60.0f; 
	float InertiaAngle; 
	
	float Deceleration_RPM; 
	
	float CurDriftTime; 

	bool bDrift; 
	bool bPressedDrift; 
	bool bUsedDrift;
	bool bRemainCentrifugalForce; 
	bool bBreakDrift; 
	bool bDoubleDrift; 
	bool bConnectDrift; 
	bool bIsConnectDrift; 
	
	FTimerHandle DriftTimerHandle; 
	FTimerHandle InertiaAngleTimerHandle; 
}; 

USTRUCT(BlueprintType) 
struct FBoosterDevice
{
	GENERATED_BODY() 

public: 
	float BoosterGauge; 
	float Max_BoosterGauge = 100.0f; 
	int Count;

	float Additional_RPM;
	float Max_Additional_RPM; 

	float Default_Max_Additional_RPM = 0.0f; 
	float Normal_Max_Additional_RPM = 6000.0f; 
	float Rotated_Normal_Max_Additional_RPM = 1500.0f; 
	float Moment_Max_Additional_RPM = 850.0f; 
	float PerfectStart_Max_Additional_RPM = 3000.0f; 
	float GoodStart_Max_Additional_RPM = 2500.0f; 
	
	bool bBoost; 
	bool bRotatedBoost; 
	bool bMomentBoostTiming;
	bool bMomentBoost; 
	bool bStartBoost;
	bool bUsedMomentBoost;
	bool bUsedStartBoost;

	FTimerHandle BoosterTimerHandle;
	FTimerHandle MomentBoosterTimerHandle; 
	FTimerHandle StartBoosterTimerHandle; 
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class POTATORIDER_DRIFT_API UChassisComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UChassisComponent(); 
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Accelerator(float Difference);
	void Handle(float Dir); 
	void DriftDevice(bool bPressed); 
	void BoosterDevice(); 

	float CalculateVelocity(bool bIncludeBooster); 
	FQuat CalculateHandleQuat();
	FQuat CalculateSuspensionQuat(FVector Axis); 
	bool IsDrift(); 
	bool IsRemainDrift(); 
	float GetDriftAngleRate(); 
	float GetDriftDir(); 
	bool IsBreakDrift(); 
	bool IsFullDrift(); 
	bool IsBoost();
	float GetSuspensionAxisAngle(); 
	bool IsRedLight(); 
	
	void ResetHandleForce(); 
	void BreakDrift();

	void OnCollisionDetection(); 
	
private:
	void Deceleration();
	void RevertHandle(); 
	void RevertDrift();
	void Boost(); 

	void SetVelocityToEngineRPM(bool bIncludeBooster); 
	void UpdateCameraAngleRate(); 
	
	bool CheckSteeringCorrection(float CurVelocity);
	float GetMaxAngle(); 

	void PlaySound(ESoundType Type); 
	
	FEngineDevice Engine; 
	FPowerTrainDevice PowerTrain; 
	FSteeringDevice Steering;
	FSuspensionDevice Suspension; 
	FDriftDevice Drift; 
	FBoosterDevice Booster;
	
	UPROPERTY(EditAnywhere, Category = Sound)
	TArray<class USoundBase*> Sound; 

	UPROPERTY()
	class ARacingGameMode* GameMode;
};

