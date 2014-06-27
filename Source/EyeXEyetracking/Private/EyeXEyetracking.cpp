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
#include "EyeXEyetracking.h"
#include <limits>

DEFINE_LOG_CATEGORY_STATIC(EyetrackingLog, All, All);

float Inf = std::numeric_limits<float>::infinity();
const FVector2D FEyeXEyetracking::InfVector = FVector2D(Inf, Inf);

void FEyeXEyetracking::StartupModule()
{
	_sceneViewProvider = NULL;

	// initialize the EyeX Engine client library.
	txInitializeSystem(TX_SYSTEMCOMPONENTOVERRIDEFLAG_NONE, nullptr, nullptr, nullptr);

	// create a context and register event handlers.
	txCreateContext(&_context, TX_FALSE);
	InitializeGlobalInteractorSnapshot(_context);
	RegisterConnectionStateChangedHandler();
	RegisterQueryHandler();
	RegisterEventHandler();

	if (txEnableConnection(_context) != TX_RESULT_OK)
	{
		StatusChangedEvent.Broadcast(false);
	}

	for (int i = 0; i < NUM_GAZEPOINTS; i++)
	{
		GazePoints[i] = InfVector;
	}
	CurrentIndex = 0;
}

void FEyeXEyetracking::ShutdownModule()
{
	if (_context != TX_EMPTY_HANDLE)
	{
		txReleaseObject(&_hGlobalInteractorSnapshot);

		// shut down, then release the context.
		txShutdownContext(_context, TX_CLEANUPTIMEOUT_DEFAULT, TX_FALSE);
		txReleaseContext(&_context);
	}
	_sceneViewProvider = NULL;
}


BOOL FEyeXEyetracking::InitializeGlobalInteractorSnapshot(TX_CONTEXTHANDLE hContext)
{
	TX_HANDLE hInteractor = TX_EMPTY_HANDLE;
	TX_HANDLE hBehavior = TX_EMPTY_HANDLE;
	TX_GAZEPOINTDATAPARAMS params = { TX_GAZEPOINTDATAMODE_LIGHTLYFILTERED };
	BOOL success;

	success = txCreateGlobalInteractorSnapshot(
		hContext,
		_globalInteractorId,
		&_hGlobalInteractorSnapshot,
		&hInteractor) == TX_RESULT_OK;
	success &= txCreateInteractorBehavior(hInteractor, &hBehavior, TX_INTERACTIONBEHAVIORTYPE_GAZEPOINTDATA) == TX_RESULT_OK;
	success &= txSetGazePointDataBehaviorParams(hBehavior, &params) == TX_RESULT_OK;

	txReleaseObject(&hBehavior);
	txReleaseObject(&hInteractor);

	return success;
}

int FEyeXEyetracking::GetNextUniqueRegionId()
{
	static int IdCounter = 0;
	return ++IdCounter;
}

void FEyeXEyetracking::AddInteractor(IEyeXInteractorInterface& newInteractor)
{
	std::lock_guard<std::mutex> lock(_mutex);

	if (_interactorToId.Contains(&newInteractor))
	{
		UE_LOG(EyetrackingLog, Warning, TEXT("Interactor already added to the API!"));
		RemoveInteractor(newInteractor);
	}

	auto newId = GetNextUniqueRegionId();
	_interactorToId.Add(&newInteractor, newId);
	_idToInteractor.Add(newId, &newInteractor);
}

void FEyeXEyetracking::RemoveInteractor(IEyeXInteractorInterface& interactorToRemove)
{
	std::lock_guard<std::mutex> lock(_mutex);

	if (_interactorToId.Contains(&interactorToRemove))
	{
		auto id = _interactorToId[&interactorToRemove];
		_interactorToId.Remove(&interactorToRemove);
		_idToInteractor.Remove(id);
	}
}

void FEyeXEyetracking::SetSceneViewProvider(IEyeXSceneViewProviderInterface* NewProvider)
{
	_sceneViewProvider = NewProvider;
}

IEyeXInteractorInterface* FEyeXEyetracking::GetFocusedInteractor()
{
	return _focusedInteractor;
}

void FEyeXEyetracking::TriggerActivation()
{
	TX_HANDLE command(TX_EMPTY_HANDLE);
	txCreateActionCommand(_context, &command, TX_ACTIONTYPE_ACTIVATE);
	txExecuteCommandAsync(command, NULL, NULL);
	txReleaseObject(&command);
}

