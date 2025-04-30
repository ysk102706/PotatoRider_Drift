// Fill out your copyright notice in the Description page of Project Settings.


#include "BoosterUI.h"
#include "Components/ProgressBar.h"

void UBoosterUI::SetBoosterGauge(float Percent)
{
	BoosterGauge->SetPercent(Percent); 
}
