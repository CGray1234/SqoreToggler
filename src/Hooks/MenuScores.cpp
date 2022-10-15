#include "config.hpp"
#include "hooks.hpp"
#include "main.hpp"

#include "GlobalNamespace/StandardLevelDetailView.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "bs-utils/shared/utils.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

using namespace QuestUI::BeatSaberUI;

MAKE_AUTO_HOOK_MATCH(MenuToggle, &GlobalNamespace::StandardLevelDetailView::RefreshContent, void, GlobalNamespace::StandardLevelDetailView *self) {

    MenuToggle(self);

    getLogger().info("Building Button & Score Status");

    static ConstString canvasName("ScoreToggleCanvas");
        
    auto parent = self->actionButton->get_transform()->GetParent();
        
    auto canvasTransform = (UnityEngine::RectTransform*) parent->Find(canvasName);

    if(!canvasTransform) {
        getLogger().info("Making canvas");
        UnityEngine::GameObject* canvas = QuestUI::BeatSaberUI::CreateCanvas();
        canvasTransform = (UnityEngine::RectTransform*) canvas->get_transform();
        canvasTransform->set_name(canvasName);
        canvasTransform->SetParent(parent, false);
        canvasTransform->set_localScale({1, 1, 1});
        canvasTransform->set_sizeDelta({10, 10});
        canvasTransform->set_anchoredPosition({0, 0});
        canvasTransform->SetAsLastSibling();
        auto canvasLayout = canvas->AddComponent<UnityEngine::UI::LayoutElement*>();
        canvasLayout->set_preferredWidth(10);

        auto Toggle = CreateToggle(canvasTransform, "", getModConfig().ScoresEnabled.GetValue(), UnityEngine::Vector2(8.0f, 0.0f),
            [=](bool value) {
                getModConfig().ScoresEnabled.SetValue(value);

                if (getModConfig().ScoresEnabled.GetValue() == true) {
                    bs_utils::Submission::enable(modInfo);

                    getLogger().info("Enabled Score Submission");
                } else {
                    bs_utils::Submission::disable(modInfo);

                    getLogger().info("Disabled Score Submission");
                 }
            }
        );

        AddHoverHint(Toggle, "Score Submission");
    }
}
