// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility.h"

Utility::Utility()
{
}

bool Utility::Between_II(float Value, float Inclusive_Min, float Inclusive_Max)
{
	return Inclusive_Min <= Value && Value <= Inclusive_Max; 
}

bool Utility::Between_IE(float Value, float Inclusive_Min, float Exclusive_Max)
{
	return Inclusive_Min <= Value && Value < Exclusive_Max; 
}

bool Utility::Between_EI(float Value, float Exclusive_Min, float Inclusive_Max)
{
	return Exclusive_Min < Value && Value <= Inclusive_Max; 
}

bool Utility::Between_EE(float Value, float Exclusive_Min, float Exclusive_Max)
{
	return Exclusive_Min < Value && Value < Exclusive_Max; 
}

float Utility::CalculateCentrifugalForce(float Velocity, FPositionData Pre, FPositionData Cur)
{
	FVector L1 = Cur.Position - Pre.Position;
	double L2 = Pre.Forward.Dot(L1);
	
	float x = FMath::Acos(Cur.Right.Dot(Pre.Right)); 
	float L3 = L2 / FMath::Max(FMath::Tan(x), 0.0001f); 
	float L4 = FMath::Sqrt(L1.Length() * L1.Length() - L2 * L2);

	UE_LOG(LogTemp, Warning, TEXT("%lf, %lf, %lf"), L4, L1.Length(), L2); 
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("%f, %f, %f"), Cur.Right.X, Cur.Right.Y, Cur.Right.Z));
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("%f, %f, %f"), Pre.Right.X, Pre.Right.Y, Pre.Right.Z));
	
	return Velocity * Velocity / (L3 + L4); 
}
