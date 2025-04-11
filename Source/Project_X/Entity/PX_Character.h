// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "GameFramework/Character.h"
#include "PX_Character.generated.h"

UCLASS()
class PROJECT_X_API APX_Character : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APX_Character();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	
};
