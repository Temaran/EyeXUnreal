#include "EyeXEyetrackingPrivatePCH.h"

#include <limits>
#include "EyeXPlayerControllerBase.h"

DEFINE_LOG_CATEGORY_STATIC(GameLog, All, All);

//Constructor/Initializer
AEyeXPlayerControllerBase::AEyeXPlayerControllerBase(const FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
	AEyeXPlayerControllerBase::StaticClass();

	IEyeXEyetracking::Get().SetSceneViewProvider(this);
}

FSceneView* AEyeXPlayerControllerBase::GetSceneView()
{
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);

	auto ViewFamilyArguments = FSceneViewFamily::ConstructionValues(LocalPlayer->ViewportClient->Viewport, GetWorld()->Scene, LocalPlayer->ViewportClient->EngineShowFlags);
	ViewFamilyArguments.SetRealtimeUpdate(true);
	FSceneViewFamilyContext ViewFamily(ViewFamilyArguments);

	// Calculate a view where the player is to update the streaming from the players start location
	FVector ViewLocation;
	FRotator ViewRotation;
	return LocalPlayer->CalcSceneView(&ViewFamily, /*out*/ ViewLocation, /*out*/ ViewRotation, LocalPlayer->ViewportClient->Viewport);
}