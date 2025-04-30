// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Main_Player.h"

UENUM(BlueprintType)
enum class EInputAction : uint8 
{
	Accelerator	UMETA(DisplayName = "Accelerator"),
	Decelerator	UMETA(DisplayName = "Decelerator"),
	LeftHandle	UMETA(DisplayName = "LeftHandle"), 
	RightHandle	UMETA(DisplayName = "RightHandle"), 
	Drift		UMETA(DisplayName = "Drift"),
	Booster		UMETA(DisplayName = "Booster")
};

UENUM(BlueprintType)
enum class EWidgetType : uint8
{
	BoosterUI UMETA(DisplayName = "BoosterUI"), 
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
};
