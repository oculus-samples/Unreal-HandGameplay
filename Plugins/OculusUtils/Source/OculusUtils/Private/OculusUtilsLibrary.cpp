// Copyright (c) Facebook, Inc. and its affiliates.

#include "OculusUtilsLibrary.h"

#include "DelayAction.h"
#include "OculusUtilsModule.h"
#include "Misc/DefaultValueHelper.h"
#include "Misc/DateTime.h"

bool UOculusUtilsLibrary::GetOculusBuildInfo(FString& SourceControlChangelist, FString& BuildDateTimeString)
{
	auto BuildInfo = GetDefault<UBuildInfo>();
	if (!BuildInfo)
	{
		SourceControlChangelist = "Unknown Changelist";
		BuildDateTimeString = "Unknown Build Time";
		return false;
	}

	// Oculus Store builds have a changelist number, otherwise it's a development one.
	SourceControlChangelist = BuildInfo->PackageChangelist;
	UE_LOG(LogOculusUtils, Display, TEXT("Build Info - source control changelist: \"%s\""), *SourceControlChangelist);

	// Extracting date and time values.
	FString DateString, TimeString;
	auto DateNumber = 0, TimeNumber = 0;
	if (BuildInfo->PackageDateAndTime.Split(TEXT(" "), &DateString, &TimeString) &&
		FDefaultValueHelper::ParseInt(DateString, DateNumber) &&
		FDefaultValueHelper::ParseInt(TimeString, TimeNumber))
	{
		UE_LOG(LogOculusUtils, Display, TEXT("Build Info - parsed package date %d and time %d"), DateNumber, TimeNumber);

		// Separating date/time components.
		auto const Day = DateNumber % 100;
		DateNumber /= 100;
		auto const Month = DateNumber % 100;
		auto const Year = DateNumber / 100;

		auto const Second = TimeNumber % 100;
		TimeNumber /= 100;
		auto const Minute = TimeNumber % 100;
		auto const Hour = TimeNumber / 100;

		// We use a generic PT for both PDT and PST, where Oculus build servers are located.
		FDateTime BuildDate(Year, Month, Day, Hour, Minute, Second);
		BuildDateTimeString = BuildDate.ToString(TEXT("%Y-%m-%d %H:%M:%S PT"));
	}
	else
	{
		// If unparsable, we keep it as is.
		UE_LOG(LogOculusUtils, Display, TEXT("Build Info - cannot parse package date and time: \"%s\""), *(BuildInfo->PackageDateAndTime));
		BuildDateTimeString = BuildInfo->PackageDateAndTime;
	}
	return !(SourceControlChangelist.IsEmpty() && BuildDateTimeString.IsEmpty());
}

TArray<UActorComponent*> UOculusUtilsLibrary::SortComponentsByName(TArray<UActorComponent*> const& Components)
{
	auto SortedComponents = Components;

	SortedComponents.Sort(
		[](UActorComponent const& A, UActorComponent const& B) -> bool
		{
			return A.GetName().Compare(B.GetName()) < 0;
		});
	return SortedComponents;
}

class FTickUntilAction : public FPendingLatentAction
{
public:
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;
	bool bIsComplete = false;

	FTickUntilAction(FLatentActionInfo const& LatentInfo) :
		ExecutionFunction(LatentInfo.ExecutionFunction),
		OutputLink(LatentInfo.Linkage),
		CallbackTarget(LatentInfo.CallbackTarget)
	{
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		if (bIsComplete)
		{
			Response.DoneIf(true);
		}
		else
		{
			Response.TriggerLink(ExecutionFunction, OutputLink, CallbackTarget);
		}
	}
};

void UOculusUtilsLibrary::TickUntil(UObject const* WorldContextObject, ETickUntilInputPin InputPin, FLatentActionInfo LatentInfo)
{
	if (auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		auto& LatentActionManager = World->GetLatentActionManager();
		auto Action = LatentActionManager.FindExistingAction<FTickUntilAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);
		if (InputPin == ETickUntilInputPin::Start)
		{
			if (Action == nullptr)
			{
				LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FTickUntilAction(LatentInfo));
			}
		}
		else if (InputPin == ETickUntilInputPin::Break)
		{
			if (Action != nullptr)
			{
				Action->bIsComplete = true;
			}
		}
	}
}
