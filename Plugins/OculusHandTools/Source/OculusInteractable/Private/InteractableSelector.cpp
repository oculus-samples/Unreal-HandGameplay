// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "InteractableSelector.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

ECollisionChannel AInteractableSelector::InteractableTraceChannel = ECC_GameTraceChannel1;
FName AInteractableSelector::BeamSource("Source");
FName AInteractableSelector::BeamTarget("Target");

AInteractableSelector::AInteractableSelector()
{
	// Property defaults
	bSelectorStartsActivated = false;
	NearFieldRadius = 10.0f;
	RaycastOffset = 0.0f;
	RaycastDistance = 1000.0f;
	RaycastAngleDegrees = 15.0f;
	RaycastStickinessAngleDegrees = 20.0f;
	DampeningFactor = 0.95f;
	AimingActorRotationRate = 0.1f;
	bRaycastDebugTrace = false;

	// State
	bAlignAimingActorWithHitNormal = false;
	bSelectorActivated = false;
	bAimingActorOwned = false;
	CandidatePreSelection = nullptr;
	CandidatePreSelectionTimeMs = 0.0f;
	SelectedInteractable = nullptr;

	PrimaryActorTick.bCanEverTick = true;

	// Root
	this->RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	ArrowComponent->SetupAttachment(RootComponent);

	if (!IsRunningCommandlet())
	{
		if (ArrowComponent)
		{
			ArrowComponent->ArrowColor = FColor(128, 92, 207);
			ArrowComponent->ArrowSize = 1.0f;
			ArrowComponent->bIsScreenSizeScaled = true;
		}
	}
#endif
}

void AInteractableSelector::BeginPlay()
{
	Super::BeginPlay();

	BuildAimingActor();
	BuildBeam();

	if (bSelectorStartsActivated)
	{
		ActivateSelector(true);
	}
}

void AInteractableSelector::EndPlay(EEndPlayReason::Type const EndPlayReason)
{
	ActivateSelector(false);
	DestroyBeam();
	DestroyAimingActor();
	SetSelectedInteractable(nullptr); // Notify selected interactable that we are going away.

	Super::EndPlay(EndPlayReason);
}

