#include "main.h"
#include "modpackSelector.h"
#include "utils/logger.h"
#include <content_redirection/redirection.h>
#include <coreinit/title.h>
#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>

WUPS_PLUGIN_NAME("SDCafiine");
WUPS_PLUGIN_DESCRIPTION("SDCafiine");
WUPS_PLUGIN_VERSION(VERSION_FULL);
WUPS_PLUGIN_AUTHOR("Maschell");
WUPS_PLUGIN_LICENSE("GPL");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE("sdcafiine"); // Unqiue id for the storage api

CRLayerHandle contentLayerHandle __attribute__((section(".data"))) = 0;
CRLayerHandle aocLayerHandle __attribute__((section(".data")))     = 0;

bool gAutoApplySingleModpack     = false;
bool gSkipPrepareIfSingleModpack = false;
bool gSDCafiineEnabled           = true;

INITIALIZE_PLUGIN() {
    // But then use libcontentredirection instead.
    ContentRedirectionStatus error;
    if ((error = ContentRedirection_InitLibrary()) != CONTENT_REDIRECTION_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init ContentRedirection. Error %s %d", ContentRedirection_GetStatusStr(error), error);
        OSFatal("Failed to init ContentRedirection.");
    }

    // Open storage to read values
    WUPSStorageError storageRes = WUPS_OpenStorage();
    if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to open storage %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);
    } else {
        // Try to get value from storage
        if ((storageRes = WUPS_GetBool(nullptr, AUTO_APPLY_SINGLE_MODPACK_STRING, &gAutoApplySingleModpack)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            // Add the value to the storage if it's missing.
            if (WUPS_StoreBool(nullptr, AUTO_APPLY_SINGLE_MODPACK_STRING, gAutoApplySingleModpack) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to store bool");
            }
        } else if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to get bool %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);
        }

        if ((storageRes = WUPS_GetBool(nullptr, SDCAFIINE_ENABLED_STRING, &gSDCafiineEnabled)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            // Add the value to the storage if it's missing.
            if (WUPS_StoreBool(nullptr, SDCAFIINE_ENABLED_STRING, gSDCafiineEnabled) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to store bool");
            }
        } else if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to get bool %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);
        }

        if ((storageRes = WUPS_GetBool(nullptr, SKIP_PREPARE_FOR_SINGLE_MODPACK_STRING, &gSkipPrepareIfSingleModpack)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            // Add the value to the storage if it's missing.
            if (WUPS_StoreBool(nullptr, SKIP_PREPARE_FOR_SINGLE_MODPACK_STRING, gSkipPrepareIfSingleModpack) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to store bool");
            }
        } else if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to get bool %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);
        }

        // Close storage
        if (WUPS_CloseStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to close storage");
        }
    }
    contentLayerHandle = 0;
    aocLayerHandle     = 0;
}

/* Entry point */
ON_APPLICATION_START() {
    initLogging();
    if (gSDCafiineEnabled) {
        HandleMultiModPacks(OSGetTitleID());
    } else {
        DEBUG_FUNCTION_LINE("SDCafiine is disabled");
    }
}

void autoApplySingleModpackChanged(ConfigItemBoolean *item, bool newValue) {
    DEBUG_FUNCTION_LINE("New value in gAutoApplySingleModpack: %d", newValue);
    gAutoApplySingleModpack = newValue;
    // If the value has changed, we store it in the storage.
    WUPS_StoreInt(nullptr, AUTO_APPLY_SINGLE_MODPACK_STRING, gAutoApplySingleModpack);
}

void skipPrepareIfSingleModpackChanged(ConfigItemBoolean *item, bool newValue) {
    DEBUG_FUNCTION_LINE("New value in gSkipPrepareIfSingleModpack: %d", newValue);
    gSkipPrepareIfSingleModpack = newValue;
    // If the value has changed, we store it in the storage.
    WUPS_StoreInt(nullptr, SKIP_PREPARE_FOR_SINGLE_MODPACK_STRING, gSkipPrepareIfSingleModpack);
}

void sdCafiineEnabledChanged(ConfigItemBoolean *item, bool newValue) {
    DEBUG_FUNCTION_LINE("New value in gSDCafiineEnabled: %d", newValue);
    gSDCafiineEnabled = newValue;
    // If the value has changed, we store it in the storage.
    WUPS_StoreInt(nullptr, SDCAFIINE_ENABLED_STRING, gSDCafiineEnabled);
}

WUPS_GET_CONFIG() {
    // We open the storage, so we can persist the configuration the user did.
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to open storage");
        return 0;
    }

    WUPSConfigHandle config;
    WUPSConfig_CreateHandled(&config, "SDCafiine");

    WUPSConfigCategoryHandle setting;
    WUPSConfig_AddCategoryByNameHandled(config, "Settings", &setting);
    WUPSConfigCategoryHandle advanced;
    WUPSConfig_AddCategoryByNameHandled(config, "Advanced settings", &advanced);

    WUPSConfigItemBoolean_AddToCategoryHandled(config, setting, SDCAFIINE_ENABLED_STRING, "Enable SDCafiine (game needs to be restarted)", gSDCafiineEnabled, &sdCafiineEnabledChanged);
    WUPSConfigItemBoolean_AddToCategoryHandled(config, advanced, AUTO_APPLY_SINGLE_MODPACK_STRING, "Auto apply the modpack if only one modpack exists", gAutoApplySingleModpack, &autoApplySingleModpackChanged);
    WUPSConfigItemBoolean_AddToCategoryHandled(config, advanced, SKIP_PREPARE_FOR_SINGLE_MODPACK_STRING, "Skip \"Preparing modpack...\" screen", gSkipPrepareIfSingleModpack, &skipPrepareIfSingleModpackChanged);

    return config;
}

WUPS_CONFIG_CLOSED() {
    // Save all changes
    if (WUPS_CloseStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to close storage");
    }
}

ON_APPLICATION_ENDS() {
    if (contentLayerHandle != 0) {
        ContentRedirection_RemoveFSLayer(contentLayerHandle);
        contentLayerHandle = 0;
    }
    if (aocLayerHandle != 0) {
        ContentRedirection_RemoveFSLayer(aocLayerHandle);
        aocLayerHandle = 0;
    }
    deinitLogging();
}