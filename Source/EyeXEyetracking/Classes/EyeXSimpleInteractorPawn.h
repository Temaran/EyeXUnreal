#pragma once

#include "GameFramework/Pawn.h"
#include "EyeXSimpleInteractorPawn.generated.h"

/**
 * Placeable Pawn Example that receives Eyetracking input.
 * This is only an example of how to set it up, don't use this as an example of how to do gaze interaction ;)
 */

UCLASS()
class AEyeXSimpleInteractorPawn : public APawn
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Destroyed() OVERRIDE;
	virtual void BeginPlay() OVERRIDE;
	virtual void Tick(float DeltaTime) OVERRIDE;

	virtual void FocusedObjectChanged(const int& NewFocusedId);

private:
	ActivatableRegion* MyRegion;
	bool IsFocused;
	float CurScalingAlpha;
	FVector BaseScale;
	FVector FocusedScale;

	void UpdateRegion();
	void UpdateFocus(float DeltaTime);
};