void AInteractableSelector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto TestShouldSelect = [this](TWeakObjectPtr<AActor> Actor)
	{
		return ShouldSelect(Cast<AInteractable>(Actor.Get()));
	};

	if (bSelectorActivated)
	{
		auto const World = GetWorld();
		if (!World)
		{
			return;
		}

		AInteractable* Candidate = nullptr;
		auto CandidateInNearField = false;
		FVector StartCast, EndCast;
		ComputeNearFieldRaycastEndpoints(StartCast, EndCast);
		TArray<FHitResult> Hits;

		auto UpdateAimingActorTransform = [&]
		{
			World->LineTraceMultiByChannel(Hits, StartCast, EndCast, ECC_Visibility);

			auto const AimingQuat = AimingActor->GetActorQuat();

			// Default target aiming when we have not hit normal.
			auto const RightVector = FVector::CrossProduct(DampenedForwardVector, FVector::UpVector);
			auto ForwardVector = FVector::CrossProduct(StartCast - EndCast, RightVector);
			auto TargetAimingQuat = FQuat(FRotationMatrix::MakeFromXZ(ForwardVector, StartCast - EndCast).Rotator());

			if (Hits.Num() > 0)
			{
				NonInteractableActorHit = Hits[0].GetActor();

				if (bAlignAimingActorWithHitNormal)
				{
					// Align with hit normal.
					ForwardVector = FVector::CrossProduct(Hits[0].ImpactNormal, RightVector);
					TargetAimingQuat = FQuat(FRotationMatrix::MakeFromXZ(ForwardVector, Hits[0].ImpactNormal).Rotator());
				}

				AimingActor->SetActorLocation(Hits[0].ImpactPoint);
			}
			else
			{
				// Place the actor at the end of the cast of nothing is hit.
				AimingActor->SetActorLocation(EndCast);
			}

			auto const LerpedTargetAimingQuat = FQuat::FastLerp(AimingQuat, TargetAimingQuat, AimingActorRotationRate);
			AimingActor->SetActorRotation(LerpedTargetAimingQuat);
		};

		// Near-field selection has priority.
		if (NearFieldRadius > 0.0f)
		{
			auto const NearFieldCollisionSphere = FCollisionShape::MakeSphere(NearFieldRadius);
			auto CandidateDistance = NearFieldRadius * 100.0f;

			World->SweepMultiByChannel(Hits, StartCast, StartCast, FQuat::Identity, InteractableTraceChannel, NearFieldCollisionSphere);

			if (Hits.Num() > 0)
			{
				for (auto const& Hit : Hits)
				{
					if (Hit.GetActor() && Hit.GetActor()->GetClass()->IsChildOf(AInteractable::StaticClass()) && TestShouldSelect(Hit.GetActor()))
					{
						auto const Distance = FVector::Distance(Hit.GetActor()->GetActorLocation(), StartCast);
						if (CandidateDistance > Distance)
						{
							CandidateDistance = Distance;
							Candidate = Cast<AInteractable>(Hit.GetActor());
							CandidateInNearField = true;
						}
					}

					// UE_LOG(LogTemp, Error, TEXT("Hit near field %s at %0.0f"), *Hit.GetActor()->GetName(), CandidateDistance);
				}
			}
		}

		// If no candidates found in the near-field, we perform far-field selection.
		if (!Candidate)
		{
			UpdateDampenedForwardVector(DampeningFactor);
			ComputeFarFieldRaycastEndpoints(StartCast, EndCast);

			if (bRaycastDebugTrace)
			{
				DrawDebugLine(World, StartCast, EndCast, FColor::Green, false, -1.0f, 0, 0.1f);
			}

			// Trace in a cone against interactable.
			auto const SphereRadius = ComputeSphereRadiusForCast();
			auto const CollisionSphere = FCollisionShape::MakeSphere(SphereRadius);
			World->SweepMultiByChannel(Hits, StartCast, EndCast, FQuat::Identity, InteractableTraceChannel, CollisionSphere);

			// Looking for the closest candidate by angle and distance.
			auto CandidateAngle = RaycastAngleDegrees;
			auto CandidateDistance = RaycastDistance;

			if (Hits.Num() > 0)
			{
				for (auto const& Hit : Hits)
				{
					// GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue, FString::Printf(TEXT("Hit %s at %f"), *Hit.GetActor()->GetName(), Hit.Distance));
					if (Hit.GetActor() && Hit.GetActor()->GetClass()->IsChildOf(AInteractable::StaticClass()) && TestShouldSelect(Hit.GetActor()))
					{
						auto const Angle = ComputeAngularDistance(Hit.GetActor());

						// Since CandidateAngle starts at RaycastAngleDegrees, we cannot select outside of the cone.
						if (Angle > CandidateAngle || Angle == CandidateAngle && Hit.Distance >= CandidateDistance)
						{
							continue;
						}

						CandidateAngle = Angle;
						CandidateDistance = Hit.Distance;
						Candidate = Cast<AInteractable>(Hit.GetActor());
					}
				}
			}

			// Do not change the selection if it is within some angle.
			if (SelectedInteractable != nullptr && Candidate != nullptr)
			{
				auto const AngleToCurrentSelection = ComputeAngularDistance(SelectedInteractable);
				if (AngleToCurrentSelection <= RaycastStickinessAngleDegrees &&
					AngleToCurrentSelection < ComputeAngularDistance(Candidate))
				{
					// Re-orient the beam.
					OrientBeam();
					if (bAlwaysShowAimingActor)
					{
						UpdateAimingActorTransform();
					}
					return;
				}
			}
		}

		if (!Candidate || bAlwaysShowAimingActor)
		{
			if (AimingActor)
			{
				// If there are no hits, we help by displaying the aiming actor.
				ActivateAimingActor(true);
				UpdateAimingActorTransform();
			}
		}
		else
		{
			// Do no show the aiming actor when we hit an interactable.
			ActivateAimingActor(false);

			// When we have a candidate, this first non-interactable actor hit is cleared.
			NonInteractableActorHit.Reset();
		}

		// Near-field selection is immediate.  , otherwise we update selection time on candidate in pre-selection.
		if (CandidateInNearField)
		{
			CandidatePreSelection = Candidate;
			CandidatePreSelectionTimeMs = Candidate->FarFieldSelectionDelayMs;
		}
		else if (CandidatePreSelection != Candidate)
		{
			CandidatePreSelection = Candidate;
			CandidatePreSelectionTimeMs = 0.0f;
		}
		else
		{
			CandidatePreSelectionTimeMs += DeltaTime * 1000.0f;
		}

		SetSelectedInteractable(CandidatePreSelection, CandidatePreSelectionTimeMs, true, CandidateInNearField);
	}
}

