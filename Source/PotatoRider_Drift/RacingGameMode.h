// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h" 
#include "GameFramework/GameModeBase.h"
#include "RacingGameMode.generated.h"

UCLASS()
class POTATORIDER_DRIFT_API ARacingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARacingGameMode(); 
	virtual void BeginPlay() override;
	
	class UUIManager* UI(); 
	
private: 
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUIManager> UIManagerFactory;

	UPROPERTY() 
	class UUIManager* UIManagerObject; 

};
