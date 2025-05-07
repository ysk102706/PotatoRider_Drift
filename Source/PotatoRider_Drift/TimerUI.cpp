// Fill out your copyright notice in the Description page of Project Settings.


#include "TimerUI.h" 
#include "Components/TextBlock.h" 

void UTimerUI::UpdatePlayTime(float PlayTime)
{ 
	FString Minute = FString::FromInt(int(PlayTime) / 60); 
	FString Second = FString::FromInt(int(PlayTime) % 60);
	FString ETC = FString::FromInt(int(PlayTime * 1000) % 1000); 
	FString Time = (Minute.Len() == 1 ? "0" : "") + Minute + TEXT(":") + 
		(Second.Len() == 1 ? "0" : "") + Second + TEXT(".") + 
		(ETC.Len() == 1 ? "00" : (ETC.Len() == 2 ? "0" : "")) + ETC; 
	TB_PlayTime->SetText(FText::FromString(Time)); 
} 

void UTimerUI::UpdateLapTime(int LapCount, float LapTime)
{
	FString Minute = FString::FromInt(int(LapTime) / 60); 
	FString Second = FString::FromInt(int(LapTime) % 60);
	FString ETC = FString::FromInt(int(LapTime * 1000) % 1000); 
	FString Time = (Minute.Len() == 1 ? "0" : "") + Minute + TEXT(":") + 
		(Second.Len() == 1 ? "0" : "") + Second + TEXT(".") + 
		(ETC.Len() == 1 ? "00" : (ETC.Len() == 2 ? "0" : "")) + ETC; 
	TB_BestLap->SetText(FText::FromString(Time)); 
	TB_LapCount->SetText(FText::FromString(FString::FromInt(LapCount))); 
}

void UTimerUI::SetMaxLap(int LapCount)
{ 
	TB_MaxLapCount->SetText(FText::FromString(FString::FromInt(LapCount))); 
}
