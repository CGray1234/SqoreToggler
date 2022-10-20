#include "UI/ViewController.hpp"
#include "config.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "UnityEngine/UI/Toggle.hpp"
#include "UnityEngine/UI/ToggleGroup.hpp"
#include "UnityEngine/Resources.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"

DEFINE_TYPE(Sqores, SqoreViewController);

using namespace QuestUI::BeatSaberUI;
using namespace Sqores;

void SqoreViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {

    if (firstActivation) {

        UnityEngine::GameObject *container = CreateScrollView(get_transform());

        CreateText(container->get_transform(), "Yeah there's nothing much here...");
        
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
