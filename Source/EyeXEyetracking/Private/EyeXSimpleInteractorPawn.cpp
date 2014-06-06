#include "EyeXEyetrackingPrivatePCH.h"

#include <limits>
#include "Engine.h"
#include "GameFramework/Pawn.h"
#include "EyeXSimpleInteractorPawn.h"

DEFINE_LOG_CATEGORY_STATIC(GameLog, All, All);

//Constructor/Initializer
AEyeXSimpleInteractorPawn::AEyeXSimpleInteractorPawn(const FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;
	IsFocused = false;
}

void AEyeXSimpleInteractorPawn::BeginPlay()
{
	Super::BeginPlay();
	MyRegion = new ActivatableRegion(IEyeXEyetracking::Get().GetNextUniqueRegionId(), RECT());
	IEyeXEyetracking::Get().AddActivatableRegion(*MyRegion);
	IEyeXEyetracking::Get().OnFocusedRegionChanged().AddUObject(this, &AEyeXSimpleInteractorPawn::FocusedObjectChanged);

	BaseScale = GetActorScale();
	FocusedScale = BaseScale * 2;
}

void AEyeXSimpleInteractorPawn::Destroyed()
{
	Super::Destroyed();
	IEyeXEyetracking::Get().RemoveActivatableRegion(*MyRegion);
	IEyeXEyetracking::Get().OnFocusedRegionChanged().RemoveUObject(this, &AEyeXSimpleInteractorPawn::FocusedObjectChanged);
	delete MyRegion;
}

void AEyeXSimpleInteractorPawn::FocusedObjectChanged(const int& NewFocusedId)
{
	IsFocused = (NewFocusedId == MyRegion->id);
}

/**
  * This is where we will update the interactor bounding box for the pawn.
  * In this example we will do this by projecting the extent points and selecting the most extreme ones to form the projected bounding box
  */
void AEyeXSimpleInteractorPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateRegion();
	UpdateFocus(DeltaSeconds);
}

void AEyeXSimpleInteractorPawn::UpdateRegion()
{
	FVector Origin;
	FVector Extents;
	GetActorBounds(false, Origin, Extents);

	auto playerController = Cast<APlayerController>(Controller);
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(playerController->Player);

	auto ViewFamilyArguments = FSceneViewFamily::ConstructionValues(LocalPlayer->ViewportClient->Viewport, GetWorld()->Scene, LocalPlayer->ViewportClient->EngineShowFlags);
	ViewFamilyArguments.SetRealtimeUpdate(true);
	FSceneViewFamilyContext ViewFamily(ViewFamilyArguments);

	// Calculate a view where the player is to update the streaming from the players start location
	FVector ViewLocation;
	FRotator ViewRotation;
	FSceneView* SceneView = LocalPlayer->CalcSceneView(&ViewFamily, /*out*/ ViewLocation, /*out*/ ViewRotation, LocalPlayer->ViewportClient->Viewport);

	FVector2D ExtentPoints[8];
	SceneView->WorldToPixel(Origin + FVector(Extents.X, Extents.Y, Extents.Z), ExtentPoints[0]); //Right Top Front
	SceneView->WorldToPixel(Origin + FVector(Extents.X, Extents.Y, -Extents.Z), ExtentPoints[1]); //Right Top Back
	SceneView->WorldToPixel(Origin + FVector(Extents.X, -Extents.Y, Extents.Z), ExtentPoints[2]); //Right Bottom Front
	SceneView->WorldToPixel(Origin + FVector(Extents.X, -Extents.Y, -Extents.Z), ExtentPoints[3]); //Right Bottom Back
	SceneView->WorldToPixel(Origin + FVector(-Extents.X, Extents.Y, Extents.Z), ExtentPoints[4]); //Left Top Front
	SceneView->WorldToPixel(Origin + FVector(-Extents.X, Extents.Y, -Extents.Z), ExtentPoints[5]); //Left Top Back
	SceneView->WorldToPixel(Origin + FVector(-Extents.X, -Extents.Y, Extents.Z), ExtentPoints[6]); //Left Bottom Front
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

	MyRegion->bounds.left = TopLeft.X;
	MyRegion->bounds.top = TopLeft.Y;
	MyRegion->bounds.right = BottomRight.X;
	MyRegion->bounds.bottom = BottomRight.Y;
}

void AEyeXSimpleInteractorPawn::UpdateFocus(float DeltaTime)
{
	if (IsFocused)
		CurScalingAlpha += DeltaTime;
	else
		CurScalingAlpha -= DeltaTime;

	CurScalingAlpha = FMath::Clamp(CurScalingAlpha, 0.0f, 1.0f);
	SetActorScale3D(FMath::Lerp<FVector, float>(BaseScale, FocusedScale, CurScalingAlpha));
}