#include "EyeXEyetrackingPrivatePCH.h"

#include "GameFramework/Pawn.h"
#include "EyeXSimpleGazePawn.h"

DEFINE_LOG_CATEGORY_STATIC(GameLog, All, All);

//Constructor/Initializer
AEyeXSimpleGazePawn::AEyeXSimpleGazePawn(const FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEyeXSimpleGazePawn::BeginPlay()
{
	Super::BeginPlay();
	IEyeXEyetracking::Get().OnNewGazeData().AddUObject(this, &AEyeXSimpleGazePawn::OnGazeData);
}

void AEyeXSimpleGazePawn::Destroyed()
{
	Super::Destroyed();
	IEyeXEyetracking::Get().OnNewGazeData().RemoveUObject(this, &AEyeXSimpleGazePawn::OnGazeData);
}

void AEyeXSimpleGazePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	auto playerController = Cast<APlayerController>(Controller);
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(playerController->Player);
	FVector WorldOrigin;
	FVector WorldDirection;
	FHitResult HitResult;

	auto ViewFamilyArguments = FSceneViewFamily::ConstructionValues(LocalPlayer->ViewportClient->Viewport, GetWorld()->Scene, LocalPlayer->ViewportClient->EngineShowFlags);
	ViewFamilyArguments.SetRealtimeUpdate(true);
	FSceneViewFamilyContext ViewFamily(ViewFamilyArguments);

	// Calculate a view where the player is to update the streaming from the players start location
	FVector ViewLocation;
	FRotator ViewRotation;
	FSceneView* SceneView = LocalPlayer->CalcSceneView(&ViewFamily, /*out*/ ViewLocation, /*out*/ ViewRotation, LocalPlayer->ViewportClient->Viewport);

	SceneView->DeprojectFVector2D(CachedGazePoint, WorldOrigin, WorldDirection);

	auto params = FCollisionQueryParams("ClickableTrace");
	params.AddIgnoredActor(this);
	auto success = GetWorld()->LineTraceSingle(HitResult, WorldOrigin + WorldDirection * 100, WorldOrigin + WorldDirection * 100000.f, ECollisionChannel::ECC_WorldStatic, params);

	if (success)
	{
		SetActorLocation(HitResult.Location);
	}
}

void AEyeXSimpleGazePawn::OnGazeData(const FVector2D& GazePoint)
{
	bool success = false;
	auto playerController = Cast<APlayerController>(Controller);
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(playerController->Player);

	if (LocalPlayer == NULL || LocalPlayer->ViewportClient == NULL || LocalPlayer->ViewportClient->Viewport == NULL)
		return;

	FIntPoint MyNewPoint;
	FIntPoint OutPoint = FIntPoint(GazePoint.X, GazePoint.Y);
	success = LocalPlayer->ViewportClient->Viewport->OperatingSystemPixelToViewportPixel(&OutPoint, MyNewPoint);
	UE_LOG(GameLog, Log, TEXT("pos: %i %i"), MyNewPoint.X, MyNewPoint.Y);

	if (!success)
		return;

	CachedGazePoint.X = MyNewPoint.X;
	CachedGazePoint.Y = MyNewPoint.Y;
}