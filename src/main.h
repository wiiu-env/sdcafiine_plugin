#pragma once
#include "version.h"

#define VERSION                                "v0.1.1"
#define VERSION_FULL                           VERSION VERSION_EXTRA

#define AUTO_APPLY_SINGLE_MODPACK_STRING       "autoApplySingleModpack"
#define SKIP_PREPARE_FOR_SINGLE_MODPACK_STRING "skipPrepareForSingleModpack"
#define SDCAFIINE_ENABLED_STRING               "sdCafiineEnabled"

extern bool gAutoApplySingleModpack;
extern bool gSkipPrepareIfSingleModpack;
