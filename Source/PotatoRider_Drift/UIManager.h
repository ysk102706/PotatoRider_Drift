// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utility.h"
#include "Blueprint/UserWidget.h"
#include "UIManager.generated.h"

UCLASS(Blueprintable, BlueprintType)
class POTATORIDER_DRIFT_API UUIManager : public UObject
{
	GENERATED_BODY()
	
public: 
	template <typename Widget>
	Widget* GetWidget(UWorld* World, EWidgetType Type)
	{
		if (!WidgetObjects.Num())
		{
			WidgetObjects.SetNum(WidgetFactorys.Num()); 
		}

		if (!WidgetObjects[int(Type)])
		{ 
			WidgetObjects[int(Type)] = CreateWidget<UUserWidget>(World, WidgetFactorys[int(Type)]);
		} 
		
		return Cast<Widget>(WidgetObjects[int(Type)]); 
	} 
	
	void ShowWidget(UWorld* World, EWidgetType Type);
	void HideWidget(UWorld* World, EWidgetType Type); 

private: 
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UUserWidget>> WidgetFactorys;
	
	UPROPERTY()
	TArray<UUserWidget*> WidgetObjects; 
	
}; 
