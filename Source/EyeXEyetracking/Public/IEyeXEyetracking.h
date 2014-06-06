// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Created by Fredrik Lindh (Temaran)
// Contact me at: temaran (at) gmail (dot) com
// Last changed: 2014-05-17
// You're free to do whatever you want with the code except claiming it's you who wrote it.
// I would also appreciate it if you kept this file header as a thank you for the code :)

#pragma once

#include "ModuleManager.h"
#include "eyex/EyeX.h"
#include "EyeXEyetrackingTypes.h"

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules 
 * within this plugin.
 */
class IEyeXEyetracking : public IModuleInterface
{
public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IEyeXEyetracking& Get()
	{
		return FModuleManager::LoadModuleChecked< IEyeXEyetracking >( "EyeXEyetracking" );
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "EyeXEyetracking" );
	}


	/**
	Public API
	*/
	
	DECLARE_EVENT_OneParam(IEyeXEyetracking, FNewGazeDataEvent, const FVector2D&);   //parameter is the gaze point
	virtual FNewGazeDataEvent& OnNewGazeData() = 0;

	DECLARE_EVENT_OneParam(IEyeXEyetracking, FStatusChangedEvent, const bool&);   //parameter is whether it is connected or not
	virtual FStatusChangedEvent& OnStatusChanged() = 0;

	DECLARE_EVENT_OneParam(IEyeXEyetracking, FFocusedRegionChangedEvent, const int&);    //parameter is the new focused id
	virtual FFocusedRegionChangedEvent& OnFocusedRegionChanged() = 0;

	DECLARE_EVENT_OneParam(IEyeXEyetracking, FRegionActivatedEvent, const int&);  //parameter is the id
	virtual FRegionActivatedEvent& OnRegionActivated() = 0;
	
	/**
	 * When defining a new Activatable region, use this function to get a guaranteed unique id for it
	 */
	virtual int GetNextUniqueRegionId() = 0;
	virtual void AddActivatableRegion(ActivatableRegion& newRegions) = 0;
	virtual void RemoveActivatableRegion(const ActivatableRegion& region) = 0;

	// triggers an activation ("Direct Click").
	// use this method if you want to bind the click command to a key other than the one used by 
	// the EyeX Engine -- or to something other than a key press event.
	virtual void TriggerActivation() = 0;
};
