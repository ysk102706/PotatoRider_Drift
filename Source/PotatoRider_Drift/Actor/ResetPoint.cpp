// Fill out your copyright notice in the Description page of Project Settings.


#include "ResetPoint.h" 
#include "Components/BoxComponent.h"
#include "Player/Main_Player.h"

AResetPoint::AResetPoint()
{
 	PrimaryActorTick.bCanEverTick = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	SetRootComponent(BoxComp);
	BoxComp->SetBoxExtent(FVector(50.0f)); 
}

void AResetPoint::BeginPlay()
{
	Super::BeginPlay();

	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AResetPoint::OnBoxCompBeginOverlap);
}

void AResetPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AResetPoint::OnBoxCompBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto* Player = Cast<AMain_Player>(OtherActor))
	{
		Player->SetResetPoint(this); 
	}
}

