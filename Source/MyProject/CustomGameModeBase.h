#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CustomGameModeBase.generated.h"

class AEnvironmentSettings;

/**
 * 
 */
UCLASS()
class MYPROJECT_API ACustomGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	ACustomGameModeBase();


	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;
private:

	AEnvironmentSettings* environment=nullptr;

};
