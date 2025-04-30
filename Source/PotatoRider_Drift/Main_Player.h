// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputAction.h"
#include "Main_Player.generated.h"

USTRUCT(BlueprintType) 
struct FPositionData 
{
	GENERATED_BODY() 

public: 
	FVector Position;
	FVector Forward;
	FVector Right; 
}; 

UCLASS()
class POTATORIDER_DRIFT_API AMain_Player : public APawn
{
	GENERATED_BODY()

public:
	AMain_Player();
	virtual void BeginPlay() override; 
	virtual void Tick(float DeltaTime) override; 
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
private:
	void OnPressAccelerator(const FInputActionValue& Value); 
	void OnReleaseAccelerator(const FInputActionValue& Value);
	void OnPressDecelerator(const FInputActionValue& Value); 
	void OnReleaseDecelerator(const FInputActionValue& Value); 
	void OnPressLeftHandle(const FInputActionValue& Value); 
	void OnReleaseLeftHandle(const FInputActionValue& Value);
	void OnPressRightHandle(const FInputActionValue& Value); 
	void OnReleaseRightHandle(const FInputActionValue& Value); 
	void OnPressStartDrift(const FInputActionValue& Value); 
	void OnReleaseDrift(const FInputActionValue& Value);
	void OnActionBooster(const FInputActionValue& Value); 

	void Accelerator(float Value);
	void Handle(float Value);
	void SetCameraAndMeshRotation(); 

	UPROPERTY()
	class ARacingGameMode* GameMode; 
	
	UPROPERTY(EditAnywhere) 
	class USceneComponent* Root; 
	UPROPERTY(EditAnywhere)
	class UBoxComponent* BoxComp; 
	UPROPERTY(EditAnywhere) 
	class UStaticMeshComponent* MeshComp; 
	UPROPERTY(EditAnywhere) 
	class USceneComponent* AimPoint; 
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* SpringArmComp;
	UPROPERTY(EditAnywhere)
	class UCameraComponent* CameraComp; 
	
	UPROPERTY() 
	class UChassisComponent* ChassisComp; 
	
	UPROPERTY(EditAnywhere, Category = Input)
	class UInputMappingContext* IMC_Player;
	UPROPERTY(EditAnywhere, Category = Input)
	TArray<class UInputAction*> IA; 

	TArray<float> AccelerationInputStack; 
	TArray<float> HandleInputStack; 
	
	FRotator MeshRot;
	FPositionData LastPositionData;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> TempSkidMark; 

};
