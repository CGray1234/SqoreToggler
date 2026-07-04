#pragma once

#include "HMUI/zzzz__ViewController_def.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils-methods.hpp"
#include "custom-types/shared/macros.hpp"
#include "HMUI/ViewController.hpp"

DECLARE_CLASS_CODEGEN(SqoreToggler, SqoreTogglerViewController, HMUI::ViewController) {
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
};
