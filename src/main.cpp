#include "main.hpp"
#include <optional>
#include <string>

#include "GlobalNamespace/StandardLevelDetailView.hpp"
#include "GlobalNamespace/LevelSelectionNavigationController.hpp"
#include "GlobalNamespace/PlatformLeaderboardViewController.hpp"
#include "GlobalNamespace/zzzz__BeatmapLevel_def.hpp"
#include "GlobalNamespace/zzzz__StandardLevelDetailView_def.hpp"
#include "HMUI/ModalView.hpp"
#include "HMUI/Screen.hpp"
#include "UI/ViewController.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"
#include "beatsaber-hook/shared/utils/typedefs-string.hpp"
#include "bsml/shared/BSML-Lite/Creation/Layout.hpp"
#include "bsml/shared/BSML-Lite/Creation/Misc.hpp"
#include "bsml/shared/BSML/Components/Settings/ToggleSetting.hpp"
#include "config.hpp"

#include "GlobalNamespace/BeatmapLevel.hpp"

#include "paper2_scotland2/shared/logger.hpp"

#include "autohooks/shared/hooks.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "logger.hpp"
#include "modInfo.hpp"
#include "scotland2/shared/modloader.h"

#include "bsml/shared/BSML.hpp"

#include "metacore/shared/game.hpp"
#include "metacore/shared/songs.hpp"
#include "metacore/shared/pp.hpp"

/// @brief Called at the early stages of game loading
/// @param info The mod info.  Update this with your mod's info.
/// @return
MOD_EXPORT_FUNC void setup(CModInfo& info) {
    // Convert the mod info to a C struct and set that as the modloader info.
    info = modInfo.to_c();

    getConfig().Init(modInfo);

    Paper::Logger::RegisterFileContextId(Logger.tag);

    Logger.info("Completed setup!");
}

using namespace BSML;
using namespace BSML::Lite;
using namespace UnityEngine;

BSML::ToggleSetting *toggle;
HMUI::ModalView* modalTransform;

bool modalReady = false;

MAKE_LATE_HOOK_MATCH(StandardLevelDetailView_RefreshContent, &GlobalNamespace::StandardLevelDetailView::RefreshContent, void, GlobalNamespace::StandardLevelDetailView *self) {
    StandardLevelDetailView_RefreshContent(self);

    MetaCore::Game::SetScoreSubmission("SqoreToggler", getConfig().SubmissionsEnabled.GetValue());
}