void AInteractableSelector::SetSelectedInteractable(AInteractable* Candidate, float SelectionDurationMs /* = 0.0f */, bool Notify /* = true */, bool CandidateInNearField /* = false */)
{
	auto const SameCandidate = SelectedInteractable == Candidate;
	auto const SameField = SelectedInteractableInNearField == CandidateInNearField;

	// Old interactable.
	if (SelectedInteractable)
	{
		if (SameCandidate)
		{
			// Same non-null target in the same field: just need to update beam orientation.
			if (SameField)
			{
				if (!CandidateInNearField)
				{
					OrientBeam();
				}

				return;
			}
		}
		else
		{
			if (Notify)
			{
				SelectedInteractable->EndSelection(this);
			}
		}
	}

	// Activating beam on the candidate.
	TargetBeam(CandidateInNearField ? nullptr : Candidate);

	// Swapping to candidate.
	if (Candidate && Candidate->FarFieldSelectionDelayMs <= SelectionDurationMs)
	{
		SelectedInteractable = Candidate;
		SelectedInteractableInNearField = CandidateInNearField;
	}
	else
	{
		SelectedInteractable = nullptr;
	}

	// New interactable (potentially nullptr).
	if (SelectedInteractable && !SameCandidate && Notify)
	{
		SelectedInteractable->BeginSelection(this);
	}
}

AInteractable* AInteractableSelector::GetSelectedInteractable(bool& SelectedInNearField) const
{
	SelectedInNearField = SelectedInteractableInNearField;
	return SelectedInteractable;
}

AActor* AInteractableSelector::GetNonInteractableHit() const
{
	// Can return null.
	return NonInteractableActorHit.Get();
}

bool AInteractableSelector::ShouldSelect_Implementation(AInteractable* Interactable) const
{
	return true;
}

void AInteractableSelector::ActivateSelector(bool Activate)
{
	if (bSelectorActivated != Activate)
	{
		if (Activate)
		{
			UpdateDampenedForwardVector(0.0f);
		}
		else
		{
			SetSelectedInteractable(nullptr);
			ActivateAimingActor(false);
		}

		bSelectorActivated = Activate;
	}
}

void AInteractableSelector::UpdateDampenedForwardVector(float Dampening)
{
	DampenedForwardVector *= Dampening;
	DampenedForwardVector += GetActorForwardVector() * (1.0f - Dampening);
	DampenedForwardVector.Normalize();
}

void AInteractableSelector::ComputeNearFieldRaycastEndpoints(FVector& Start, FVector& End) const
{
	auto const ActorPos = GetActorLocation();
	auto const ActorFwd = GetActorForwardVector();
	Start = ActorPos - ActorFwd * NearFieldRadius;
	End = ActorPos + ActorFwd * NearFieldRadius;
}

void AInteractableSelector::ComputeFarFieldRaycastEndpoints(FVector& Start, FVector& End) const
{
	auto const ActorPos = GetActorLocation();
	auto const ActorFwd = DampenedForwardVector;
	Start = ActorPos + ActorFwd * RaycastOffset;
	End = ActorPos + ActorFwd * (RaycastOffset + RaycastDistance);
}

float AInteractableSelector::ComputeSphereRadiusForCast() const
{
	return RaycastDistance * FMath::Tan(FMath::DegreesToRadians(RaycastAngleDegrees * 0.5f));
}

