// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BoosterUI.generated.h"

UCLASS()
class POTATORIDER_DRIFT_API UBoosterUI : public UUserWidget
{
	GENERATED_BODY()

public: 
	void UpdateBoosterGauge(float Percent); 
	void UpdateBooster(int Count); 
	
private: 
	UPROPERTY(meta=(BindWidget))
	class UProgressBar* PB_BoosterGauge; 
	UPROPERTY(meta=(BindWidget)) 
	class UImage* IMG_Slot1; 
	UPROPERTY(meta=(BindWidget))
	class UImage* IMG_Slot2; 

	
};