MAKE_LATE_HOOK_MATCH(LevelSelectionNavigationController_DidActivate, &GlobalNamespace::LevelSelectionNavigationController::DidActivate, void, GlobalNamespace::LevelSelectionNavigationController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    LevelSelectionNavigationController_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    if (firstActivation) {
        auto screen = CreateCanvas();
        screen -> AddComponent<HMUI::Screen *>();

        auto canvasTransform = screen->get_transform();
        canvasTransform->set_localPosition({1.2, 1.1, 4});

        screen->get_transform()->set_eulerAngles(Vector3(0.0f, 17.5f, 0.0f));

        modalTransform = CreateModal(screen->get_transform(), Vector2(75.0f, 60.0f), nullptr, false);
        modalTransform->get_transform()->set_eulerAngles(Vector3(0.0f, 17.5f, 0.0f));

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

        auto noticeDescription = CreateText(layout->get_transform(), "<size=100%>This map is ranked, but your score submissions are off.\nDo you want to enable score submissions?");

        auto enableButton = CreateUIButton(layout->get_transform(), "Yes", "PlayButton", UnityEngine::Vector2(0.0f, -35.0f),
            [=]() {
                getConfig().SubmissionsEnabled.SetValue(true);
                MetaCore::Game::SetScoreSubmission("SqoreToggler", true);

                toggle->set_Value(true);

                modalTransform->Hide(true, nullptr);
            }
        );

        auto disableButton = CreateUIButton(layout->get_transform(), "No", UnityEngine::Vector2(0.0f, -35.0f),
            [=]() {
                modalTransform->Hide(true, nullptr);
            }
        );

        auto noshowButton = CreateUIButton(layout->get_transform(), "No, don't show again", UnityEngine::Vector2(0.0f, -35.0f),
            [=]() {
                getConfig().RankedWarning.SetValue(false);

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

MAKE_LATE_HOOK_MATCH(LevelSelectionNavigationController_DidDeactivate, &GlobalNamespace::LevelSelectionNavigationController::DidDeactivate, void, GlobalNamespace::LevelSelectionNavigationController *self, bool removedFromHierarchy, bool screenSystemDisabling) {
    LevelSelectionNavigationController_DidDeactivate(self, removedFromHierarchy, screenSystemDisabling);

    modalTransform->Hide(true, nullptr);
}

MAKE_LATE_HOOK_MATCH(StandardLevelDetailView_SetContent, static_cast<
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
    GlobalNamespace::PlayerData* playerData
) {
    StandardLevelDetailView_SetContent(self, level, allowedBeatmapDifficultyMask, notAllowedCharacteristics, defaultDifficulty, defaultBeatmapCharacteristic, playerData);

    GlobalNamespace::BeatmapKey map = MetaCore::Songs::GetSelectedKey();
    MetaCore::PP::GetMapInfo(map, [level](std::optional<MetaCore::PP::BLSongDiff> bl, std::optional<MetaCore::PP::SSSongDiff> ss) {
        bool blRanked = false;
        if (bl) {
            blRanked = MetaCore::PP::IsRanked(*bl);
        }

        bool ssRanked = false;
        if (ss) {
            ssRanked = MetaCore::PP::IsRanked(*ss);
        }

        Logger.info("Song {} ranked status is {}", level->___songName, ssRanked || blRanked);

        if (modalReady) {
            if ((blRanked || ssRanked) && getConfig().RankedWarning.GetValue() && !getConfig().SubmissionsEnabled.GetValue()) {
                modalTransform->Show(true, true, nullptr);
            } else {
                modalTransform->Hide(true, nullptr);
            }
        } else {
            return;
        }
    });
}

MAKE_LATE_HOOK_MATCH(MakeToggle, &GlobalNamespace::PlatformLeaderboardViewController::DidActivate, void, GlobalNamespace::PlatformLeaderboardViewController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    MakeToggle(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    if (firstActivation)
    {
        auto submissionText = CreateText(self->get_transform(), "[Error]:\nCould not get submission status", UnityEngine::Vector2(-40.5f, -38.5f));
        submissionText->set_alignment(TMPro::TextAlignmentOptions::Top);
        submissionText->set_enableWordWrapping(false);
        submissionText->set_fontSize(3.25);
        submissionText->set_color(UnityEngine::Color::get_red());

        if (!getConfig().SubmissionsEnabled.GetValue()) {
            submissionText->set_text("Scores Disabled");
            submissionText->set_color(UnityEngine::Color::get_red());
        } else {
            if (MetaCore::Game::IsScoreSubmissionDisabled()) {
                submissionText->set_text("Scores Disabled\n(by external mod)");
                submissionText->set_color(UnityEngine::Color::get_yellow());
            } else {
                submissionText->set_text("Scores Enabled");
                submissionText->set_color(UnityEngine::Color::get_green());
            }
        }

        toggle = CreateToggle(self->get_transform(), "", getConfig().SubmissionsEnabled.GetValue(), UnityEngine::Vector2(-90.0f, -72.5f),
            [=](bool value) {
                getConfig().SubmissionsEnabled.SetValue(value);

                MetaCore::Game::SetScoreSubmission("SqoreToggler", value);

                if (!value) {
                    submissionText->set_text("Scores Disabled");
                    submissionText->set_color(UnityEngine::Color::get_red());
                } else {
                    if (MetaCore::Game::IsScoreSubmissionDisabled()) {
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

/// @brief Called early on in the game loading
/// @return
MOD_EXPORT_FUNC void load() {
    // Initialize il2cpp functions
    il2cpp_functions::Init();

    // Get the number of early hooks that will be installed.
    auto earlyHookCount = EARLY_HOOK_COUNT;

    // install early hooks
    if (earlyHookCount > 0) {
        Logger.info("Installing {} early hook{}", earlyHookCount, earlyHookCount == 0 || earlyHookCount > 1 ? "s" : "");
        INSTALL_EARLY_HOOKS();
        Logger.info("Finished installing early hook{}", earlyHookCount == 0 || earlyHookCount > 1 ? "s" : "");
    }
}

/// @brief Called later on in the game loading - a good time to install function hooks
/// @return
MOD_EXPORT_FUNC void late_load() {
    BSML::Init();
    BSML::Register::RegisterSettingsMenu<SqoreToggler::SqoreTogglerViewController*>("SqoreToggler");

    // Get the number of late hooks that will be installed.
    auto lateHookCount = LATE_HOOK_COUNT;

    // Install late hooks
    if (lateHookCount > 0) {
        Logger.info("Installing {} late hook{}", lateHookCount, lateHookCount > 1 ? "s" : "");
        INSTALL_LATE_HOOKS();
        Logger.info("Finished installing late hook{}", lateHookCount == 0 || lateHookCount > 1 ? "s" : "");
    }
}
