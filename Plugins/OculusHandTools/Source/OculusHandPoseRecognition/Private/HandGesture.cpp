// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "HandGesture.h"
#include "OculusHandPoseRecognitionModule.h"
#include "HandPoseRecognizer.h"
#include "Misc/Char.h"

bool FHandGesture::ProcessEncodedGestureString(UHandPoseRecognizer* HandPoseRecognizer)
{
	TCHAR const* Buffer = CustomEncodedGesture.GetCharArray().GetData();

	while (Buffer && *Buffer)
	{
		int PoseIndex;
		float PoseMinDuration;

		if (!ReadTimedPose(HandPoseRecognizer, &Buffer, &PoseIndex, &PoseMinDuration))
		{
			UE_LOG(LogHandPoseRecognition, Error, TEXT("Hand gesture error near position %d of %s"),
				CustomEncodedGesture.GetCharArray().GetData() - Buffer,
				*CustomEncodedGesture);
			return false;
		}

		TimedPoses.Add({PoseIndex, PoseMinDuration, 0.0f, 0.0f});
	}

	// Reset state
	Reset(true);

	return true;
}

bool FHandGesture::Step(int PoseIndex, float PoseDuration, float DeltaTime, float CurrentTime, FVector const& Location)
{
	// Nothing to do if there are no poses
	if (TimedPoses.Num() == 0)
	{
		return false;
	}

	if (GestureState == EGestureState::GestureNotStarted)
	{
		// We only go into "gesture in progress" state when we have held the initial pose a minimum amount of time.
		if (TimedPoses.Num() > 0 && TimedPoses[0].PoseIndex == PoseIndex && TimedPoses[0].PoseMinDuration < PoseDuration)
		{
			GestureState = EGestureState::GestureInProgress;
			CurrentStep = 0;
			GestureStartLocation = GestureEndLocation = Location;
			DurationInCurrentStep = PoseDuration;
			TimedPoses[0].StepFirstTime = TimedPoses[0].StepLastTime = CurrentTime;

			// Debugging
			if (bGestureDebugLog)
			{
				UE_LOG(LogHandPoseRecognition, Display,
					TEXT("Gesture %s step: index=%d duration=%0.2fs delta=%0.2fs time=%0.2fs"),
					*GestureName,
					PoseIndex, PoseDuration, DeltaTime, CurrentTime);
				UE_LOG(LogHandPoseRecognition, Display, TEXT("   Started with %0.2fs on first pose"), PoseDuration);
			}
		}
	}
	else // if (GestureState == GestureInProgress || (IsLooping && GestureState == GestureCompleted))
	{
		// Debugging
		if (bGestureDebugLog)
		{
			UE_LOG(LogHandPoseRecognition, Display,
				TEXT("Gesture %s step: index=%d duration=%0.2fs delta=%0.2fs time=%0.2fs"),
				*GestureName,
				PoseIndex, PoseDuration, DeltaTime, CurrentTime);
		}

		if (TimedPoses[CurrentStep].PoseIndex == PoseIndex)
		{
			// We need a minimum of time in the current pose before we can move on to the next one.
			DurationInCurrentStep = PoseDuration;
			DurationInTransition = 0.0f;

			TimedPoses[CurrentStep].StepLastTime = CurrentTime;

			// For gesture start location, we are looking for a 'late' location in the first step,
			// and for the end location, we are looking for an 'early' location in the last step.
			if (CurrentStep == 0)
			{
				GestureStartLocation = GestureStartLocation * DirectionBufferingFactor + Location * (1.0f - DirectionBufferingFactor);
			}
			else if (CurrentStep == (TimedPoses.Num() - 1))
			{
				GestureEndLocation = GestureEndLocation * (1.0f - DirectionBufferingFactor) + Location * DirectionBufferingFactor;
			}

			// Debugging
			if (bGestureDebugLog)
			{
				UE_LOG(LogHandPoseRecognition, Display, TEXT("   Pose held for %0.2fs"), DurationInCurrentStep);
			}
		}
		else
		{
			// We also cannot be in any other pose longer than the MaxTransitionTime.
			auto const NextStep = (CurrentStep + 1) % (TimedPoses.Num() + (bIsLooping ? 0 : 1));

			if (NextStep < TimedPoses.Num() &&
				DurationInCurrentStep >= TimedPoses[CurrentStep].PoseMinDuration &&
				TimedPoses[NextStep].PoseIndex == PoseIndex)
			{
				// We meet all the conditions to move forward
				CurrentStep = NextStep;
				DurationInCurrentStep = PoseDuration;
				DurationInTransition = 0.0f;

				TimedPoses[CurrentStep].StepFirstTime = TimedPoses[CurrentStep].StepLastTime = CurrentTime;

				if (CurrentStep == (TimedPoses.Num() - 1))
				{
					GestureEndLocation = Location;
				}

				// Debugging
				if (bGestureDebugLog)
				{
					UE_LOG(LogHandPoseRecognition, Display,
						TEXT("   Moved to next pose since current pose duration %0.2fs > minimum %0.2fs"),
						DurationInCurrentStep, TimedPoses[CurrentStep].PoseMinDuration);
				}
			}
			else
			{
				// This is not the current pose, and we are not ready to move to the next one,
				// so this counts as a transition.
				DurationInTransition += DeltaTime;

				// Non-looping gestures do not allow transitions on the last pose.
				// In all other situations, we test against the max transition time.
				if (DurationInTransition > MaxTransitionTime || (GestureState == EGestureState::GestureCompleted && !bIsLooping))
				{
					if (bGestureDebugLog)
					{
						UE_LOG(LogHandPoseRecognition, Display,
							TEXT("   In transition for %0.2fs > max transition time %0.2fs => reset"),
							DurationInTransition, MaxTransitionTime);
					}

					Reset();
					return false;
				}
				if (bGestureDebugLog)
				{
					UE_LOG(LogHandPoseRecognition, Display,
						TEXT("   In transition for %0.2fs"),
						DurationInTransition);
				}
			}
		}
	}

	// Checking for completion.
	if (GestureState != EGestureState::GestureNotStarted &&
		CurrentStep == (TimedPoses.Num() - 1) &&
		DurationInCurrentStep >= TimedPoses[CurrentStep].PoseMinDuration)
	{
		if (bGestureDebugLog && GestureState != EGestureState::GestureCompleted)
		{
			UE_LOG(LogHandPoseRecognition, Display, TEXT("   Gesture completed!"));
		}

		GestureState = EGestureState::GestureCompleted;
	}

	return GestureState == EGestureState::GestureCompleted;
}

