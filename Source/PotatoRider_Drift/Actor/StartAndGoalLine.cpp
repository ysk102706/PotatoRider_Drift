// Fill out your copyright notice in the Description page of Project Settings.


#include "StartAndGoalLine.h" 
#include "Components/BoxComponent.h" 
#include "RacingGameMode.h" 
#include "Player/Main_Player.h"

AStartAndGoalLine::AStartAndGoalLine()
{ 
	PrimaryActorTick.bCanEverTick = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp")); 
	SetRootComponent(BoxComp); 
}

void AStartAndGoalLine::BeginPlay()
{
	Super::BeginPlay(); 

	GameMode = GetWorld()->GetAuthGameMode<ARacingGameMode>();
	
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AStartAndGoalLine::OnBoxCompBeginOverlap); 
}

void AStartAndGoalLine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AStartAndGoalLine::OnBoxCompBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{ 
	if (GameMode->GetResetPointIdx() == 0) 
	{
		GameMode->UpdateLapTime();
	} 
}

