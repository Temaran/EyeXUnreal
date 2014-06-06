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

#include "EyeXEyetrackingPrivatePCH.h"
#include <mutex>
#include "eyex/EyeX.h"

class FEyeXEyetracking : public IEyeXEyetracking
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() OVERRIDE;
	virtual void ShutdownModule() OVERRIDE;
	
public:
	DECLARE_DERIVED_EVENT(FEyeXEyetracking, IEyeXEyetracking::FNewGazeDataEvent, FNewGazeDataEvent);   //parameter is the gaze point
	virtual FNewGazeDataEvent& OnNewGazeData() OVERRIDE { return NewGazeDataEvent; }

	DECLARE_DERIVED_EVENT(FEyeXEyetracking, IEyeXEyetracking::FStatusChangedEvent, FStatusChangedEvent);   //parameter is whether it is connected or not
	virtual FStatusChangedEvent& OnStatusChanged() OVERRIDE{ return StatusChangedEvent; }

	DECLARE_DERIVED_EVENT(FEyeXEyetracking, IEyeXEyetracking::FFocusedRegionChangedEvent, FFocusedRegionChangedEvent);    //parameter is the id
	virtual FFocusedRegionChangedEvent& OnFocusedRegionChanged() OVERRIDE{ return FocusedRegionChangedEvent; }

	DECLARE_DERIVED_EVENT(FEyeXEyetracking, IEyeXEyetracking::FRegionActivatedEvent, FRegionActivatedEvent);  //parameter is the id
	virtual FRegionActivatedEvent& OnRegionActivated() OVERRIDE{ return RegionActivatedEvent; }

	virtual int GetNextUniqueRegionId() OVERRIDE;

	virtual void AddActivatableRegion(ActivatableRegion& newRegions) OVERRIDE;
	virtual void RemoveActivatableRegion(const ActivatableRegion& region) OVERRIDE;
	virtual void TriggerActivation() OVERRIDE;

private:
	// registers handlers for notifications from the engine.
	bool RegisterConnectionStateChangedHandler();
	bool RegisterQueryHandler();
	bool RegisterEventHandler();

	// event handlers.
	void OnEngineConnectionStateChanged(TX_CONNECTIONSTATE connectionState);
	void HandleQuery(TX_CONSTHANDLE hAsyncData);
	void HandleEvent(TX_CONSTHANDLE hAsyncData);
	void HandleActivatableEvent(TX_HANDLE hEvent, int interactorId);
	void OnActivationFocusChanged(TX_HANDLE hBehavior, int interactorId);
	void OnActivated(TX_HANDLE hBehavior, int interactorId);
	BOOL InitializeGlobalInteractorSnapshot(TX_CONTEXTHANDLE hContext);
	void OnGazeDataEvent(TX_HANDLE hGazeDataBehavior);

	// callback function invoked when a snapshot has been committed.
	static void TX_CALLCONVENTION OnSnapshotCommitted(TX_CONSTHANDLE hAsyncData, TX_USERPARAM param);

	static bool QueryIsForWindowId(TX_HANDLE hQuery, const TX_CHAR* windowId);

	// mutex protecting the state of the object from race conditions caused by multiple threads.
	// (for example, a call to SetActivatableRegions from the main thread while the HandleQuery 
	// method is iterating through the regions on a worker thread.)
	std::mutex _mutex;
	std::vector<ActivatableRegion> _regions;
	TX_CONTEXTHANDLE _context;
	TX_TICKET _connectionStateChangedTicket;
	TX_TICKET _queryHandlerTicket;
	TX_TICKET _eventHandlerTicket;
	TX_STRING _globalInteractorId = "EyeXUnreal";
	TX_HANDLE _hGlobalInteractorSnapshot = TX_EMPTY_HANDLE;

	FNewGazeDataEvent NewGazeDataEvent;
	FStatusChangedEvent StatusChangedEvent;
	FFocusedRegionChangedEvent FocusedRegionChangedEvent;
	FRegionActivatedEvent RegionActivatedEvent;
};

IMPLEMENT_MODULE(FEyeXEyetracking, EyeXEyetracking)
