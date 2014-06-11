#include "EyeXEyetrackingPrivatePCH.h"

#include "GameFramework/Pawn.h"
#include "EyeXSimpleInteractorPawn.h"

DEFINE_LOG_CATEGORY_STATIC(GameLog, All, All);

//Constructor/Initializer
AEyeXSimpleInteractorPawn::AEyeXSimpleInteractorPawn(const FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
	AEyeXSimpleInteractorPawn::StaticClass();
	PrimaryActorTick.bCanEverTick = true;
	IsFocused = false;
}

void AEyeXSimpleInteractorPawn::BeginPlay()
{
	Super::BeginPlay();
	IEyeXEyetracking::Get().AddInteractor(*this);

	BaseScale = GetActorScale();
	FocusedScale = BaseScale * 2;
}

void AEyeXSimpleInteractorPawn::Destroyed()
{
	Super::Destroyed();

	IEyeXEyetracking::Get().RemoveInteractor(*this);
}


void AEyeXSimpleInteractorPawn::GetBounds(FVector& Center, FVector& Extents) const
{
	GetActorBounds(false, Center, Extents);
}

void AEyeXSimpleInteractorPawn::GotFocus()
{
	IsFocused = true;
}

void AEyeXSimpleInteractorPawn::LostFocus()
{
	IsFocused = false;
}

void AEyeXSimpleInteractorPawn::Activate()
{

}

/**
* This is where we will update the interactor bounding box for the pawn.
* In this example we will do this by projecting the extent points and selecting the most extreme ones to form the projected bounding box
*/
void AEyeXSimpleInteractorPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsFocused)
		CurScalingAlpha += DeltaSeconds;
	else
		CurScalingAlpha -= DeltaSeconds;

	CurScalingAlpha = FMath::Clamp(CurScalingAlpha, 0.0f, 1.0f);
	SetActorScale3D(FMath::Lerp<FVector, float>(BaseScale, FocusedScale, CurScalingAlpha));
}