float FHandGesture::ComputeTransitionTime(
	int FirstPoseIndex /* = 0 */, EPoseTimeType FirstPoseType /* = PoseLastTime */,
	int SecondPoseIndex /* = -1 */, EPoseTimeType SecondPoseType /* = PoseFirstTime */) const
{
	auto const NumPoses = TimedPoses.Num();

	if (SecondPoseIndex == -1)
	{
		SecondPoseIndex = TimedPoses.Num() - 1;
	}

	if (FirstPoseIndex < 0 || FirstPoseIndex >= NumPoses ||
		SecondPoseIndex < 0 || SecondPoseIndex >= NumPoses ||
		FirstPoseIndex >= SecondPoseIndex)
	{
		return 0.0f;
	}

	auto const FirstTime = FirstPoseType == PoseFirstTime ? TimedPoses[FirstPoseIndex].StepFirstTime : TimedPoses[FirstPoseIndex].StepLastTime;
	auto const SecondTime = SecondPoseType == PoseFirstTime ? TimedPoses[SecondPoseIndex].StepFirstTime : TimedPoses[SecondPoseIndex].StepLastTime;

	auto const Duration = SecondTime - FirstTime;
	return Duration < 0.0f ? 0.0f : Duration;
}

void FHandGesture::Reset(bool Force /* = false */)
{
	if (Force || GestureState != EGestureState::GestureNotStarted)
	{
		GestureState = EGestureState::GestureNotStarted;
		CurrentStep = -1;
		DurationInCurrentStep = 0.0f;
		DurationInTransition = 0.0f;

		GestureStartLocation.Set(0, 0, 0);
		GestureEndLocation.Set(0, 0, 0);
		DirectionBufferingFactor = 0.75f;

		for (auto& TimedPose : TimedPoses)
		{
			TimedPose.StepFirstTime = 0.0f;
			TimedPose.StepLastTime = 0.0f;
		}
	}
}

