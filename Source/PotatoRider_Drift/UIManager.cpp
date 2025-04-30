// Fill out your copyright notice in the Description page of Project Settings.


#include "UIManager.h" 

void UUIManager::ShowWidget(UWorld* World, EWidgetType Type)
{
	if (auto widget = GetWidget<UUserWidget>(World, Type))
	{
		widget->AddToViewport(); 
	}
}

void UUIManager::HideWidget(UWorld* World, EWidgetType Type)
{
	if (auto widget = GetWidget<UUserWidget>(World, Type))
	{
		widget->RemoveFromParent(); 
	}
}
