// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MiniMapCapture.generated.h"

UCLASS()
class POTATORIDER_DRIFT_API AMiniMapCapture : public AActor
{
	GENERATED_BODY()
	
public:	
	AMiniMapCapture(); 
	virtual void BeginPlay() override; 
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere)
	class USceneCaptureComponent2D* MinimapCaptureComp;

	UPROPERTY()
	class AActor* Target;
	
};
