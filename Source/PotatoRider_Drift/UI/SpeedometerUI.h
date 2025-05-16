// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpeedometerUI.generated.h"

UCLASS()
class POTATORIDER_DRIFT_API USpeedometerUI : public UUserWidget
{
	GENERATED_BODY()
	
public: 
	void UpdateSpeed(float Speed); 
	
private: 
	UPROPERTY(meta=(BindWidget)) 
	class UTextBlock* TB_Speed; 
	
};
