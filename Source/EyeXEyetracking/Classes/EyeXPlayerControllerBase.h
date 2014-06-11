#pragma once

#include "GameFramework/PlayerController.h"
#include "EyeXSceneViewProviderInterface.h"
#include "EyeXPlayerControllerBase.generated.h"

/**
 * An example player controller base that provides bound calculation services for interactors.
 * Without using something like this, it is not possible to correctly calculate on-screen interactor bounds since we need a scene view to do this.
 */

UCLASS(Blueprintable, BlueprintType)
class EYEXEYETRACKING_API AEyeXPlayerControllerBase : public APlayerController, public IEyeXSceneViewProviderInterface
{
	GENERATED_UCLASS_BODY()

public:
	//IEyeXSceneViewProviderInterface

	virtual FSceneView* GetSceneView() OVERRIDE;
};
