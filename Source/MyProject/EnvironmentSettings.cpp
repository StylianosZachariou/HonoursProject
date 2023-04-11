#include "EnvironmentSettings.h"

///////////Getters///////////

//Light Position
FVector AEnvironmentSettings::GetLightPosition() const
{
	return lightPosition;
}

//Wind Direction
FVector AEnvironmentSettings::GetWindDirection() const
{
	return windDirection;
}

//Wind Power
float AEnvironmentSettings::GetWindPower() const
{
	return windPower;
}

//Temperature
float AEnvironmentSettings::GetTemperature() const
{
	return temperature;
}

//Moisture
float AEnvironmentSettings::GetMoisture() const
{
	return moisture;
}

//Soil Acidity
float AEnvironmentSettings::GetSoilAcidity() const
{
	return soilAcidity;
}
