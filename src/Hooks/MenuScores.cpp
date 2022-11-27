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

using namespace QuestUI::BeatSaberUI;

UnityEngine::UI::Toggle *toggle;

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
        toggle = CreateToggle(self->get_transform(), "", getModConfig().ScoresEnabled.GetValue(), UnityEngine::Vector2(-90.0f, -60.0f),
            [=](bool value) {
                getModConfig().ScoresEnabled.SetValue(value);

                if (value == false) {
                    bs_utils::Submission::disable(modInfo);
                } else {
                    bs_utils::Submission::enable(modInfo);
                }
            }
        );

        AddHoverHint(toggle->get_gameObject(), "Score Submissions");
    }
}