// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedometerUI.h" 
#include "Components/TextBlock.h"  

void USpeedometerUI::UpdateSpeed(float Speed)
{ 
	TB_Speed->SetText(FText::FromString(FString::FromInt(int(Speed)))); 
}
