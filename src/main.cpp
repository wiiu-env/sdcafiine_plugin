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

CRLayerHandle sContentLayerHandle = 0;
CRLayerHandle sAocLayerHandle     = 0;

#define DEFAULT_AUTO_APPLY_SINGLE_MODPACK      false
#define DEFAULT_SKIP_PREPARE_IF_SINGLE_MODPACK false
#define DEFAULT_SDCAFIINE_ENABLED              true

bool gAutoApplySingleModpack     = DEFAULT_AUTO_APPLY_SINGLE_MODPACK;
bool gSkipPrepareIfSingleModpack = DEFAULT_SKIP_PREPARE_IF_SINGLE_MODPACK;
bool gSDCafiineEnabled           = DEFAULT_SDCAFIINE_ENABLED;

WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle);

void ConfigMenuClosedCallback();

INITIALIZE_PLUGIN() {
    // But then use libcontentredirection instead.
    ContentRedirectionStatus error;
    if ((error = ContentRedirection_InitLibrary()) != CONTENT_REDIRECTION_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init ContentRedirection. Error %s %d", ContentRedirection_GetStatusStr(error), error);
        OSFatal("Failed to init ContentRedirection.");
    }

    if (WUPSStorageAPI::GetOrStoreDefault(AUTO_APPLY_SINGLE_MODPACK_STRING, gAutoApplySingleModpack, DEFAULT_AUTO_APPLY_SINGLE_MODPACK) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\"", AUTO_APPLY_SINGLE_MODPACK_STRING);
    }
    if (WUPSStorageAPI::GetOrStoreDefault(SDCAFIINE_ENABLED_STRING, gSDCafiineEnabled, DEFAULT_SDCAFIINE_ENABLED) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\"", SDCAFIINE_ENABLED_STRING);
    }
    if (WUPSStorageAPI::GetOrStoreDefault(SKIP_PREPARE_FOR_SINGLE_MODPACK_STRING, gSkipPrepareIfSingleModpack, DEFAULT_SKIP_PREPARE_IF_SINGLE_MODPACK) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\"", gSkipPrepareIfSingleModpack);
    }

    if (WUPSStorageAPI::SaveStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to save storage");
    }

    WUPSConfigAPIOptionsV1 configOptions = {.name = "SDCafiine"};
    if (WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback) != WUPSCONFIG_API_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init config api");
    }

    sContentLayerHandle = 0;
    sAocLayerHandle     = 0;
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
    if (WUPSStorageAPI::Store(AUTO_APPLY_SINGLE_MODPACK_STRING, gAutoApplySingleModpack) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to close storage");
    }
}

void skipPrepareIfSingleModpackChanged(ConfigItemBoolean *item, bool newValue) {
    DEBUG_FUNCTION_LINE("New value in gSkipPrepareIfSingleModpack: %d", newValue);
    gSkipPrepareIfSingleModpack = newValue;
    // If the value has changed, we store it in the storage.
    if (WUPSStorageAPI::Store(SKIP_PREPARE_FOR_SINGLE_MODPACK_STRING, gSkipPrepareIfSingleModpack) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to close storage");
    }
}

void sdCafiineEnabledChanged(ConfigItemBoolean *item, bool newValue) {
    DEBUG_FUNCTION_LINE("New value in gSDCafiineEnabled: %d", newValue);
    gSDCafiineEnabled = newValue;
    // If the value has changed, we store it in the storage.
    if (WUPSStorageAPI::Store(SDCAFIINE_ENABLED_STRING, gSDCafiineEnabled) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to close storage");
    }
}

WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
    try {
        WUPSConfigCategory root = WUPSConfigCategory(rootHandle);


        root.add(WUPSConfigItemBoolean::Create(SDCAFIINE_ENABLED_STRING,
                                               "Enable SDCafiine (game needs to be restarted)",
                                               DEFAULT_SDCAFIINE_ENABLED, gSDCafiineEnabled,
                                               &sdCafiineEnabledChanged));


        auto advancedSettings = WUPSConfigCategory::Create("Advanced settings");

        advancedSettings.add(WUPSConfigItemBoolean::Create(AUTO_APPLY_SINGLE_MODPACK_STRING,
                                                           "Auto apply the modpack if only one modpack exists",
                                                           DEFAULT_AUTO_APPLY_SINGLE_MODPACK, gAutoApplySingleModpack,
                                                           &autoApplySingleModpackChanged));

        advancedSettings.add(WUPSConfigItemBoolean::Create(SKIP_PREPARE_FOR_SINGLE_MODPACK_STRING,
                                                           "Skip \"Preparing modpack...\" screen",
                                                           DEFAULT_SKIP_PREPARE_IF_SINGLE_MODPACK, gSkipPrepareIfSingleModpack,
                                                           &skipPrepareIfSingleModpackChanged));
        root.add(std::move(advancedSettings));

    } catch (std::exception &e) {
        OSReport("Exception T_T : %s\n", e.what());
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }
    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

void ConfigMenuClosedCallback() {
    // Save all changes
    if (WUPSStorageAPI::SaveStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to close storage");
    }
}

ON_APPLICATION_ENDS() {
    if (sContentLayerHandle != 0) {
        ContentRedirection_RemoveFSLayer(sContentLayerHandle);
        sContentLayerHandle = 0;
    }
    if (sAocLayerHandle != 0) {
        ContentRedirection_RemoveFSLayer(sAocLayerHandle);
        sAocLayerHandle = 0;
    }
    deinitLogging();
}