// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "EnvironmentSettings.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API AEnvironmentSettings : public AWorldSettings
{
	GENERATED_BODY()

	AEnvironmentSettings();

public:
	FVector GetLightPosition() const;
	FVector GetWindDirection() const;
	float GetWindPower() const;
	float GetTemperature() const;
	float GetMoisture() const;
	float GetSoilAcidity() const;

private:
	UPROPERTY(EditAnywhere, Category = "EnvironmentSettings")
		FVector lightPosition;

	UPROPERTY(EditAnywhere, Category = "EnvironmentSettings")
		FVector windDirection;

	UPROPERTY(EditAnywhere, Category = "EnvironmentSettings")
		float windPower;

	UPROPERTY(EditAnywhere, Category = "EnvironmentSettings")
		float temperature;

	UPROPERTY(EditAnywhere, Category = "EnvironmentSettings")
		float moisture;

	UPROPERTY(EditAnywhere, Category = "EnvironmentSettings")
		float soilAcidity;
};
