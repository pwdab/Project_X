// Fill out your copyright notice in the Description page of Project Settings.

#include "Entity/PX_Character.h"

bool APX_Character::ServerBeginAim_Validate(const bool bPressed)
{
	return true;
}

void APX_Character::ServerBeginAim_Implementation(const bool bPressed)
{
	if (!HasAuthority()) return;
	if (!bPressed)	return;			// Invlaid Function call
	if (bIsAiming) return;			// Duplicated Function call

	//UE_LOG(LogTemp, Log, TEXT("Server Begin Aim.. Value : %s"), bPressed ? TEXT("true") : TEXT("false"));
	bIsAiming = bPressed;
}

bool APX_Character::ServerEndAim_Validate(const bool bPressed)
{
	return true;
}

void APX_Character::ServerEndAim_Implementation(const bool bPressed)
{
	if (!HasAuthority()) return;
	if (bPressed)	return;			// Invlaid Function call
	if (!bIsAiming) return;			// Duplicated Function call

	//UE_LOG(LogTemp, Log, TEXT("Server End Aim.. Value : %s"), bPressed ? TEXT("true") : TEXT("false"));
	bIsAiming = bPressed;
	if (bIsDrawing)
	{
		bIsDrawing = bPressed;
		DrawProgress = 0.f;
	}
}

bool APX_Character::ServerBeginDraw_Validate(const bool bPressed)
{
    return true;
}

void APX_Character::ServerBeginDraw_Implementation(const bool bPressed)
{
	if (!HasAuthority()) return;
	if (!bPressed)	return;			// Invlaid Function call
	if (!bIsAiming) return;			// Duplicated Function call

	//UE_LOG(LogTemp, Log, TEXT("Server Begin Draw.. Value : %s"), bPressed ? TEXT("true") : TEXT("false"));
	bIsDrawing = bPressed;
}

bool APX_Character::ServerEndDraw_Validate(const bool bPressed)
{
	return true;
}

void APX_Character::ServerEndDraw_Implementation(const bool bPressed)
{
	if (!HasAuthority()) return;
	if (bPressed)	return;			// Invlaid Function call
	if (!bIsAiming) return;			// Duplicated Function call

	//UE_LOG(LogTemp, Log, TEXT("Server End Draw.. Value : %s"), bPressed ? TEXT("true") : TEXT("false"));
	bIsDrawing = bPressed;
	DrawProgress = 0.f;
}

bool APX_Character::ServerUpdateDrawProgress_Validate(float Progress)
{
	return true;
}
void APX_Character::ServerUpdateDrawProgress_Implementation(float Progress)
{
	if (!HasAuthority()) return;
	if (!bIsAiming || !bIsDrawing) return;

	DrawProgress = FMath::Clamp(Progress, 0.f, 1.f);
}