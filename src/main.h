#pragma once
#include "version.h"

#define VERSION_RAW                            "0.1"
#define VERSION                                "v" VERSION_RAW
#define VERSION_FULL_RAW                       VERSION_RAW VERSION_EXTRA

#define AUTO_APPLY_SINGLE_MODPACK_STRING       "autoApplySingleModpack"
#define SKIP_PREPARE_FOR_SINGLE_MODPACK_STRING "skipPrepareForSingleModpack"
#define SDCAFIINE_ENABLED_STRING               "sdCafiineEnabled"

extern bool gAutoApplySingleModpack;
extern bool gSkipPrepareIfSingleModpack;
