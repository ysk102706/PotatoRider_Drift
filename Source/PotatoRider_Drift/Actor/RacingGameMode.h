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
	virtual void Tick(float DeltaSeconds) override;
	
	class UUIManager* UI(); 

	float GetTimer(); 
	void UpdateLapTime();
	bool IsRaceEnd(); 
	
private: 
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUIManager> UIManagerFactory;

	UPROPERTY() 
	class UUIManager* UIManagerObject; 

	float CountDownTimer; 
	float PlayTimer; 

	int LapCount;
	UPROPERTY(EditAnywhere) 
	int MaxLapCount = 3; 
	float LapTimer; 
	float BestLapTimer;

	UPROPERTY(EditAnywhere) 
	int LapPerResetPoint; 
	
	bool bIsRaceEnd; 

	FTimerHandle CountDownTimerHandle;

};
