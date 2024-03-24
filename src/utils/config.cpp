#include "config.h"
#include "globals.h"
#include "logger.h"
#include <wups/config.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config_api.h>
#include <wups/storage.h>

static void bool_item_callback(ConfigItemBoolean *item, bool newValue) {
    if (!item || !item->identifier) {
        DEBUG_FUNCTION_LINE_WARN("Invalid item or identifier in bool item callback");
        return;
    }
    DEBUG_FUNCTION_LINE_VERBOSE("New value in %s changed: %d", item->identifier, newValue);
    if (std::string_view(AUTO_APPLY_SINGLE_MODPACK_STRING) == item->identifier) {
        gAutoApplySingleModpack = newValue;
    } else if (std::string_view(SKIP_PREPARE_FOR_SINGLE_MODPACK_STRING) == item->identifier) {
        gSkipPrepareIfSingleModpack = newValue;
    } else if (std::string_view(SDCAFIINE_ENABLED_STRING) == item->identifier) {
        gSDCafiineEnabled = newValue;
    } else {
        DEBUG_FUNCTION_LINE_WARN("Unexpected boolean item: %s", item->identifier);
        return;
    }
    WUPSStorageError err;
    if ((err = WUPSStorageAPI::Store(item->identifier, newValue)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_WARN("Failed to store value %d to storage item \"%s\": %s (%d)", newValue, item->identifier, WUPSStorageAPI_GetStatusStr(err), err);
    }
}

static WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
    try {
        WUPSConfigCategory root = WUPSConfigCategory(rootHandle);


        root.add(WUPSConfigItemBoolean::Create(SDCAFIINE_ENABLED_STRING,
                                               "Enable SDCafiine (game needs to be restarted)",
                                               DEFAULT_SDCAFIINE_ENABLED, gSDCafiineEnabled,
                                               &bool_item_callback));


        auto advancedSettings = WUPSConfigCategory::Create("Advanced settings");

        advancedSettings.add(WUPSConfigItemBoolean::Create(AUTO_APPLY_SINGLE_MODPACK_STRING,
                                                           "Auto apply the modpack if only one modpack exists",
                                                           DEFAULT_AUTO_APPLY_SINGLE_MODPACK, gAutoApplySingleModpack,
                                                           &bool_item_callback));

        advancedSettings.add(WUPSConfigItemBoolean::Create(SKIP_PREPARE_FOR_SINGLE_MODPACK_STRING,
                                                           "Skip \"Preparing modpack...\" screen",
                                                           DEFAULT_SKIP_PREPARE_IF_SINGLE_MODPACK, gSkipPrepareIfSingleModpack,
                                                           &bool_item_callback));
        root.add(std::move(advancedSettings));

    } catch (std::exception &e) {
        DEBUG_FUNCTION_LINE_ERR("Exception: %s\n", e.what());
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }
    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

static void ConfigMenuClosedCallback() {
    // Save all changes
    WUPSStorageError err;
    if ((err = WUPSStorageAPI::SaveStorage()) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to close storage: %s (%d)", WUPSStorageAPI_GetStatusStr(err), err);
    }
}


void InitStorageAndConfig() {
    WUPSStorageError err;
    if ((err = WUPSStorageAPI::GetOrStoreDefault(AUTO_APPLY_SINGLE_MODPACK_STRING, gAutoApplySingleModpack, DEFAULT_AUTO_APPLY_SINGLE_MODPACK)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", AUTO_APPLY_SINGLE_MODPACK_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }
    if ((err = WUPSStorageAPI::GetOrStoreDefault(SDCAFIINE_ENABLED_STRING, gSDCafiineEnabled, DEFAULT_SDCAFIINE_ENABLED)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", SDCAFIINE_ENABLED_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }
    if ((err = WUPSStorageAPI::GetOrStoreDefault(SKIP_PREPARE_FOR_SINGLE_MODPACK_STRING, gSkipPrepareIfSingleModpack, DEFAULT_SKIP_PREPARE_IF_SINGLE_MODPACK)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", gSkipPrepareIfSingleModpack, WUPSStorageAPI_GetStatusStr(err), err);
    }

    if ((err = WUPSStorageAPI::SaveStorage()) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to save storage: %s (%d)", WUPSStorageAPI_GetStatusStr(err), err);
    }

    WUPSConfigAPIOptionsV1 configOptions = {.name = "SDCafiine"};
    WUPSConfigAPIStatus configErr;
    if ((configErr = WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init config api: %s (%d)", WUPSConfigAPI_GetStatusStr(configErr), configErr);
    }
}
