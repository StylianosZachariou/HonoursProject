// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentSettings.h"

AEnvironmentSettings::AEnvironmentSettings()
{

}

FVector AEnvironmentSettings::GetLightPosition() const
{
	return lightPosition;
}

FVector AEnvironmentSettings::GetWindDirection() const
{
	return windDirection;
}

float AEnvironmentSettings::GetWindPower() const
{
	return windPower;
}

float AEnvironmentSettings::GetTemperature() const
{
	return temperature;
}

float AEnvironmentSettings::GetMoisture() const
{
	return moisture;
}

float AEnvironmentSettings::GetSoilAcidity() const
{
	return soilAcidity;
}
