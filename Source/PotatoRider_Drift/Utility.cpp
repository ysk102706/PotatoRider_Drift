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
	//L1 = L1.GetSafeNormal2D() * L1.GetClampedToSize2D(-100.f, 100.f);
	double L2 = Pre.Forward.Dot(L1); 
	
	float x = FMath::Acos(Cur.Right.Dot(Pre.Right)); 
	float L3 = FMath::Max(L2 / FMath::Max(FMath::Tan(x), KINDA_SMALL_NUMBER), KINDA_SMALL_NUMBER); 
	float L4 = FMath::Max(FMath::Sqrt(L1.Length() * L1.Length() - L2 * L2), KINDA_SMALL_NUMBER);

	float ret = Velocity * Velocity / (L3 + L4);
	if (ret < -99999999.f || ret > 99999999.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cur.Position.X(%f), Pre.Position.X(%f), ret(%f), Velocity(%f), L1(%f), L2(%f), (L3(%f), L4(%f))"), Cur.Position.X, Pre.Position.X, ret, Velocity, L1.Length(), L2, L3, L4);
	}
	return  ret;
}
