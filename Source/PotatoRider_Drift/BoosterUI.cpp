// Fill out your copyright notice in the Description page of Project Settings.


#include "BoosterUI.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"

void UBoosterUI::UpdateBoosterGauge(float Percent)
{
	PB_BoosterGauge->SetPercent(Percent); 
}

void UBoosterUI::UpdateBooster(int Count)
{ 
	FLinearColor None(1.0f, 1.0f, 1.0f, 0.0f); 
	FLinearColor Booster(1.0f, 1.0f, 1.0f, 1.0f); 

	switch (Count)
	{
	case 0:
		IMG_Slot1->SetColorAndOpacity(None); 
		IMG_Slot2->SetColorAndOpacity(None); 
		break; 
	case 1: 
		IMG_Slot1->SetColorAndOpacity(Booster);
		IMG_Slot2->SetColorAndOpacity(None);
		break;
	case 2: 
		IMG_Slot1->SetColorAndOpacity(Booster); 
		IMG_Slot2->SetColorAndOpacity(Booster); 
		break;
	}
}
