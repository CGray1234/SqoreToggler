#include "UI/ViewController.hpp"
#include "UnityEngine/zzzz__GameObject_def.hpp"
#include "bsml/shared/BSML-Lite/Creation/Layout.hpp"
#include "bsml/shared/BSML-Lite/Creation/Settings.hpp"
#include "bsml/shared/BSML/Components/Settings/ToggleSetting.hpp"
#include "config.hpp"

DEFINE_TYPE(SqoreToggler, SqoreTogglerViewController);

using namespace BSML;
using namespace BSML::Lite;
using namespace SqoreToggler;

BSML::ToggleSetting* warningToggle;

void SqoreTogglerViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (firstActivation) {
        UnityEngine::GameObject *container = CreateScrollView(get_transform());

        warningToggle = CreateToggle(container->get_transform(), "Ranked map warning", getConfig().RankedWarning.GetValue(), [=](bool value) {
            getConfig().RankedWarning.SetValue(value);
        });
    }

    warningToggle->set_Value(getConfig().RankedWarning.GetValue()); // set the value of the button on every DidActivate bc im too lazy to make a flow coordinator
}
