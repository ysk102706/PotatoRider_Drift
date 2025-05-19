// Fill out your copyright notice in the Description page of Project Settings.


#include "RacingGameMode.h" 
#include "../UIManager.h"
#include "../UI/CountDownUI.h"
#include "../UI/ResultUI.h"
#include "../UI/TimerUI.h"

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
	LapCount = 1;

	auto* pc = GetWorld()->GetFirstPlayerController();
	pc->SetShowMouseCursor(false);
	pc->SetInputMode(FInputModeGameOnly()); 
} 

void ARacingGameMode::Tick(float DeltaSeconds)
{ 
	Super::Tick(DeltaSeconds); 

	if (CountDownTimer < 5.75f) 
	{ 
		CountDownTimer += DeltaSeconds; 
	} 
	else 
	{ 
		UIManagerObject->HideWidget(GetWorld(), EWidgetType::CountDownUI); 
	}

	if (!bIsRaceEnd && CountDownTimer > 4.0f) 
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

bool ARacingGameMode::IsRaceEnd()
{
	return bIsRaceEnd; 
}

int ARacingGameMode::GetResetPointIdx()
{
	return ResetPointIdx; 
}

bool ARacingGameMode::IsUpdateResetPoint(int Idx)
{
	bool c1 = ResetPointIdx + 1 == Idx; 	
	bool c2 = ResetPointIdx != 1 && ResetPointIdx - 1 == Idx; 	
	bool c3 = (ResetPointIdx) % LapPerResetPoint == Idx;

	bool ret = c1 || c2 || c3;
	if (!ret)
	{
		
	}
	else
	{
		ResetPointIdx = Idx; 
	}
	return ret;
}

void ARacingGameMode::UpdateLapTime()
{ 
	if (LapCount > MaxLapCount)
	{ 
		bIsRaceEnd = true; 
		UIManagerObject->GetWidget<UResultUI>(GetWorld(), EWidgetType::ResultUI)->UpdateResult(PlayTimer); 
		UIManagerObject->ShowWidget(GetWorld(), EWidgetType::ResultUI); 
		UIManagerObject->HideWidget(GetWorld(), EWidgetType::BoosterUI); 
		UIManagerObject->HideWidget(GetWorld(), EWidgetType::SpeedometerUI); 
		UIManagerObject->HideWidget(GetWorld(), EWidgetType::TimerUI);

		auto* pc = GetWorld()->GetFirstPlayerController();
		pc->SetShowMouseCursor(true);
		pc->SetInputMode(FInputModeUIOnly()); 
		return; 
	}

	ResetPointIdx = 1; 
	
	BestLapTimer = LapCount > 1 ? (BestLapTimer == 0.0f ? LapTimer : FMath::Min(BestLapTimer, LapTimer)) : 0.0f; 
	UIManagerObject->GetWidget<UTimerUI>(GetWorld(), EWidgetType::TimerUI)->UpdateLapTime(LapCount++, BestLapTimer); 

	LapTimer = 0.0f; 
}
