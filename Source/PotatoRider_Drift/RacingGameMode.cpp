// Fill out your copyright notice in the Description page of Project Settings.


#include "RacingGameMode.h" 
#include "UIManager.h"
#include "CountDownUI.h"
#include "TimerUI.h"

ARacingGameMode::ARacingGameMode()
{ 
	PrimaryActorTick.bCanEverTick = true;

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

	UIManagerObject->ShowWidget(GetWorld(), EWidgetType::CountDownUI); 
	UIManagerObject->ShowWidget(GetWorld(), EWidgetType::TimerUI); 
	GetWorld()->GetTimerManager().SetTimer(CountDownTimerHandle, FTimerDelegate::CreateLambda([&]()
	{ 
		if (!UIManagerObject->GetWidget<UCountDownUI>(GetWorld(), EWidgetType::CountDownUI)->UpdateCountDown())
		{
			GetWorld()->GetTimerManager().ClearTimer(CountDownTimerHandle); 
		} 
	}), 1.0f, true); 

	UIManagerObject->GetWidget<UTimerUI>(GetWorld(), EWidgetType::TimerUI)->SetMaxLap(MaxLapCount); 
	LapCount = 0; 
} 

void ARacingGameMode::Tick(float DeltaSeconds)
{ 
	Super::Tick(DeltaSeconds); 

	if (CountDownTimer < 5.1f) 
	{ 
		CountDownTimer += DeltaSeconds; 
	} 
	else
	{
		UIManagerObject->HideWidget(GetWorld(), EWidgetType::CountDownUI); 
	}

	if (CountDownTimer > 4.0f) 
	{
		PlayTimer += DeltaSeconds; 
		LapTimer += DeltaSeconds; 

		UIManagerObject->GetWidget<UTimerUI>(GetWorld(), EWidgetType::TimerUI)->UpdatePlayTime(PlayTimer); 
	} 
}

UUIManager* ARacingGameMode::UI()
{
	return UIManagerObject; 
}

float ARacingGameMode::GetTimer()
{
	return CountDownTimer;
}

void ARacingGameMode::UpdateLapTime()
{ 
	BestLapTimer = LapCount ? (BestLapTimer == 0.0f ? LapTimer : FMath::Min(BestLapTimer, LapTimer)) : 0.0f; 
	UIManagerObject->GetWidget<UTimerUI>(GetWorld(), EWidgetType::TimerUI)->UpdateLapTime(++LapCount, BestLapTimer); 

	LapTimer = 0.0f; 

	if (LapCount > MaxLapCount)
	{ 

	}
}