float AInteractableSelector::ComputeAngularDistance(AActor* Target) const
{
	auto const LineStart = GetActorLocation();
	auto const LineEnd = GetActorLocation() + 100.0f * DampenedForwardVector; // Any point will do, but projecting forward will reduce errors.
	auto const TargetLocation = Target->GetActorLocation();

	auto const NearestLocation = FMath::ClosestPointOnInfiniteLine(LineStart, LineEnd, TargetLocation);

	auto const DistNearestToUs = FVector::Distance(NearestLocation, LineStart);
	auto const DistNearestToTargetCenter = FVector::Distance(NearestLocation, TargetLocation);
	auto const DistNearestToTarget = FMath::Max(DistNearestToTargetCenter - Target->GetSimpleCollisionRadius() * 0.5f, 0.0f);

	auto const AngleToTargetRadians = FMath::FastAsin(DistNearestToTarget / DistNearestToUs);

	return FMath::RadiansToDegrees(AngleToTargetRadians);
}

void AInteractableSelector::ActivateAimingActor(bool Activate) const
{
	if (!AimingActor)
	{
		return;
	}

	if (Activate)
	{
		// This is one of AAimingActor's events.
		AimingActor->Activate();
	}
	else
	{
		// This is one of AAimingActor's events.
		AimingActor->Deactivate(SelectedInteractable);
	}
}

void AInteractableSelector::BuildAimingActor()
{
	// Minimum handling of multiplayer: don't spawn aiming actor if we are not on the server.
	if (!HasAuthority())
	{
		return;
	}

	auto World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!AimingActor && AimingActorClass)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.bAllowDuringConstructionScript = true;
		Params.Owner = this;

		// Spawn actor of desired class.
		auto Location = GetActorLocation();
		auto Rotation = GetActorRotation();
		AimingActor = Cast<AAimingActor>(World->SpawnActor(AimingActorClass, &Location, &Rotation, Params));

		if (!AimingActor)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not spawn aiming actor of class %s for interactable selector %s"),
				*AimingActorClass->GetName(),
				*GetName());
		}
		else
		{
			bAimingActorOwned = true;
			AimingActor->SetActorTickEnabled(true);
			AimingActor->SetActorEnableCollision(false);
		}
	}
}

void AInteractableSelector::DestroyAimingActor()
{
	auto World = GetWorld();
	if (!World)
	{
		return;
	}

	if (bAimingActorOwned && AimingActor->HasAuthority() && !AimingActor->IsPendingKillOrUnreachable())
	{
		World->DestroyActor(AimingActor);
		AimingActor = nullptr;
		bAimingActorOwned = false;
	}
}

void AInteractableSelector::TargetBeam(AActor* Target)
{
	if (!Beam)
	{
		return;
	}

	if (Target)
	{
		Beam->SetActorParameter(BeamSource, this);
		Beam->SetActorParameter(BeamTarget, Target);
		Beam->SetComponentTickEnabled(true);
		Beam->SetHiddenInGame(false);
		OrientBeam();
	}
	else
	{
		Beam->SetHiddenInGame(true);
		Beam->SetComponentTickEnabled(false);
	}
}

void AInteractableSelector::OrientBeam() const
{
	if (Beam)
	{
		auto const BeamTangent = DampenedForwardVector;
		// UE_LOG(LogTemp, Error, TEXT("*** BEAM UPDATE %0.2f %0.2f %0.2f"), DampenedForwardVector.X, DampenedForwardVector.Y, DampenedForwardVector.Z);
		Beam->SetBeamSourceTangent(0, BeamTangent, 0);
	}
}

void AInteractableSelector::BuildBeam()
{
	if (BeamTemplate)
	{
		Beam = UGameplayStatics::SpawnEmitterAttached(BeamTemplate, this->GetRootComponent());
		TargetBeam(nullptr);
	}
}

void AInteractableSelector::DestroyBeam()
{
	if (Beam)
	{
		Beam->DestroyComponent();
		Beam = nullptr;
	}
}
