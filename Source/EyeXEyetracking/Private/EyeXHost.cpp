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

#include "EyeXEyetrackingPrivatePCH.h"
#include "EyeXHost.h"

DEFINE_LOG_CATEGORY_STATIC(EyetrackingLog, All, All);

FEyeXHost::FEyeXHost()
{
	// initialize the EyeX Engine client library.
	txInitializeSystem(TX_SYSTEMCOMPONENTOVERRIDEFLAG_NONE, nullptr, nullptr, nullptr);

	// create a context and register event handlers.
	txCreateContext(&_context, TX_FALSE);
	RegisterConnectionStateChangedHandler();
	RegisterQueryHandler();
	RegisterEventHandler();

	if (txEnableConnection(_context) != TX_RESULT_OK)
	{
		StatusChangedEvent.Broadcast(false);
	}
}

FEyeXHost::~FEyeXHost()
{
	if (_context != TX_EMPTY_HANDLE)
	{
		// shut down, then release the context.
		txShutdownContext(_context, TX_CLEANUPTIMEOUT_DEFAULT, TX_FALSE);
		txReleaseContext(&_context);
	}
}

void FEyeXHost::SetActivatableRegions(const std::vector<ActivatableRegion>& regions)
{
	std::lock_guard<std::mutex> lock(_mutex);

	_regions.assign(regions.begin(), regions.end());
}

void FEyeXHost::TriggerActivation()
{
	TX_HANDLE command(TX_EMPTY_HANDLE);
	txCreateActionCommand(_context, &command, TX_ACTIONTYPE_ACTIVATE);
	txExecuteCommandAsync(command, NULL, NULL);
	txReleaseObject(&command);
}

void FEyeXHost::OnEngineConnectionStateChanged(TX_CONNECTIONSTATE connectionState)
{
	// note the use of the asynchronous PostMessage function to marshal the event to the main thread.
	// (this callback function is typically invoked on a worker thread.)
	switch (connectionState)
	{
	case TX_CONNECTIONSTATE::TX_CONNECTIONSTATE_CONNECTED:
		StatusChangedEvent.Broadcast(true);
		break;

	case TX_CONNECTIONSTATE::TX_CONNECTIONSTATE_DISCONNECTED:
	case TX_CONNECTIONSTATE::TX_CONNECTIONSTATE_TRYINGTOCONNECT:
	case TX_CONNECTIONSTATE::TX_CONNECTIONSTATE_SERVERVERSIONTOOLOW:
	case TX_CONNECTIONSTATE::TX_CONNECTIONSTATE_SERVERVERSIONTOOHIGH:
		StatusChangedEvent.Broadcast(false);
		break;

	default:
		break;
	}
}

bool FEyeXHost::RegisterConnectionStateChangedHandler()
{
	auto connectionStateChangedTrampoline = [](TX_CONNECTIONSTATE connectionState, TX_USERPARAM userParam)
	{
		static_cast<FEyeXHost*>(userParam)->OnEngineConnectionStateChanged(connectionState);
	};

	bool success = txRegisterConnectionStateChangedHandler(_context, &_connectionStateChangedTicket, connectionStateChangedTrampoline, this) == TX_RESULT_OK;
	return success;
}

bool FEyeXHost::RegisterQueryHandler()
{
	auto queryHandlerTrampoline = [](TX_CONSTHANDLE hObject, TX_USERPARAM userParam)
	{
		static_cast<FEyeXHost*>(userParam)->HandleQuery(hObject);
	};

	bool success = txRegisterQueryHandler(_context, &_queryHandlerTicket, queryHandlerTrampoline, this) == TX_RESULT_OK;
	return success;
}

bool FEyeXHost::RegisterEventHandler()
{
	auto eventHandlerTrampoline = [](TX_CONSTHANDLE hObject, TX_USERPARAM userParam)
	{
		static_cast<FEyeXHost*>(userParam)->HandleEvent(hObject);
	};

	bool success = txRegisterEventHandler(_context, &_eventHandlerTicket, eventHandlerTrampoline, this) == TX_RESULT_OK;
	return success;
}

