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
#include "GlobalNamespace/IBeatmapLevel.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"

#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/Object.hpp"

#include "song-details/shared/SongDetails.hpp"

#include <string>

SongDetailsCache::SongDetails* songDetails;

enum class RankedStatus {
    BeatLeader,
    ScoreSaber,
    Ranked,
    Unranked
};

RankedStatus GetRankedStatus(std::string hash)
{
    const SongDetailsCache::Song* song;
    if(!songDetails->songs.FindByHash(hash, song))
        return RankedStatus::Unranked;

    using Ranked = SongDetailsCache::RankedStates;
    auto& states = song->rankedStates;

    if(SongDetailsCache::hasFlags(states, Ranked::BeatleaderRanked) && SongDetailsCache::hasFlags(states, Ranked::ScoresaberRanked))
        return RankedStatus::Ranked;
    if(SongDetailsCache::hasFlags(states, Ranked::BeatleaderRanked))
        return RankedStatus::BeatLeader;
    if(SongDetailsCache::hasFlags(states, Ranked::ScoresaberRanked))
        return RankedStatus::ScoreSaber;

    return RankedStatus::Unranked;
}

using namespace QuestUI::BeatSaberUI;
using namespace UnityEngine;

UnityEngine::UI::Toggle *toggle;
TMPro::TextMeshProUGUI *submissionText;

HMUI::ModalView* modalTransform;

bool modalReady = false;

MAKE_AUTO_HOOK_MATCH(SetSubmissions, &GlobalNamespace::StandardLevelDetailView::RefreshContent, void, GlobalNamespace::StandardLevelDetailView *self) {

    SetSubmissions(self);

    // Set submission status

    if (getModConfig().ScoresEnabled.GetValue() == true) {
        bs_utils::Submission::enable(modInfo);

        getLogger().info("Enabled Score Submission");
    } else {
        bs_utils::Submission::disable(modInfo);

        getLogger().info("Disabled Score Submission");
    }
}



