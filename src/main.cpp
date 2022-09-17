#include "modpackSelector.h"
#include "utils/logger.h"
#include <content_redirection/redirection.h>
#include <coreinit/title.h>
#include <wups.h>

WUPS_PLUGIN_NAME("SDCafiine");
WUPS_PLUGIN_DESCRIPTION("SDCafiine");
WUPS_PLUGIN_VERSION("0.1");
WUPS_PLUGIN_AUTHOR("Maschell");
WUPS_PLUGIN_LICENSE("GPL");

WUPS_USE_WUT_DEVOPTAB();

CRLayerHandle contentLayerHandle __attribute__((section(".data"))) = 0;

INITIALIZE_PLUGIN() {
    // But then use libcontentredirection instead.
    ContentRedirectionStatus error;
    if ((error = ContentRedirection_InitLibrary()) != CONTENT_REDIRECTION_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init ContentRedirection. Error %d", error);
        OSFatal("Failed to init ContentRedirection.");
    }
}

/* Entry point */
ON_APPLICATION_START() {
    initLogging();
    HandleMultiModPacks(OSGetTitleID());
}

ON_APPLICATION_ENDS() {
    if (contentLayerHandle != 0) {
        ContentRedirection_RemoveFSLayer(contentLayerHandle);
        contentLayerHandle = 0;
    }
    deinitLogging();
}