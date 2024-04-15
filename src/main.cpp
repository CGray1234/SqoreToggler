#include "main.hpp"
#include "UI/ViewController.hpp"

auto logger = Paper::ConstLoggerContext(MOD_ID);

modloader::ModInfo modInfo{MOD_ID, VERSION, 0};

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    config.Load();
    return config;
}

#include "config.hpp"
#include "main.hpp"

#include "GlobalNamespace/StandardLevelDetailView.hpp"
#include "GlobalNamespace/PlatformLeaderboardViewController.hpp"
#include "GlobalNamespace/LevelSelectionNavigationController.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "bs-utils/shared/utils.hpp"
#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapLevelSO.hpp"
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

using namespace BSML;
using namespace BSML::Lite;
using namespace UnityEngine;

BSML::ToggleSetting *toggle;

HMUI::ModalView* modalTransform;

bool modalReady = false;

MAKE_HOOK_MATCH(SetSubmissions, &GlobalNamespace::StandardLevelDetailView::RefreshContent, void, GlobalNamespace::StandardLevelDetailView *self) {

    SetSubmissions(self);

    // Set submission status

    if (getModConfig().ScoresEnabled.GetValue() == true) {
        bs_utils::Submission::enable(modInfo);

        logger.info("Enabled Score Submission");
    } else {
        bs_utils::Submission::disable(modInfo);

        logger.info("Disabled Score Submission");
    }
}

MAKE_HOOK_MATCH(LevelSelectionNavigationController_DidActivate, &GlobalNamespace::LevelSelectionNavigationController::DidActivate, void, GlobalNamespace::LevelSelectionNavigationController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {

    LevelSelectionNavigationController_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    // Create screen

    if (firstActivation) {

        auto screen = CreateCanvas();

        screen->AddComponent<HMUI::Screen *>();
        auto canvasTransform = screen->get_transform();

        canvasTransform->set_localPosition({1.2, 1.1, 4});
        screen->get_transform()->set_eulerAngles(UnityEngine::Vector3(0.0f, 17.5f, 0.0f));

        modalTransform = CreateModal(screen->get_transform(), UnityEngine::Vector2(75.0f, 60.0f), nullptr, false);

        // modalTransform = CreateModal(screen->get_transform(), [screen](auto modal) {
        //     Object::Destroy(screen);
        // }, false);

        modalTransform->get_transform()->set_eulerAngles(UnityEngine::Vector3(0.0f, 17.5f, 0.0f));

        auto horizontal = CreateHorizontalLayoutGroup(modalTransform->get_transform());
        auto vertical = CreateVerticalLayoutGroup(horizontal->get_transform());
        
        auto layout = vertical;

        auto *layoutelem = layout->get_gameObject()->AddComponent<UnityEngine::UI::LayoutElement*>();
        layoutelem->set_preferredWidth(55.0f);
        layoutelem->set_preferredHeight(55.0f);

        layout->set_childAlignment(UnityEngine::TextAnchor::MiddleCenter);
        
        auto buttonLayout = CreateVerticalLayoutGroup(layoutelem->get_transform());

        auto redNoticeText = CreateText(layout->get_transform(), "<size=170%>WARNING");
        redNoticeText->set_color(UnityEngine::Color(1, 0, 0, 1));

        auto noticeDescription = CreateText(layout->get_transform(), "<size=100%>This map is ranked, but your score submissions are off.\nDo you want to enable them?");

        auto enableButton = CreateUIButton(layout->get_transform(), "Yes, enable it.", "PlayButton", UnityEngine::Vector2(0.0f, -35.0f), UnityEngine::Vector2(1, 5), 
            [=]() {
                getModConfig().ScoresEnabled.SetValue(true);

                toggle->set_Value(true);

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

MAKE_HOOK_MATCH(LevelSelectionNavigationController_DidDeactivate, &GlobalNamespace::LevelSelectionNavigationController::DidDeactivate, void, GlobalNamespace::LevelSelectionNavigationController *self, bool removedFromHierarchy, bool screenSystemDisabling) {
    
    // Hide modal when the viewcontroller deactivates

    LevelSelectionNavigationController_DidDeactivate(self, removedFromHierarchy, screenSystemDisabling);

    modalTransform->Hide(true, nullptr);
}

MAKE_HOOK_MATCH(StandardLevelDetailView_SetContent, static_cast<
    void(GlobalNamespace::StandardLevelDetailView::*)(
        ::GlobalNamespace::BeatmapLevel* level,
        ::GlobalNamespace::BeatmapDifficultyMask allowedBeatmapDifficultyMask,
        ::System::Collections::Generic::HashSet_1<::UnityW<::GlobalNamespace::BeatmapCharacteristicSO>>* notAllowedCharacteristics,
        ::GlobalNamespace::BeatmapDifficulty defaultDifficulty,
        ::GlobalNamespace::BeatmapCharacteristicSO* defaultBeatmapCharacteristic,
        ::GlobalNamespace::PlayerData* playerData
    )>(&GlobalNamespace::StandardLevelDetailView::SetContent), 
    void, GlobalNamespace::StandardLevelDetailView* self,
    GlobalNamespace::BeatmapLevel* level,
    GlobalNamespace::BeatmapDifficultyMask allowedBeatmapDifficultyMask,
    System::Collections::Generic::HashSet_1<::UnityW<::GlobalNamespace::BeatmapCharacteristicSO>>* notAllowedCharacteristics,
    GlobalNamespace::BeatmapDifficulty defaultDifficulty,
    GlobalNamespace::BeatmapCharacteristicSO* defaultBeatmapCharacteristic,
    GlobalNamespace::PlayerData* playerData) {
    
    StandardLevelDetailView_SetContent(self, level, allowedBeatmapDifficultyMask, notAllowedCharacteristics, defaultDifficulty, defaultBeatmapCharacteristic, playerData);
    
    if(!songDetails->songs.get_isDataAvailable())
        return;
    
    RankedStatus rankedStatus = RankedStatus::Unranked;

    if(level->__cordl_internal_get_levelID()->StartsWith("custom_level"))
    {
        StringW hash = level->__cordl_internal_get_levelID()->Substring(13);
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

MAKE_HOOK_MATCH(MakeToggle, &GlobalNamespace::PlatformLeaderboardViewController::DidActivate, void, GlobalNamespace::PlatformLeaderboardViewController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    MakeToggle(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    if (firstActivation)
    {
        auto submissionText = CreateText(self->get_transform(), "[Error]:\nCould not get submission status", UnityEngine::Vector2(-40.5f, -38.5f));
        submissionText->set_alignment(TMPro::TextAlignmentOptions::Top);
        submissionText->set_enableWordWrapping(false);
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

// Called at the early stages of game loading
extern "C" void setup(CModInfo* info) {
    info->id = MOD_ID;
    info->version = VERSION;
    modInfo.assign(*info);
	
    getConfig().Load();
    logger.info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    getModConfig().Init(modInfo);
    BSML::Init();
    BSML::Register::RegisterSettingsMenu<Sqores::SqoreViewController*>("Sqore Toggler");

    logger.info("Installing hooks...");
    INSTALL_HOOK(logger, SetSubmissions);
    INSTALL_HOOK(logger, LevelSelectionNavigationController_DidActivate);
    INSTALL_HOOK(logger, LevelSelectionNavigationController_DidDeactivate);
    INSTALL_HOOK(logger, StandardLevelDetailView_SetContent);
    INSTALL_HOOK(logger, MakeToggle);
    logger.info("Installed all hooks!");
}