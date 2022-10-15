#pragma once

#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(ModConfig,

    CONFIG_VALUE(ScoresEnabled, bool, "Enable/disable scores", true);
    CONFIG_VALUE(RestartRequired, bool, "Restart Required (DO NOT MANUALLY CHANGE THIS)", true);

    CONFIG_INIT_FUNCTION(

        CONFIG_INIT_VALUE(ScoresEnabled);
        CONFIG_INIT_VALUE(RestartRequired);
    )
)
