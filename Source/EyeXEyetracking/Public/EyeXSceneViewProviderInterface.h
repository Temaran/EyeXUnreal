#include "EyeXSceneViewProviderInterface.generated.h"

#pragma once

/** Interface for actors that are interactable */
UINTERFACE()
class UEyeXSceneViewProviderInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class IEyeXSceneViewProviderInterface
{
	GENERATED_IINTERFACE_BODY()

public:
	virtual FSceneView* GetSceneView() = 0;
};
