#include "config.hpp"
#include "hooks.hpp"
#include "main.hpp"

#include "GlobalNamespace/StandardLevelDetailView.hpp"
#include "GlobalNamespace/PlatformLeaderboardViewController.hpp"
#include "GlobalNamespace/LevelSelectionNavigationController.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "bs-utils/shared/utils.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

#include "GlobalNamespace/StandardLevelDetailView.hpp"

#include <string>

using namespace QuestUI::BeatSaberUI;

UnityEngine::UI::Toggle *toggle;
TMPro::TextMeshProUGUI *submissionText;

MAKE_AUTO_HOOK_MATCH(SetSubmissions, &GlobalNamespace::StandardLevelDetailView::RefreshContent, void, GlobalNamespace::StandardLevelDetailView *self) {

    SetSubmissions(self);

    if (getModConfig().ScoresEnabled.GetValue() == true) {
        bs_utils::Submission::enable(modInfo);

        getLogger().info("Enabled Score Submission");
    } else {
        bs_utils::Submission::disable(modInfo);

        getLogger().info("Disabled Score Submission");
    }
}

MAKE_AUTO_HOOK_MATCH(MakeToggle, &GlobalNamespace::PlatformLeaderboardViewController::DidActivate, void, GlobalNamespace::PlatformLeaderboardViewController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    MakeToggle(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    if (firstActivation)
    {
        submissionText = CreateText(self->get_transform(), "[Error]:\nCould not get submission status", UnityEngine::Vector2(-42.0f, -31.5f));
        submissionText->set_alignment(TMPro::TextAlignmentOptions::Center);
        submissionText->set_fontSize(3.25);
        submissionText->set_color(UnityEngine::Color::get_red());
        if (getModConfig().ScoresEnabled.GetValue() == false) {
            submissionText->set_text("Score Submissions:\nDisabled (by SqoreToggler)");
            submissionText->set_color(UnityEngine::Color::get_red());
        } else {
            if (bs_utils::Submission::getEnabled() == false) {
                submissionText->set_text("Score Submissions:\nDisabled (by external mod)");
                submissionText->set_color(UnityEngine::Color::get_yellow());
            } else {
                submissionText->set_text("Score Submissions:\nEnabled");
                submissionText->set_color(UnityEngine::Color::get_green());
            }
        }

        toggle = CreateToggle(self->get_transform(), "", getModConfig().ScoresEnabled.GetValue(), UnityEngine::Vector2(-90.0f, -60.0f),
            [=](bool value) {
                getModConfig().ScoresEnabled.SetValue(value);

                if (value == false) {
                    bs_utils::Submission::disable(modInfo);
                    submissionText->set_text("Score Submissions:\nDisabled (by SqoreToggler)");
                    submissionText->set_color(UnityEngine::Color::get_red());
                } else {
                    bs_utils::Submission::enable(modInfo);
                    if (bs_utils::Submission::getEnabled() == false) {
                        submissionText->set_text("Score Submissions:\nDisabled (by external mod)");
                        submissionText->set_color(UnityEngine::Color::get_yellow());
                    } else {
                        submissionText->set_text("Score Submissions:\nEnabled");
                        submissionText->set_color(UnityEngine::Color::get_green());
                    }
                }
            }
        );

        AddHoverHint(toggle->get_gameObject(), "Score Submissions");
    }
}