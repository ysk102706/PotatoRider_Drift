// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CountDownUI.generated.h"

UCLASS()
class POTATORIDER_DRIFT_API UCountDownUI : public UUserWidget
{
	GENERATED_BODY()
	
public: 
	virtual void NativeConstruct() override; 
	bool UpdateCountDown();
	void ShowResult(FString Result); 
	
private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_CountDown;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Result;

	int Idx; 
	UPROPERTY() 
	TArray<FText> CountDownMessages; 

};
