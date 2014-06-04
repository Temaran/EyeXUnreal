#include "EyeXEyetrackingPrivatePCH.h"

#include "Engine.h"
#include "GameFramework/Pawn.h"
#include "EyeXSimpleInteractorPawn.h"

DEFINE_LOG_CATEGORY_STATIC(GameLog, All, All);

//Constructor/Initializer
AEyeXSimpleInteractorPawn::AEyeXSimpleInteractorPawn(const FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEyeXSimpleInteractorPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AEyeXSimpleInteractorPawn::Destroyed()
{
	Super::Destroyed();
}

void AEyeXSimpleInteractorPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}