#pragma once

#include "GameFramework/Pawn.h"
#include "EyeXSimpleGazePawn.generated.h"

/**
 * Placeable Pawn Example that receives Eyetracking input.
 * This is only an example of how to set it up, don't use this as an example of how to do gaze interaction ;)
 */

UCLASS(Blueprintable, BlueprintType)
class EYEXEYETRACKING_API AEyeXSimpleGazePawn : public APawn
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Destroyed() OVERRIDE;
	virtual void BeginPlay() OVERRIDE;
	virtual void Tick(float DeltaTime) OVERRIDE;

	UFUNCTION(BlueprintCallable, Category = Eyetracking)
	virtual void OnGazeData(const FVector2D& gazePoint);

private:
	FIntPoint CachedGazePoint;
};


