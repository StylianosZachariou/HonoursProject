// Copyright Epic Games, Inc. All Rights Reserved.


#include "CustomGameModeBase.h"
#include "EnvironmentSettings.h"

ACustomGameModeBase::ACustomGameModeBase()
{
	environment = Cast<AEnvironmentSettings>(GetWorldSettings());
}

void ACustomGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

}

void ACustomGameModeBase::StartPlay()
{

	Super::StartPlay();
}