void FEyeXEyetracking::OnEngineConnectionStateChanged(TX_CONNECTIONSTATE connectionState)
{
	// note the use of the asynchronous PostMessage function to marshal the event to the main thread.
	// (this callback function is typically invoked on a worker thread.)
	switch (connectionState)
	{
	case TX_CONNECTIONSTATE::TX_CONNECTIONSTATE_CONNECTED:
		StatusChangedEvent.Broadcast(true);
		txCommitSnapshotAsync(_hGlobalInteractorSnapshot, OnSnapshotCommitted, NULL);
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

bool FEyeXEyetracking::RegisterConnectionStateChangedHandler()
{
	auto connectionStateChangedTrampoline = [](TX_CONNECTIONSTATE connectionState, TX_USERPARAM userParam)
	{
		static_cast<FEyeXEyetracking*>(userParam)->OnEngineConnectionStateChanged(connectionState);
	};

	bool success = txRegisterConnectionStateChangedHandler(_context, &_connectionStateChangedTicket, connectionStateChangedTrampoline, this) == TX_RESULT_OK;
	return success;
}

bool FEyeXEyetracking::RegisterQueryHandler()
{
	auto queryHandlerTrampoline = [](TX_CONSTHANDLE hObject, TX_USERPARAM userParam)
	{
		static_cast<FEyeXEyetracking*>(userParam)->HandleQuery(hObject);
	};

	bool success = txRegisterQueryHandler(_context, &_queryHandlerTicket, queryHandlerTrampoline, this) == TX_RESULT_OK;
	return success;
}

bool FEyeXEyetracking::RegisterEventHandler()
{
	auto eventHandlerTrampoline = [](TX_CONSTHANDLE hObject, TX_USERPARAM userParam)
	{
		static_cast<FEyeXEyetracking*>(userParam)->HandleEvent(hObject);
	};

	bool success = txRegisterEventHandler(_context, &_eventHandlerTicket, eventHandlerTrampoline, this) == TX_RESULT_OK;
	return success;
}

void FEyeXEyetracking::HandleQuery(TX_CONSTHANDLE hAsyncData)
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
		for (auto pair : _interactorToId)
		{
			FVector Origin;
			FVector Extents;
			
			pair.Key->GetBounds(Origin, Extents);

			FSceneView* SceneView = _sceneViewProvider->GetSceneView();			

			FVector2D ExtentPoints[8];
			SceneView->WorldToPixel(Origin + FVector(Extents.X, Extents.Y, Extents.Z), ExtentPoints[0]);	//Right Top Front
			SceneView->WorldToPixel(Origin + FVector(Extents.X, Extents.Y, -Extents.Z), ExtentPoints[1]);	//Right Top Back
			SceneView->WorldToPixel(Origin + FVector(Extents.X, -Extents.Y, Extents.Z), ExtentPoints[2]);	//Right Bottom Front
			SceneView->WorldToPixel(Origin + FVector(Extents.X, -Extents.Y, -Extents.Z), ExtentPoints[3]);	//Right Bottom Back
			SceneView->WorldToPixel(Origin + FVector(-Extents.X, Extents.Y, Extents.Z), ExtentPoints[4]);	//Left Top Front
			SceneView->WorldToPixel(Origin + FVector(-Extents.X, Extents.Y, -Extents.Z), ExtentPoints[5]);	//Left Top Back
			SceneView->WorldToPixel(Origin + FVector(-Extents.X, -Extents.Y, Extents.Z), ExtentPoints[6]);	//Left Bottom Front
			SceneView->WorldToPixel(Origin + FVector(-Extents.X, -Extents.Y, -Extents.Z), ExtentPoints[7]); //Left Bottom Back

			FVector2D TopLeft = FVector2D(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			FVector2D BottomRight = FVector2D(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

			for (auto Point : ExtentPoints)
			{
				if (Point.X < TopLeft.X)
					TopLeft.X = Point.X;
				else if (Point.X > BottomRight.X)
					BottomRight.X = Point.X;

				if (Point.Y < TopLeft.Y)
					TopLeft.Y = Point.Y;
				else if (Point.Y > BottomRight.Y)
					BottomRight.Y = Point.Y;
			}

			EyeXRect regionBounds(TopLeft.X, TopLeft.Y, BottomRight.X, BottomRight.Y);

			if (EyeXRect::Intersects(queryBounds, regionBounds))
			{
				TX_HANDLE hInteractor(TX_EMPTY_HANDLE);

				sprintf_s(stringBuffer, bufferSize, "%d", pair.Value);

				TX_RECT bounds;
				bounds.X = TopLeft.X;
				bounds.Y = TopLeft.Y;
				bounds.Width = BottomRight.X - TopLeft.X;
				bounds.Height = BottomRight.Y - TopLeft.Y;

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

void FEyeXEyetracking::HandleEvent(TX_CONSTHANDLE hAsyncData)
{
	TX_HANDLE hEvent = TX_EMPTY_HANDLE;
	TX_HANDLE hBehavior = TX_EMPTY_HANDLE;
	txGetAsyncDataContent(hAsyncData, &hEvent);

	// NOTE. Uncomment the following line of code to view the event object. The same function can be used with any interaction object.
	//OutputDebugStringA(txDebugObject(hEvent));
	
	if (txGetEventBehavior(hEvent, &hBehavior, TX_INTERACTIONBEHAVIORTYPE_GAZEPOINTDATA) == TX_RESULT_OK)
	{
		OnGazeDataEvent(hBehavior);
		txReleaseObject(&hBehavior);
	}

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

void FEyeXEyetracking::OnGazeDataEvent(TX_HANDLE hGazeDataBehavior)
{
	TX_GAZEPOINTDATAEVENTPARAMS eventParams;
	if (txGetGazePointDataEventParams(hGazeDataBehavior, &eventParams) == TX_RESULT_OK)
	{
		// Return if data is NaN
		if (FMath::IsNaN(eventParams.X) || FMath::IsNaN(eventParams.Y))
			return;

		FVector2D GazeData(eventParams.X, eventParams.Y);

		if (GEngine == NULL || GEngine->GameViewport == NULL || GEngine->GameViewport->Viewport == NULL)
			return;

		FIntPoint TransformedPoint;
		FIntPoint OsPoint = FIntPoint(GazeData.X, GazeData.Y);
		if (!GEngine->GameViewport->Viewport->OperatingSystemPixelToViewportPixel(&OsPoint, TransformedPoint))
			return;

		GazePoints[CurrentIndex] = TransformedPoint;
		CurrentIndex = (CurrentIndex + 1) % NUM_GAZEPOINTS;

		FVector WorldOrigin;
		FVector WorldDirection;

		FVector2D SumVector = FVector2D::ZeroVector;
		int NumValidGazePoints = 0;
		for (int i = 0; i < NUM_GAZEPOINTS; i++)
		{
			if (GazePoints[i] == InfVector) continue;
			NumValidGazePoints++;
			SumVector += GazePoints[i];
		}
		FVector2D AveragedPoint = SumVector / (float)NumValidGazePoints;
// 		if (NULL != _sceneViewProvider)
// 		{
// 			FSceneView* SceneView = _sceneViewProvider->GetSceneView();
// 			FVector2D TransformedPointVector(TransformedPoint.X, TransformedPoint.Y);
// 			SceneView->DeprojectFVector2D(TransformedPointVector, WorldOrigin, WorldDirection);
// 		}
// 		UE_LOG(EyetrackingLog, Log, TEXT("Gaze Data: (%.1f : %.1f) Transformed: (%s) Eye world pos: (%s) Eye world direction: (%s) timestamp %.0f ms\n"), eventParams.X, eventParams.Y, *TransformedPoint.ToString(), *WorldOrigin.ToString(), *WorldDirection.ToString(), eventParams.Timestamp);
 		NewGazeDataEvent.Broadcast(TransformedPoint, AveragedPoint, WorldOrigin, WorldDirection);
	}
	else
	{
		UE_LOG(EyetrackingLog, Error, TEXT("Failed to interpret gaze data event packet.\n"));
	}
}

void FEyeXEyetracking::HandleActivatableEvent(TX_HANDLE hEvent, int interactorId)
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

void FEyeXEyetracking::OnActivationFocusChanged(TX_HANDLE hBehavior, int interactorId)
{
	TX_ACTIVATIONFOCUSCHANGEDEVENTPARAMS eventData;
	if (txGetActivationFocusChangedEventParams(hBehavior, &eventData) == TX_RESULT_OK)
	{
		auto prevFocusedInteractor = _focusedInteractor;
		if (prevFocusedInteractor != nullptr)
			prevFocusedInteractor->LostFocus();

		if (eventData.HasActivationFocus)
		{
			_focusedInteractor = _idToInteractor[interactorId];
			if (prevFocusedInteractor != _focusedInteractor)
				_focusedInteractor->GotFocus();
		}
		else
		{
			_focusedInteractor = nullptr;
		}

		FocusedRegionChangedEvent.Broadcast(_focusedInteractor);
	}
}

void FEyeXEyetracking::OnActivated(TX_HANDLE hBehavior, int interactorId)
{
	auto interactor = _idToInteractor[interactorId];
	interactor->Activate();
	RegionActivatedEvent.Broadcast(*interactor);

}

void TX_CALLCONVENTION FEyeXEyetracking::OnSnapshotCommitted(TX_CONSTHANDLE hAsyncData, TX_USERPARAM param)
{
	// check the result code using an assertion.
	// this will catch validation errors and runtime errors in debug builds. in release builds it won't do anything.

	TX_RESULT result = TX_RESULT_UNKNOWN;
	txGetAsyncDataResultCode(hAsyncData, &result);
	if (result != TX_RESULT_OK && result != TX_RESULT_CANCELLED)
		UE_LOG(EyetrackingLog, Error, TEXT("Unexpected result! %d"), (int)result);
}

bool FEyeXEyetracking::QueryIsForWindowId(TX_HANDLE hQuery, const TX_CHAR* windowId)
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