// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResultUI.generated.h"

/**
 * 
 */
UCLASS()
class POTATORIDER_DRIFT_API UResultUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override; 
	
	void UpdateResult(float ResultTime);
	void SaveBestTime(); 
	void LoadBestTime(); 

private: 
	UFUNCTION()
	void Restart();
	UFUNCTION()
	void Quit();
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* TB_ResultTime;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* TB_BestTime;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* TB_ResultMessage1;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* TB_ResultMessage2;

	UPROPERTY(meta=(BindWidget))
	class UButton* BTN_Restart; 
	UPROPERTY(meta=(BindWidget))
	class UButton* BTN_Quit;

	float BestTime; 
	FString SaveGameSlotName = TEXT("BestTimeSaveData");
	int32 SaveGameSlotIdx = 0; 
	
};
