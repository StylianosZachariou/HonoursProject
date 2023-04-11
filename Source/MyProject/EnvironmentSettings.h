#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "EnvironmentSettings.generated.h"

//Environment affecting trees
UCLASS()
class MYPROJECT_API AEnvironmentSettings : public AWorldSettings
{
	GENERATED_BODY()

public:

	//Getters
	FVector GetLightPosition() const;
	FVector GetWindDirection() const;
	float GetWindPower() const;
	float GetTemperature() const;
	float GetMoisture() const;
	float GetSoilAcidity() const;

private:

	//Environmental Factors
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
