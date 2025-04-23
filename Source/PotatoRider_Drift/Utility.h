// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#define PI 3.141592

UENUM(BlueprintType)
enum class EInputAction : uint8 
{
	Accelerator UMETA(DisplayName = "Accelerator"),
	Handle UMETA(DisplayName = "Handle") 
};

class POTATORIDER_DRIFT_API Utility
{
public:
	Utility();
};
