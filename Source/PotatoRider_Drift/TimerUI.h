// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerUI.generated.h"

UCLASS()
class POTATORIDER_DRIFT_API UTimerUI : public UUserWidget
{
	GENERATED_BODY()
	
public: 
	void UpdatePlayTime(float PlayTime); 
	void UpdateLapTime(int LapCount, float LapTime); 
	void SetMaxLap(int LapCount); 

private: 
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* TB_PlayTime; 
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_BestLap; 
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_LapCount; 
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_MaxLapCount;

};
