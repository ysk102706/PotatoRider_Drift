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
	void SetBoosterGauge(float Percent); 
	
private: 
	UPROPERTY(meta=(BindWidget))
	class UProgressBar* BoosterGauge; 
	
};
