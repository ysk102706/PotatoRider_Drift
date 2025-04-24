// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputAction.h"
#include "Main_Player.generated.h"

UCLASS()
class POTATORIDER_DRIFT_API AMain_Player : public APawn
{
	GENERATED_BODY()

public:
	AMain_Player();
	virtual void BeginPlay() override; 
	virtual void Tick(float DeltaTime) override; 
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UChassisComponent* ChassisComp;
private:
	void OnPressAccelerator(const FInputActionValue& Value); 
	void OnReleaseAccelerator(const FInputActionValue& Value); 
	void OnPressHandle(const FInputActionValue& Value); 
	void OnReleaseHandle(const FInputActionValue& Value); 
	
	
	UPROPERTY(EditAnywhere, Category = Input)
	class UInputMappingContext* IMC_Player;
	UPROPERTY(EditAnywhere, Category = Input)
	TArray<class UInputAction*> IA; 

};
