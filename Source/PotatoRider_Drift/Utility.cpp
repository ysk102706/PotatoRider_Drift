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
	// float L3 = FMath::Max(L2 / FMath::Max(FMath::Tan(x), KINDA_SMALL_NUMBER), KINDA_SMALL_NUMBER); 
	// float tan = FMath::Tan(x);
	// float L3 = tan < KINDA_SMALL_NUMBER ? 0 : L2 / tan; 
	// float L4 = FMath::Max(FMath::Sqrt(L1.Length() * L1.Length() - L2 * L2), KINDA_SMALL_NUMBER);

	float L3 = L2 / FMath::Sin(x); 
	
	return Velocity * Velocity / L3; 
}

FVector Utility::CalculateInelasticCollision(FVector Dir, float V, float e)
{
	float V_Prime = V / 2 - e * 0.5f * V; 
	return Dir * V_Prime; 
}

FVector Utility::CalculateReflectionVector(FVector Incident, FVector Normal)
{
	return Incident + 2 * Normal * -Incident.Dot(Normal); 
}
