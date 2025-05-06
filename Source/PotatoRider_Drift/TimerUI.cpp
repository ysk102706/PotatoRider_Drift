// Fill out your copyright notice in the Description page of Project Settings.


#include "TimerUI.h" 
#include "Components/TextBlock.h" 

void UTimerUI::UpdatePlayTime(float PlayTime)
{ 
	FString Time = FString::FromInt(int(PlayTime) / 60) + TEXT(":") + 
		FString::FromInt(int(PlayTime) % 60) + TEXT(".") + 
		FString::FromInt(int(PlayTime * 1000) % 1000); 
	TB_PlayTime->SetText(FText::FromString(Time)); 
} 

void UTimerUI::UpdateLapTime(int LapCount, float LapTime)
{ 
	FString Time = FString::FromInt(int(LapTime) / 60) + TEXT(":") + 
		FString::FromInt(int(LapTime) % 60) + TEXT(".") + 
		FString::FromInt(int(LapTime * 1000) % 1000); 
	TB_BestLap->SetText(FText::FromString(Time)); 
	TB_LapCount->SetText(FText::FromString(FString::FromInt(LapCount))); 
}

void UTimerUI::SetMaxLap(int LapCount)
{ 
	TB_MaxLapCount->SetText(FText::FromString(FString::FromInt(LapCount))); 
}
