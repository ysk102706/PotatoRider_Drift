// Fill out your copyright notice in the Description page of Project Settings.


#include "ResultUI.h" 
#include "TimeAttackSaveGame.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UResultUI::NativeConstruct()
{
	Super::NativeConstruct();

	BTN_Restart->OnClicked.AddDynamic(this, &UResultUI::Restart); 
	BTN_Quit->OnClicked.AddDynamic(this, &UResultUI::Quit); 
}

void UResultUI::UpdateResult(float ResultTime)
{
	FString Minute = FString::FromInt(int(ResultTime) / 60); 
	FString Second = FString::FromInt(int(ResultTime) % 60);
	FString ETC = FString::FromInt(int(ResultTime * 1000) % 1000); 
	FString Time = (Minute.Len() == 1 ? "0" : "") + Minute + TEXT(":") + 
		(Second.Len() == 1 ? "0" : "") + Second + TEXT(".") + 
		(ETC.Len() == 1 ? "00" : (ETC.Len() == 2 ? "0" : "")) + ETC; 
	
	LoadBestTime();
	FString BMinute = FString::FromInt(int(BestTime) / 60); 
	FString BSecond = FString::FromInt(int(BestTime) % 60);
	FString BETC = FString::FromInt(int(BestTime * 1000) % 1000); 
	FString BTime = (BMinute.Len() == 1 ? "0" : "") + BMinute + TEXT(":") + 
		(BSecond.Len() == 1 ? "0" : "") + BSecond + TEXT(".") + 
		(BETC.Len() == 1 ? "00" : (BETC.Len() == 2 ? "0" : "")) + BETC; 
	
	TB_ResultMessage1->SetText(FText::FromString(ResultTime < BestTime ? TEXT("신기록 달성") : TEXT("완주"))); 
	TB_ResultMessage2->SetText(FText::FromString(ResultTime < BestTime ? TEXT("NEW") : TEXT("TIME")));  
	TB_ResultTime->SetText(FText::FromString(Time));
	TB_BestTime->SetText(FText::FromString(ResultTime < BestTime ? Time : BTime));

	if (ResultTime < BestTime)
	{
		BestTime = ResultTime; 
		SaveBestTime(); 
	}
}

void UResultUI::SaveBestTime()
{ 
	if (auto* sg = Cast<UTimeAttackSaveGame>(UGameplayStatics::CreateSaveGameObject(UTimeAttackSaveGame::StaticClass())))
	{
		sg->BestTime = BestTime;
		UGameplayStatics::SaveGameToSlot(sg, SaveGameSlotName, SaveGameSlotIdx); 
	} 
}

void UResultUI::LoadBestTime()
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveGameSlotName, SaveGameSlotIdx))
	{
		BestTime = 99999999.0f; 
		return; 
	}

	auto* sg = Cast<UTimeAttackSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveGameSlotName, SaveGameSlotIdx));
	BestTime = sg->BestTime; 
}

void UResultUI::Restart()
{
	FString levelName = UGameplayStatics::GetCurrentLevelName(GetWorld()); 
	UGameplayStatics::OpenLevel(GetWorld(), FName(*levelName)); 
}

void UResultUI::Quit()
{
	auto* pc = GetWorld()->GetFirstPlayerController(); 
	UKismetSystemLibrary::QuitGame(GetWorld(), pc, EQuitPreference::Quit, false); 
}