void FHandGesture::DumpGestureState(int GestureIndex, UHandPoseRecognizer const* HandPoseRecognizer) const
{
	FString StateString;
	switch (GestureState)
	{
	case EGestureState::GestureNotStarted:
		StateString = TEXT("NotStarted");
		break;
	case EGestureState::GestureInProgress:
		StateString = TEXT("InProgress");
		break;
	case EGestureState::GestureCompleted:
		StateString = TEXT("Completed");
		break;
	}

	UE_LOG(LogHandPoseRecognition, Display, TEXT("Gesture %s[%d] %s"), *GestureName, GestureIndex, *StateString);
	UE_LOG(LogHandPoseRecognition, Display, TEXT("Duration %05.3f Transition %05.3f"), DurationInCurrentStep, DurationInTransition);

	auto Step = 0;
	for (auto const& TimedPose : TimedPoses)
	{
		UE_LOG(LogHandPoseRecognition, Display, TEXT(" %c %d %8s[%d] %05.3f - %05.3f"),
			CurrentStep==Step ? '>' : ' ', Step,
			*(HandPoseRecognizer->Poses[TimedPose.PoseIndex].PoseName),
			TimedPose.PoseIndex,
			TimedPose.StepFirstTime,
			TimedPose.StepLastTime);

		Step++;
	}
}

int FHandGesture::FindPoseIndex(UHandPoseRecognizer* Recognizer, FString const& PoseName)
{
	for (auto PoseIndex = 0; PoseIndex < Recognizer->Poses.Num(); ++PoseIndex)
	{
		if (Recognizer->Poses[PoseIndex].PoseName == PoseName)
		{
			return PoseIndex;
		}
	}

	return -1;
}

bool FHandGesture::ReadTimedPose(UHandPoseRecognizer* Recognizer, TCHAR const** Buffer, int* PoseIndex, float* PoseMinDuration)
{
	// Pose name
	FString PoseNameRead;
	while (FChar::IsWhitespace(**Buffer))
	{
		++(*Buffer);
	}
	while (**Buffer && **Buffer != '/' && **Buffer != ',' && !FChar::IsWhitespace(**Buffer))
	{
		PoseNameRead += **Buffer;
		++(*Buffer);
	}

	// Pose index
	auto const PoseIndexFound = FindPoseIndex(Recognizer, PoseNameRead);
	if (PoseIndexFound == -1)
	{
		UE_LOG(LogHandPoseRecognition, Error, TEXT("Unrecognized pose called %s in %s"), *PoseNameRead, *Recognizer->GetName());
		return false;
	}

	// Pose min duration
	auto PoseMinDurationReadMillis = 0;
	while (FChar::IsWhitespace(**Buffer))
	{
		++(*Buffer);
	}
	if (**Buffer == '/')
	{
		++(*Buffer);

		// We have a min duration in milliseconds
		while (FChar::IsWhitespace(**Buffer))
		{
			++(*Buffer);
		}
		while (FChar::IsDigit(**Buffer))
		{
			PoseMinDurationReadMillis *= 10;
			PoseMinDurationReadMillis += (**Buffer) - '0';
			++(*Buffer);
		}
	}

	// Consume end of timed pose
	while (FChar::IsWhitespace(**Buffer))
	{
		++(*Buffer);
	}
	if (**Buffer == ',')
	{
		++(*Buffer);
	}
	else if (**Buffer)
	{
		UE_LOG(LogHandPoseRecognition, Error, TEXT("End of timed pose expected near '%s'"), *Buffer);
		return false;
	}

	// Return results
	*PoseIndex = PoseIndexFound;
	*PoseMinDuration = PoseMinDurationReadMillis * 0.001f;

	return true;
}
