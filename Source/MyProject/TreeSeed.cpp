#include "TreeSeed.h"
#include "EnvironmentSettings.h"

//Constructor
ATreeSeed::ATreeSeed():
TimeOfGrowth(100),
RateOfGrowth(1),
NodeMeshesRenderedPerFrame(4),
GrowingNodeMeshesRenderedPerFrame(4),
MeshGrowthRate(3),
LevelOfDetail(15),
MaximumAngleOfLightRotation(15),
CrownRadius(400),
TrunkHeight(150),
renderedNodeMeshes(0),
windOffset(FVector::Zero()),
lightRotation(FRotator::ZeroRotator),
newBranchesGenerated(0)
{
	PrimaryActorTick.bCanEverTick = true;
}

//Environment Application
void ATreeSeed::ApplyEnvironment()
{
	if (UseEnvironment)
	{
		//Get the Environment
		AEnvironmentSettings* environment = Cast<AEnvironmentSettings>(GetWorldSettings());

		//Calculate crown starting position using trunk height
		FVector crownStartPosition = GetActorLocation();
		crownStartPosition.Z += TrunkHeight;

		//Wind

		//Maximum wind power that affects Oak Trees
		float maxWindPower = 63;
		float minWindPower = 25;
		float windPower = FMath::Min(environment->GetWindPower(), maxWindPower);

		//If the wind is strong
		if (windPower > minWindPower)
		{
			//Calculate wind offset using direction
			FVector direction = environment->GetWindDirection();
			direction.Normalize();
			windOffset = direction * (windPower * CrownRadius / maxWindPower);
		}

		//Light
		if (UseLight)
		{
			//Calculate direction of growth towards light
			FVector lightGrowthDirection = environment->GetLightPosition() - crownStartPosition;
			lightGrowthDirection.Normalize();

			//Calculate rotation due to light
			lightRotation = lightGrowthDirection.ToOrientationRotator().Add(-90, 0, 0);
			lightRotation.Pitch = FMath::Min(lightRotation.Pitch, MaximumAngleOfLightRotation);
			lightRotation.Pitch = FMath::Max(lightRotation.Pitch, -MaximumAngleOfLightRotation);
		}

		//Rate of growth factors
//		RateOfGrowth = 1;

		//Calculate effect of moisture
		float moisturePercentage = environment->GetMoisture();
		if (moisturePercentage > 45)
		{
			RateOfGrowth -= FMath::Abs(45 - moisturePercentage) / 25;
		}
		else if (moisturePercentage < 20)
		{
			RateOfGrowth -= FMath::Abs(20 - moisturePercentage) / 10;
		}

		//Calculate effect of Soil Acidity
		float soilPH = environment->GetSoilAcidity();
		if (soilPH > 8)
		{
			RateOfGrowth -= FMath::Abs(8 - soilPH) / 3;
		}
		else if (soilPH < 4.5)
		{
			RateOfGrowth -= FMath::Abs(4.5 - soilPH) / 2;
		}

		//Calculate effect of Temperature
		float temperatureCelcius = environment->GetTemperature();
		if (temperatureCelcius > 30)
		{
			RateOfGrowth -= FMath::Abs(30 - temperatureCelcius) / 11;
		}
		else if (temperatureCelcius < 25)
		{
			RateOfGrowth -= FMath::Abs(25 - temperatureCelcius) / 21;
		}

		//If rate of growth is less than 0, set it to 0
		RateOfGrowth = FMath::Max(0, RateOfGrowth);
	}
}

//Mesh Calculations
void ATreeSeed::CalculateNodeMeshVerticesAndUV(float radius, FRotator rotation, FVector translation, TArray<FVector>& vertices, TArray<FVector2D>& uvs)
{
	for (int i = 0; i < LevelOfDetail+1; i++)
	{
		//Rotation perpendicular to nodes direction
		FTransform rotationTF = FTransform::Identity;
		rotationTF.SetRotation(rotation.Quaternion());

		//Translation to node's position
		FTransform translationTF = FTransform::Identity;
		translationTF.AddToTranslation(translation);

		// radial angle of the vertex
		float alpha = ((float)i / LevelOfDetail) * PI * 2.f;

		//Translation based on radial angle
		FTransform radialTranslationTF = FTransform::Identity;
		radialTranslationTF.AddToTranslation(FVector(FMath::Cos(alpha) * radius, FMath::Sin(alpha) * radius, 0));

		//Calculate total transform
		FTransform total = radialTranslationTF * rotationTF * translationTF;

		//Transform position of the vertex
		FVector pos = FVector::Zero();
		pos = total.TransformPosition(pos);
		pos -= GetActorLocation();

		//Calculate vertex UV
		FVector2D uv = FVector2D(i / LevelOfDetail, 0);

		//Add to arrays
		vertices.Add(pos);
		uvs.Add(uv);
	}
}
