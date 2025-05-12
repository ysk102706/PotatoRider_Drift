// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StartAndGoalLine.generated.h"

UCLASS()
class POTATORIDER_DRIFT_API AStartAndGoalLine : public AActor
{
	GENERATED_BODY()
	
public:	
	AStartAndGoalLine(); 
	virtual void BeginPlay() override; 
	virtual void Tick(float DeltaTime) override; 

private: 
	UFUNCTION() 
	void OnBoxCompBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); 

	UPROPERTY(EditAnywhere) 
	class UBoxComponent* BoxComp; 

	int Count; 

	UPROPERTY()
	class ARacingGameMode* GameMode; 

};