void FEyeXHost::HandleQuery(TX_CONSTHANDLE hAsyncData)
{
	std::lock_guard<std::mutex> lock(_mutex);

	// NOTE. This method will fail silently if, for example, the connection is lost before the snapshot has been committed, 
	// or if we run out of memory. This is by design, because there is nothing we can do to recover from these errors anyway.

	TX_HANDLE hQuery = TX_EMPTY_HANDLE;
	txGetAsyncDataContent(hAsyncData, &hQuery);

	const int bufferSize = 20;
	TX_CHAR stringBuffer[bufferSize];

	// read the query bounds from the query, that is, the area on the screen that the query concerns.
	// the query region is always rectangular.
	TX_HANDLE hBounds(TX_EMPTY_HANDLE);
	txGetQueryBounds(hQuery, &hBounds);
	TX_REAL pX, pY, pWidth, pHeight;
	txGetRectangularBoundsData(hBounds, &pX, &pY, &pWidth, &pHeight);
	txReleaseObject(&hBounds);
	EyeXRect queryBounds(pX, pY, pX + pWidth, pY + pHeight);

	// create a new snapshot with the same window id and bounds as the query.
	TX_HANDLE hSnapshot(TX_EMPTY_HANDLE);
	txCreateSnapshotForQuery(hQuery, &hSnapshot);

	TX_CHAR windowIdString[bufferSize];
	sprintf_s(windowIdString, bufferSize, "%p", GetActiveWindow());

	if (QueryIsForWindowId(hQuery, windowIdString))
	{
		// define options for our activatable regions: no, we don't want tentative focus events.
		TX_ACTIVATABLEPARAMS params;
		params.EnableTentativeFocus = false;

		// iterate through all regions and create interactors for those that overlap with the query bounds.
		for (auto region : _regions)
		{
			EyeXRect regionBounds(region.bounds.left, region.bounds.top, region.bounds.right, region.bounds.bottom);

			if (EyeXRect::Intersects(queryBounds, regionBounds))
			{
				TX_HANDLE hInteractor(TX_EMPTY_HANDLE);

				sprintf_s(stringBuffer, bufferSize, "%d", region.id);

				TX_RECT bounds;
				bounds.X = region.bounds.left;
				bounds.Y = region.bounds.top;
				bounds.Width = region.bounds.right - region.bounds.left;
				bounds.Height = region.bounds.bottom - region.bounds.top;

				txCreateRectangularInteractor(hSnapshot, &hInteractor, stringBuffer, &bounds, TX_LITERAL_ROOTID, windowIdString);
				txSetActivatableBehavior(hInteractor, &params);

				txReleaseObject(&hInteractor);
			}
		}
	}

	txCommitSnapshotAsync(hSnapshot, OnSnapshotCommitted, nullptr);
	txReleaseObject(&hSnapshot);
	txReleaseObject(&hQuery);
}

void FEyeXHost::HandleEvent(TX_CONSTHANDLE hAsyncData)
{
	TX_HANDLE hEvent = TX_EMPTY_HANDLE;
	txGetAsyncDataContent(hAsyncData, &hEvent);

	// NOTE. Uncomment the following line of code to view the event object. The same function can be used with any interaction object.
	//OutputDebugStringA(txDebugObject(hEvent));

	// read the interactor ID from the event.
	const int bufferSize = 20;
	TX_CHAR stringBuffer[bufferSize];
	TX_SIZE idLength(bufferSize);
	if (txGetEventInteractorId(hEvent, stringBuffer, &idLength) == TX_RESULT_OK)
	{
		int interactorId = atoi(stringBuffer);

		HandleActivatableEvent(hEvent, interactorId);
	}

	txReleaseObject(&hEvent);
}

void FEyeXHost::HandleActivatableEvent(TX_HANDLE hEvent, int interactorId)
{
	TX_HANDLE hActivatable(TX_EMPTY_HANDLE);
	if (txGetEventBehavior(hEvent, &hActivatable, TX_INTERACTIONBEHAVIORTYPE_ACTIVATABLE) == TX_RESULT_OK)
	{
		TX_ACTIVATABLEEVENTTYPE eventType;
		if (txGetActivatableEventType(hActivatable, &eventType) == TX_RESULT_OK)
		{
			if (eventType == TX_ACTIVATABLEEVENTTYPE_ACTIVATED)
			{
				OnActivated(hActivatable, interactorId);
			}
			else if (eventType == TX_ACTIVATABLEEVENTTYPE_ACTIVATIONFOCUSCHANGED)
			{
				OnActivationFocusChanged(hActivatable, interactorId);
			}
		}

		txReleaseObject(&hActivatable);
	}
}

void FEyeXHost::OnActivationFocusChanged(TX_HANDLE hBehavior, int interactorId)
{
	TX_ACTIVATIONFOCUSCHANGEDEVENTPARAMS eventData;
	if (txGetActivationFocusChangedEventParams(hBehavior, &eventData) == TX_RESULT_OK)
	{
		if (eventData.HasActivationFocus)
		{
			FocusedRegionChangedEvent.Broadcast(interactorId);
		}
		else
		{
			FocusedRegionChangedEvent.Broadcast(-1);
		}
	}
}

void FEyeXHost::OnActivated(TX_HANDLE hBehavior, int interactorId)
{
	RegionActivatedEvent.Broadcast(interactorId);
}

void TX_CALLCONVENTION FEyeXHost::OnSnapshotCommitted(TX_CONSTHANDLE hAsyncData, TX_USERPARAM param)
{
	// check the result code using an assertion.
	// this will catch validation errors and runtime errors in debug builds. in release builds it won't do anything.

	TX_RESULT result = TX_RESULT_UNKNOWN;
	txGetAsyncDataResultCode(hAsyncData, &result);
	if (result != TX_RESULT_OK && result != TX_RESULT_CANCELLED)
		UE_LOG(EyetrackingLog, Error, TEXT("Unexpected result! %d"), (int)result);
}

bool FEyeXHost::QueryIsForWindowId(TX_HANDLE hQuery, const TX_CHAR* windowId)
{
	const int bufferSize = 20;
	TX_CHAR buffer[bufferSize];

	TX_SIZE count;
	if (TX_RESULT_OK == txGetQueryWindowIdCount(hQuery, &count))
	{
		for (int i = 0; i < count; i++)
		{
			TX_SIZE size = bufferSize;
			if (TX_RESULT_OK == txGetQueryWindowId(hQuery, i, buffer, &size))
			{
				if (0 == strcmp(windowId, buffer))
				{
					return true;
				}
			}
		}
	}

	return false;
}
