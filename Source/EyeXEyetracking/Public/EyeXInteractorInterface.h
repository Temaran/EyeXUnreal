#include "EyeXInteractorInterface.generated.h"

#pragma once

/** Interface for actors that are interactable */
UINTERFACE()
class UEyeXInteractorInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class IEyeXInteractorInterface
{
	GENERATED_IINTERFACE_BODY()

public:
	/** Returns the bounds of the actor in worldspace
	  * @Center is the center of the actor in world space
	  * @Extents is the distance from the center to the edge in each axis, so the width would be Extents.X * 2 for example.
	  */
	virtual void GetBounds(FVector& OutCenter, FVector& OutExtents) const = 0;
	
	/**
	  * This is called when the interactor attains gaze focus
	  */
	virtual void GotFocus() = 0;
	
	/**
	  * This is called when the interactor loses gaze focus
	  */
	virtual void LostFocus() = 0;

	/**
	  * Optional activation function.
	  */
	virtual void Activate() = 0;
};
