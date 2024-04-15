#include "UI/ViewController.hpp"
#include "config.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML.hpp"

DEFINE_TYPE(Sqores, SqoreViewController);

using namespace BSML;
using namespace BSML::Lite;
using namespace Sqores;

void SqoreViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {

    if (firstActivation) {
        
        UnityEngine::GameObject *container = CreateScrollView(get_transform());

        auto warningToggle = CreateToggle(container->get_transform(), "Enable/disable the confirmation on ranked maps", getModConfig().WarningEnabled.GetValue(),
            [=](bool value) {
                getModConfig().WarningEnabled.SetValue(value);
            }
        );
    }
}