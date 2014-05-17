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

#include <mutex>
#include "eyex/EyeX.h"

class FEyeXHost
{
public:
	struct EyeXRect
	{
	public:
		int Left;
		int Top;
		int Right;
		int Bottom;

		EyeXRect(int left, int top, int right, int bottom)
		{
			Left = left;
			Top = top;
			Right = right;
			Bottom = bottom;
		}

		static bool Intersects(const EyeXRect& A, const EyeXRect& B)
		{
			//  Segments A and B do not intersect when:
			//
			//       (left)   A     (right)
			//         o-------------o
			//  o---o        OR         o---o
			//    B                       B
			//
			//
			// We assume the A and B are well-formed rectangles.
			// i.e. (Top,Left) is above and to the left of (Bottom,Right)
			const bool bDoNotOverlap =
				B.Right < A.Left || A.Right < B.Left ||
				B.Bottom < A.Top || A.Bottom < B.Top;

			return !bDoNotOverlap;
		}
	};


	// Represents an activatable region, that is, one particular kind of interactor.
	struct ActivatableRegion
	{
		int id;
		RECT bounds;

		ActivatableRegion(int paramId, RECT paramBounds) : id(paramId), bounds(paramBounds) { }
	};

	FEyeXHost();
	virtual ~FEyeXHost();

	DECLARE_EVENT_OneParam(FEyeXHost, FStatusChangedEvent, const bool&);   //parameter is whether it is connected or not
	virtual FStatusChangedEvent& OnStatusChanged() { return StatusChangedEvent; }

	DECLARE_EVENT_OneParam(FEyeXHost, FFocusedRegionChangedEvent, const int&);    //parameter is the id
	virtual FFocusedRegionChangedEvent& OnFocusedRegionChanged() { return FocusedRegionChangedEvent; }

	DECLARE_EVENT_OneParam(FEyeXHost, FRegionActivatedEvent, const int&);  //parameter is the id
	virtual FRegionActivatedEvent& OnRegionActivated() { return RegionActivatedEvent; }


	// updates the collection (repository) of activatable regions.
	void SetActivatableRegions(const std::vector<ActivatableRegion>& regions);

	// triggers an activation ("Direct Click").
	// use this method if you want to bind the click command to a key other than the one used by 
	// the EyeX Engine -- or to something other than a key press event.
	void TriggerActivation();

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
	
	// private copy constructor and operator making the class non-copyable (declared but not implemented).
	FEyeXHost(const FEyeXHost&);
	FEyeXHost& operator = (const FEyeXHost&);

	FStatusChangedEvent StatusChangedEvent;
	FFocusedRegionChangedEvent FocusedRegionChangedEvent;
	FRegionActivatedEvent RegionActivatedEvent;
};
