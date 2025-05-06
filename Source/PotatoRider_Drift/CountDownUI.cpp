// Fill out your copyright notice in the Description page of Project Settings.


#include "CountDownUI.h" 
#include "Components/TextBlock.h"

void UCountDownUI::NativeConstruct()
{ 
	CountDownMessages.Add(FText::FromString("3"));
	CountDownMessages.Add(FText::FromString("2"));
	CountDownMessages.Add(FText::FromString("1"));
	CountDownMessages.Add(FText::FromString("Go"));
}

bool UCountDownUI::UpdateCountDown()
{
	if (Idx >= CountDownMessages.Num()) 
	{
		return false;
	}

	TB_CountDown->SetText(CountDownMessages[Idx++]); 

	return true; 
}
