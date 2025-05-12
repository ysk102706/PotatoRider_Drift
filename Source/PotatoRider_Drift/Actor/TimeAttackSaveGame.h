// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "TimeAttackSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class POTATORIDER_DRIFT_API UTimeAttackSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY() 
	float BestTime;
	
};
