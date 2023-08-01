// Copyright (c) Facebook, Inc. and its affiliates.

#include "OculusDeveloperTelemetry.h"

#include "OculusXRHMD/Private/OculusXRHMDModule.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Text/SRichTextBlock.h"

#define PRIVACY_POLICY_URL "https://www.oculus.com/legal/privacy-policy/"

#define LOCTEXT_NAMESPACE "FOculusUtilsModule"

#if WITH_EDITOR
#include "Dialog/SCustomDialog.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"

void UOculusDeveloperTelemetry::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	Flush();
}

auto MakeTelemetryTextBlock(float Width)
{
	auto const TelemetryMessage = LOCTEXT("EnableTelemetryMessage",
		"Enabling telemetry will transmit data to Oculus about your usage of its samples and tools. "
		"This information is used by Oculus to improve our products and better serve our developers. For more information, go to this url: "
		"<a id=\"link\" href=\"" PRIVACY_POLICY_URL "\">" PRIVACY_POLICY_URL "</>");
	
	auto const OnPrivacyPolicyClicked = FSlateHyperlinkRun::FOnClick::CreateLambda(
		[](auto)
		{
			FPlatformProcess::LaunchURL(TEXT(PRIVACY_POLICY_URL), nullptr, nullptr);
		});
	
	return SNew(SRichTextBlock).
		Text(TelemetryMessage).
		WrapTextAt(Width).
		AutoWrapText(Width == 0)
		+ SRichTextBlock::HyperlinkDecorator(TEXT("link"), OnPrivacyPolicyClicked);
}

static auto const EnableText = LOCTEXT("EnableTelemetryEnableButton", "Enable");
static auto const OptOutText = LOCTEXT("EnableTelemetryOptOut", "Opt out");
#endif

void UOculusDeveloperTelemetry::SendEvent(const char* eventName, const char* param, const char* source)
{
	OnFlush.AddLambda([eventName, param, source]
	{
		FOculusXRHMDModule::GetPluginWrapper().SendEvent2(eventName, param, source);
	});
	Flush();
}

void UOculusDeveloperTelemetry::Flush()
{
#if WITH_EDITOR
	if (bHasPrompted == false)
	{
		auto const Title = LOCTEXT("EnableTelemetryTitle", "Enable Oculus Telemetry");

		auto const Return = TSharedRef<SCustomDialog>(SNew(SCustomDialog).
			Title(Title).
			Content() [
				MakeTelemetryTextBlock(400)
			].
			Buttons({
				SCustomDialog::FButton(EnableText),
				SCustomDialog::FButton(OptOutText),
			}))->ShowModal();

		bHasPrompted = true;
		bIsEnabled = Return == 0;

		Modify();
		SaveConfig();
	}

	if (bIsEnabled && FOculusXRHMDModule::GetPluginWrapper().SendEvent2)
	{
		FOculusXRHMDModule::GetPluginWrapper().SetDeveloperMode(true);
		OnFlush.Broadcast();
		OnFlush.Clear();
	}

#endif
}

#if WITH_EDITOR
void FOculusTelemetrySettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	for (auto Object : Objects)
	{
		auto Telemetry = MakeWeakObjectPtr(Cast<UOculusDeveloperTelemetry>(Object));
		if (Telemetry != nullptr)
		{
			auto&& Category = DetailBuilder.EditCategory(TEXT("Privacy"));
			auto IsEnabledProperty = DetailBuilder.GetProperty(TEXT("bIsEnabled"));

			auto IsEnabled = [Telemetry]{ return Telemetry.IsValid() && Telemetry->bIsEnabled ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };
			auto IsOptedOut = [Telemetry]{ return Telemetry.IsValid() && Telemetry->bIsEnabled ? ECheckBoxState::Unchecked : ECheckBoxState::Checked; };
			auto SetEnabled = [Telemetry](ECheckBoxState state)
			{
				if (Telemetry.IsValid())
				{
					Telemetry->bIsEnabled = state == ECheckBoxState::Checked;
					Telemetry->bHasPrompted = true;
					Telemetry->Modify();
					Telemetry->SaveConfig();
					Telemetry->Flush();
				}
			};
			auto SetOptOut = [SetEnabled](ECheckBoxState state){ SetEnabled(state == ECheckBoxState::Unchecked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked); };

			Category.InitiallyCollapsed(false)
					.AddProperty(IsEnabledProperty)
					.ShouldAutoExpand(true)
					.CustomWidget(true)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight() [ MakeTelemetryTextBlock(0) ]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SCheckBox).
								IsChecked_Lambda(IsEnabled).
								OnCheckStateChanged_Lambda(SetEnabled).
								Content() [ SNew(STextBlock).Text(EnableText) ]
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SCheckBox).
								IsChecked_Lambda(IsOptedOut).
								OnCheckStateChanged_Lambda(SetOptOut).
								Content() [ SNew(STextBlock).Text(OptOutText) ]
						]
					];
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE

#undef PRIVACY_POLICY_URL
