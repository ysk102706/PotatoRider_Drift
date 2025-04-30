// Fill out your copyright notice in the Description page of Project Settings.


#include "RacingGameMode.h" 
#include "UIManager.h"

ARacingGameMode::ARacingGameMode()
{
	ConstructorHelpers::FClassFinder<UUIManager> UIManager(TEXT("/Script/Engine.Blueprint'/Game/Blueprints/BP_UIManager.BP_UIManager_C'")); 
	if (UIManager.Succeeded())
	{
		UIManagerFactory = UIManager.Class; 
	}
}

void ARacingGameMode::BeginPlay()
{
	Super::BeginPlay();

	UIManagerObject = NewObject<UUIManager>(this, UIManagerFactory);
}

UUIManager* ARacingGameMode::UI()
{
	return UIManagerObject; 
}
