#pragma once

#include "GameFramework/Pawn.h"
#include "EyeXInteractorInterface.h"
#include "EyeXSimpleInteractorPawn.generated.h"

/**
 * Placeable Pawn Example that receives Eyetracking input.
 * This is only an example of how to set it up, don't use this as an example of how to do gaze interaction ;)
 */

UCLASS(Blueprintable, BlueprintType)
class EYEXEYETRACKING_API AEyeXSimpleInteractorPawn : public APawn, public IEyeXInteractorInterface
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Destroyed() OVERRIDE;
	virtual void BeginPlay() OVERRIDE;
	virtual void Tick(float DeltaTime) OVERRIDE;
	
public:
	//IEyeXInteractorInterface
	virtual void GetBounds(FVector& Center, FVector& Extents) const OVERRIDE;
	virtual void GotFocus() OVERRIDE;
	virtual void LostFocus() OVERRIDE;
	virtual void Activate() OVERRIDE;

private:
	bool IsFocused;
	float CurScalingAlpha;
	FVector BaseScale;
	FVector FocusedScale;
};