MAKE_AUTO_HOOK_MATCH(LevelSelectionNavigationController_DidActivate, &GlobalNamespace::LevelSelectionNavigationController::DidActivate, void, GlobalNamespace::LevelSelectionNavigationController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {

    LevelSelectionNavigationController_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    // Create screen

    if (firstActivation) {

        auto screen = CreateCanvas();

        screen->AddComponent<HMUI::Screen *>();
        auto canvasTransform = reinterpret_cast<UnityEngine::RectTransform *>(screen->get_transform());

        canvasTransform->set_localPosition({1.2, 1.1, 4});
        screen->get_transform()->set_eulerAngles(UnityEngine::Vector3(0.0f, 17.5f, 0.0f));

        modalTransform = CreateModal(screen->get_transform(), false);

        // modalTransform = CreateModal(screen->get_transform(), [screen](auto modal) {
        //     Object::Destroy(screen);
        // }, false);

        reinterpret_cast<UnityEngine::RectTransform *>(modalTransform->get_transform())->set_sizeDelta(UnityEngine::Vector2(75.0f, 60.0f));
        modalTransform->get_transform()->set_eulerAngles(UnityEngine::Vector3(0.0f, 17.5f, 0.0f));

        auto horizontal = CreateHorizontalLayoutGroup(modalTransform->get_transform());
        auto vertical = CreateVerticalLayoutGroup(horizontal->get_transform());
        
        auto layout = vertical;
        
        

        auto *layoutelem = layout->get_gameObject()->AddComponent<UnityEngine::UI::LayoutElement*>();
        layoutelem->set_preferredWidth(55.0f);
        layoutelem->set_preferredHeight(55.0f);

        layout->set_childAlignment(UnityEngine::TextAnchor::MiddleCenter);
        
        auto buttonLayout = CreateVerticalLayoutGroup(layoutelem->get_transform());

        TMPro::TextMeshProUGUI *redNoticeText = CreateText(layout->get_transform(), "<size=170%>WARNING");
        redNoticeText->set_color(UnityEngine::Color(1, 0, 0, 1));

        TMPro::TextMeshProUGUI *noticeDescription = CreateText(layout->get_transform(), "<size=100%>This map is ranked, but your score submissions are off.\nDo you want to enable them?");

        auto enableButton = CreateUIButton(layout->get_transform(), "Yes, enable it.", "PlayButton", UnityEngine::Vector2(0.0f, -35.0f), UnityEngine::Vector2(1, 5), 
            [=]() {
                getModConfig().ScoresEnabled.SetValue(true);

                toggle->set_isOn(true);

                modalTransform->Hide(true, nullptr);
            }
        );

        auto disableButton = CreateUIButton(layout->get_transform(), "No, don't enable.", UnityEngine::Vector2(0.0f, -35.0f), UnityEngine::Vector2(1, 5),
            [=]() {
                modalTransform->Hide(true, nullptr);
            }
        );

        auto noshowButton = CreateUIButton(layout->get_transform(), "No, don't show again.", UnityEngine::Vector2(0.0f, -35.0f), UnityEngine::Vector2(1, 5),
            [=]() {
                getModConfig().WarningEnabled.SetValue(false);

                modalTransform->Hide(true, nullptr);
            }
        );

        redNoticeText->set_enableWordWrapping(true);
        redNoticeText->set_alignment(TMPro::TextAlignmentOptions::Center);
        redNoticeText->set_fontSize(redNoticeText->get_fontSize() * 1.125f);

        redNoticeText->set_enableWordWrapping(true);

        noticeDescription->set_alignment(TMPro::TextAlignmentOptions::Center);

        modalReady = true;
    }
}

MAKE_AUTO_HOOK_MATCH(LevelSelectionNavigationController_DidDeactivate, &GlobalNamespace::LevelSelectionNavigationController::DidDeactivate, void, GlobalNamespace::LevelSelectionNavigationController *self, bool removedFromHierarchy, bool screenSystemDisabling) {
    
    // Hide modal when the viewcontroller deactivates

    LevelSelectionNavigationController_DidDeactivate(self, removedFromHierarchy, screenSystemDisabling);

    modalTransform->Hide(true, nullptr);
}

MAKE_AUTO_HOOK_MATCH(StandardLevelDetailView_SetContent, &GlobalNamespace::StandardLevelDetailView::SetContent, void, GlobalNamespace::StandardLevelDetailView *self, GlobalNamespace::IBeatmapLevel* level, 
GlobalNamespace::BeatmapDifficulty defaultDifficulty, GlobalNamespace::BeatmapCharacteristicSO* defaultBeatmapCharacteristic, GlobalNamespace::PlayerData* playerData) {

    auto currentSelectedLevel = reinterpret_cast<GlobalNamespace::IPreviewBeatmapLevel*>(level);
    
    StandardLevelDetailView_SetContent(self, level, defaultDifficulty, defaultBeatmapCharacteristic, playerData);
    
    if(!songDetails->songs.get_isDataAvailable())
        return;
    
    RankedStatus rankedStatus = RankedStatus::Unranked;

    if(currentSelectedLevel->get_levelID()->StartsWith("custom_level"))
    {
        StringW hash = currentSelectedLevel->get_levelID()->Substring(13);
        rankedStatus = GetRankedStatus(static_cast<std::string>(hash));
    }

    bool isRanked = rankedStatus != RankedStatus::Unranked;

    if (modalReady == true) {
        if (isRanked == true && getModConfig().WarningEnabled.GetValue() == true && getModConfig().ScoresEnabled.GetValue() == false) {
            modalTransform->Show(true, true, nullptr);
        } else {
            modalTransform->Hide(true, nullptr);
        }
    } else {
        return;
    }
}

MAKE_AUTO_HOOK_MATCH(MakeToggle, &GlobalNamespace::PlatformLeaderboardViewController::DidActivate, void, GlobalNamespace::PlatformLeaderboardViewController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    MakeToggle(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    if (firstActivation)
    {
        submissionText = CreateText(self->get_transform(), "[Error]:\nCould not get submission status", UnityEngine::Vector2(-41.0f, -43.5f));
        submissionText->set_alignment(TMPro::TextAlignmentOptions::Top);
        submissionText->set_fontSize(3.25);
        submissionText->set_color(UnityEngine::Color::get_red());
        if (getModConfig().ScoresEnabled.GetValue() == false) {
            submissionText->set_text("Scores Disabled");
            submissionText->set_color(UnityEngine::Color::get_red());
        } else {
            if (bs_utils::Submission::getEnabled() == false) {
                submissionText->set_text("Scores Disabled\n(by external mod)");
                submissionText->set_color(UnityEngine::Color::get_yellow());
            } else {
                submissionText->set_text("Scores Enabled");
                submissionText->set_color(UnityEngine::Color::get_green());
            }
        }

        toggle = CreateToggle(self->get_transform(), "", getModConfig().ScoresEnabled.GetValue(), UnityEngine::Vector2(-90.0f, -72.5f),
            [=](bool value) {
                getModConfig().ScoresEnabled.SetValue(value);

                if (value == false) {
                    bs_utils::Submission::disable(modInfo);
                    submissionText->set_text("Scores Disabled");
                    submissionText->set_color(UnityEngine::Color::get_red());
                } else {
                    bs_utils::Submission::enable(modInfo);
                    if (bs_utils::Submission::getEnabled() == false) {
                        submissionText->set_text("Scores Disabled\n(by external mod)");
                        submissionText->set_color(UnityEngine::Color::get_yellow());
                    } else {
                        submissionText->set_text("Scores Enabled");
                        submissionText->set_color(UnityEngine::Color::get_green());
                    }
                }
            }
        );

        AddHoverHint(toggle->get_gameObject(), "Score Submissions");
    }
}