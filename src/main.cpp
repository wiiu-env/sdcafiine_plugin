#include <wups.h>
#include <whb/log_udp.h>
#include "modpackSelector.h"
#include <coreinit/title.h>

WUPS_PLUGIN_NAME("SDCafiine");
WUPS_PLUGIN_DESCRIPTION("Wiiload Server");
WUPS_PLUGIN_VERSION("0.1");
WUPS_PLUGIN_AUTHOR("Maschell");
WUPS_PLUGIN_LICENSE("GPL");

WUPS_USE_WUT_DEVOPTAB();

/* Entry point */
ON_APPLICATION_START() {
    WHBLogUdpInit();
    HandleMultiModPacks(OSGetTitleID());
}