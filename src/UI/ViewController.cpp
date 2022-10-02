#include "UI/ViewController.hpp"
#include "config.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

DEFINE_TYPE(Sqores, SqoreViewController);

using namespace QuestUI::BeatSaberUI;
using namespace Sqores;

void SqoreViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {

    if (firstActivation) {

        UnityEngine::GameObject *container = CreateScrollView(get_transform());

        auto scoreToggle = CreateToggle(container->get_transform(), "Enable/disable scores", getModConfig().ScoresEnabled.GetValue(), 
            [=](bool value) {
                getModConfig().ScoresEnabled.SetValue(value);
            }
        );
    }
}