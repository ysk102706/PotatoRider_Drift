// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResetPoint.generated.h"

UCLASS()
class POTATORIDER_DRIFT_API AResetPoint : public AActor
{
	GENERATED_BODY()
	
public: 
	AResetPoint(); 
	virtual void BeginPlay() override; 
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere) 
	int32 Idx; 

private:
	UFUNCTION()
	void OnBoxCompBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); 

	UPROPERTY(EditAnywhere)
	class UBoxComponent* BoxComp; 
	
}; 
