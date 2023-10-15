#pragma once

#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(ModConfig,
    CONFIG_VALUE(ScoresEnabled, bool, "Enable/disable scores", true);
    CONFIG_VALUE(WarningEnabled, bool, "Enable/disable warning on ranked maps", true);
)