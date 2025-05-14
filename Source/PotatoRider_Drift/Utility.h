// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Player/Main_Player.h"

UENUM(BlueprintType)
enum class EInputAction : uint8 
{
	Accelerator	UMETA(DisplayName = "Accelerator"),
	Decelerator	UMETA(DisplayName = "Decelerator"),
	LeftHandle	UMETA(DisplayName = "LeftHandle"), 
	RightHandle	UMETA(DisplayName = "RightHandle"), 
	Drift		UMETA(DisplayName = "Drift"),
	Booster		UMETA(DisplayName = "Booster"),
	Reset 		UMETA(DisplayName = "Reset"), 
};

UENUM(BlueprintType)
enum class EWidgetType : uint8
{
	BoosterUI UMETA(DisplayName = "BoosterUI"), 
	SpeedometerUI UMETA(DisplayName = "SpeedometerUI"), 
	CountDownUI UMETA(DisplayName = "CountDownUI"),
	TimerUI UMETA(DisplayName = "TimerUI"),
	ResultUI UMETA(DisplayName = "ResultUI"),
};

class POTATORIDER_DRIFT_API Utility
{
public:
	Utility();

	static bool Between_II(float Value, float Inclusive_Min, float Inclusive_Max); 
	static bool Between_IE(float Value, float Inclusive_Min, float Exclusive_Max); 
	static bool Between_EI(float Value, float Exclusive_Min, float Inclusive_Max); 
	static bool Between_EE(float Value, float Exclusive_Min, float Exclusive_Max);

	static float CalculateCentrifugalForce(float Velocity, FPositionData Pre, FPositionData Cur);
	static FVector CalculateInelasticCollision(FVector Dir, float V1, float e = 0.35f);
	static FVector CalculateReflectionVector(FVector Incident, FVector Normal); 
};
