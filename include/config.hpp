#pragma once

#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(Config) {
    CONFIG_VALUE(SubmissionsEnabled, bool, "Submissions Enabled", true);
    CONFIG_VALUE(RankedWarning, bool, "Ranked Warning", true);
};
