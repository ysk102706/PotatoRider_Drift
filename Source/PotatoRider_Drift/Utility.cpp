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

