#include "UI/ViewController.hpp"
#include "config.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "UnityEngine/Resources.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"

DEFINE_TYPE(Sqores, SqoreViewController);

using namespace QuestUI::BeatSaberUI;
using namespace Sqores;

void SqoreViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {

    if (firstActivation) {

        UnityEngine::GameObject *container = CreateScrollView(get_transform());

        auto scoreToggle = CreateToggle(container->get_transform(), "Enable/disable scores (don't use this toggle, use the other one please. I'll be removing this soon)", getModConfig().ScoresEnabled.GetValue(), 
            [=](bool value) {
                getModConfig().ScoresEnabled.SetValue(value);

                getModConfig().RestartRequired.SetValue(true);
            }
        );
    }
}

using namespace UnityEngine;
using namespace GlobalNamespace;

void SqoreViewController::DidDeactivate(bool removedFromHierarchy, bool screenSystemDisabling) {

    if (getModConfig().RestartRequired.GetValue() == true) {
        Resources::FindObjectsOfTypeAll<MenuTransitionsHelper*>().FirstOrDefault()->RestartGame(nullptr);

        getModConfig().RestartRequired.SetValue(false);
    }
}
