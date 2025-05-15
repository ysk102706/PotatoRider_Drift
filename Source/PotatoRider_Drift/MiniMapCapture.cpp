// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniMapCapture.h"

#include "Components/SceneCaptureComponent2D.h"

AMiniMapCapture::AMiniMapCapture()
{
 	PrimaryActorTick.bCanEverTick = true;

	MinimapCaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>("MinimapCaptureComponent"); 
	SetRootComponent(MinimapCaptureComp); 
}

void AMiniMapCapture::BeginPlay()
{
	Super::BeginPlay();

	Target = GetWorld()->GetFirstPlayerController()->GetPawn(); 
}

void AMiniMapCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); 

	SetActorLocation(Target->GetActorLocation()); 
}

