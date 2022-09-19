#include "modpackSelector.h"
#include "main.h"
#include "utils/input.h"
#include "version.h"
#include <content_redirection/redirection.h>
#include <coreinit/screen.h>
#include <coreinit/thread.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fs/DirList.h>
#include <malloc.h>
#include <map>
#include <memory/mappedmemory.h>
#include <string>
#include <utils/logger.h>
#include <wups/storage.h>

#define TEXT_SEL(x, text1, text2) ((x) ? (text1) : (text2))

void HandleMultiModPacks(uint64_t titleID) {
    char TitleIDString[17];
    snprintf(TitleIDString, 17, "%016llX", titleID);

    std::map<std::string, std::string> modTitlePath;

    std::map<std::string, std::string> mounting_points;

    const std::string modTitleIDPath = std::string("fs:/vol/external01/sdcafiine/").append(TitleIDString);
    DirList modTitleDirList(modTitleIDPath, nullptr, DirList::Dirs);

    modTitleDirList.SortList();

    for (int index = 0; index < modTitleDirList.GetFilecount(); index++) {
        std::string curFile = modTitleDirList.GetFilename(index);

        if (curFile == "." || curFile == "..") {
            continue;
        }

        const std::string &packageName = curFile;
        modTitlePath[packageName]      = (modTitleIDPath + "/").append(curFile);
        DEBUG_FUNCTION_LINE_VERBOSE("Found %s  %s", packageName.c_str(), modTitlePath[packageName].c_str());
    }

    if (modTitlePath.empty()) {
        return;
    }
    if (modTitlePath.size() == 1 && gSkipPrepareIfSingleModpack) {
        ReplaceContent(modTitlePath.begin()->second);
        return;
    }

    int selected   = 0;
    int initScreen = 1;
    int x_offset   = -2;

    // Init screen and screen buffers
    OSScreenInit();
    uint32_t screen_buf0_size = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t screen_buf1_size = OSScreenGetBufferSizeEx(SCREEN_DRC);
    auto *screenBuffer        = (uint8_t *) MEMAllocFromMappedMemoryForGX2Ex(screen_buf0_size + screen_buf1_size, 0x100);
    if (screenBuffer == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("Failed to alloc screenBuffer");
        OSFatal("SDCafiine plugin: Failed to alloc screenBuffer.");
        return;
    }
    OSScreenSetBufferEx(SCREEN_TV, (void *) screenBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, (void *) (screenBuffer + screen_buf0_size));

    OSScreenEnableEx(SCREEN_TV, 1);
    OSScreenEnableEx(SCREEN_DRC, 1);

    // Clear screens
    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);

    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);

    uint32_t buttonsTriggered;

    VPADStatus vpad_data{};
    VPADReadError vpad_error;
    KPADStatus kpad_data{};
    KPADError kpad_error;

    bool displayAutoSkipOption = modTitlePath.size() == 1;

    int wantToExit = 0;
    int page       = 0;
    int per_page   = displayAutoSkipOption ? 11 : 13;
    int max_pages  = (modTitlePath.size() / per_page) + 1;

    int curState = 0;
    if (gAutoApplySingleModpack && modTitlePath.size() == 1) {
        curState = 1;
    }

    int durationInFrames = 60;
    int frameCounter     = 0;
    KPADInit();
    WPADEnableURCC(true);

    while (true) {
        buttonsTriggered = 0;

        VPADRead(VPAD_CHAN_0, &vpad_data, 1, &vpad_error);
        if (vpad_error == VPAD_READ_SUCCESS) {
            buttonsTriggered = vpad_data.trigger;
        }

        for (int i = 0; i < 4; i++) {
            memset(&kpad_data, 0, sizeof(kpad_data));
            if (KPADReadEx((KPADChan) i, &kpad_data, 1, &kpad_error) > 0) {
                if (kpad_error == KPAD_ERROR_OK && kpad_data.extensionType != 0xFF) {
                    if (kpad_data.extensionType == WPAD_EXT_CORE || kpad_data.extensionType == WPAD_EXT_NUNCHUK) {
                        buttonsTriggered |= remapWiiMoteButtons(kpad_data.trigger);
                    } else {
                        buttonsTriggered |= remapClassicButtons(kpad_data.classic.trigger);
                    }
                }
            }
        }

        if (curState == 1) {
            if (buttonsTriggered & VPAD_BUTTON_MINUS) {
                curState = 0;
                continue;
            }

            if (initScreen) {
                OSScreenClearBufferEx(SCREEN_TV, 0);
                OSScreenClearBufferEx(SCREEN_DRC, 0);
                console_print_pos(x_offset, -1, "SDCafiine plugin " VERSION_FULL);
                console_print_pos(x_offset, 1, "Preparing modpack \"%s\"...", modTitlePath.begin()->first.c_str());
                console_print_pos(x_offset, 3, "Press MINUS to open menu");
                // Flip buffers
                OSScreenFlipBuffersEx(SCREEN_TV);
                OSScreenFlipBuffersEx(SCREEN_DRC);
            }

            if (frameCounter >= durationInFrames) {
                ReplaceContent(modTitlePath.begin()->second);
                break;
            }

            frameCounter++;
        } else {
            if (buttonsTriggered & VPAD_BUTTON_A) {
                wantToExit = 1;
                initScreen = 1;
            } else if (modTitlePath.size() == 1 && (buttonsTriggered & VPAD_BUTTON_MINUS)) {
                OSScreenClearBufferEx(SCREEN_TV, 0);
                OSScreenClearBufferEx(SCREEN_DRC, 0);

                console_print_pos(x_offset, -1, "SDCafiine plugin " VERSION_FULL);
                console_print_pos(x_offset, 1, "Save settings...");

                // Flip buffers
                OSScreenFlipBuffersEx(SCREEN_TV);
                OSScreenFlipBuffersEx(SCREEN_DRC);

                // We open the storage, so we can persist the configuration the user did.
                if (WUPS_OpenStorage() == WUPS_STORAGE_ERROR_SUCCESS) {
                    gAutoApplySingleModpack = !gAutoApplySingleModpack;
                    // If the value has changed, we store it in the storage.
                    if (WUPS_StoreInt(nullptr, AUTO_APPLY_SINGLE_MODPACK_STRING, gAutoApplySingleModpack) != WUPS_STORAGE_ERROR_SUCCESS) {
                    }
                    if (WUPS_CloseStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
                        DEBUG_FUNCTION_LINE_ERR("Failed to close storage");
                    }
                }
                initScreen = 1;
            } else if (buttonsTriggered & VPAD_BUTTON_B) {
                break;
            } else if (buttonsTriggered & VPAD_BUTTON_DOWN) {
                selected++;
                initScreen = 1;
            } else if (buttonsTriggered & VPAD_BUTTON_UP) {
                selected--;
                initScreen = 1;
            } else if (buttonsTriggered & VPAD_BUTTON_L) {
                selected -= per_page;
                initScreen = 1;
            } else if (buttonsTriggered & VPAD_BUTTON_R) {
                selected += per_page;
                initScreen = 1;
            }
            if (selected < 0) { selected = 0; }
            if (selected >= modTitlePath.size()) { selected = modTitlePath.size() - 1; }
            page = selected / per_page;

            if (initScreen) {
                OSScreenClearBufferEx(SCREEN_TV, 0);
                OSScreenClearBufferEx(SCREEN_DRC, 0);
                console_print_pos(x_offset, -1, "SDCafiine plugin " VERSION_FULL);
                console_print_pos(x_offset, 1, "Press A to launch a modpack");
                console_print_pos(x_offset, 2, "Press B to launch without a modpack");
                if (modTitlePath.size() == 1) {
                    if (gAutoApplySingleModpack) {
                        console_print_pos(x_offset, 4, "Press MINUS to disable autostart for a single modpack");
                    } else {
                        console_print_pos(x_offset, 4, "Press MINUS to enable autostart for a single modpack");
                    }
                }
                int y_offset = displayAutoSkipOption ? 6 : 4;
                int cur_     = 0;

                for (auto &it : modTitlePath) {
                    std::string key   = it.first;
                    std::string value = it.second;

                    if (wantToExit && cur_ == selected) {
                        ReplaceContent(value);
                        break;
                    }

                    if (cur_ >= (page * per_page) && cur_ < ((page + 1) * per_page)) {
                        console_print_pos(x_offset, y_offset++, "%s %s", TEXT_SEL((selected == cur_), "--->", "    "), key.c_str());
                    }
                    cur_++;
                }

                if (wantToExit) { //just in case.
                    break;
                }

                if (max_pages > 0) {
                    console_print_pos(x_offset, 17, "Page %02d/%02d. Press L/R to change page.", page + 1, max_pages);
                }

                // Flip buffers
                OSScreenFlipBuffersEx(SCREEN_TV);
                OSScreenFlipBuffersEx(SCREEN_DRC);

                initScreen = 0;
            }
        }
    }

    KPADShutdown();
    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);

    // Flip buffers
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);

    MEMFreeToMappedMemory(screenBuffer);
}
extern CRLayerHandle contentLayerHandle;
extern CRLayerHandle aocLayerHandle;

bool ReplaceContentInternal(const std::string &basePath, const std::string &subdir, CRLayerHandle *layerHandle);

bool ReplaceContent(const std::string &basePath) {
    bool contentRes = ReplaceContentInternal(basePath, "content", &contentLayerHandle);
    bool aocRes     = ReplaceContentInternal(basePath, "aoc", &aocLayerHandle);

    if (!contentRes && !aocRes) {
        DEBUG_FUNCTION_LINE_ERR("Failed to apply modpack. Starting without mods.");
        OSScreenClearBufferEx(SCREEN_TV, 0);
        OSScreenClearBufferEx(SCREEN_DRC, 0);
        console_print_pos(-2, -1, "SDCafiine plugin " VERSION VERSION_EXTRA);
        console_print_pos(-2, 1, "Failed to apply modpack. Starting without mods.");
        OSScreenFlipBuffersEx(SCREEN_TV);
        OSScreenFlipBuffersEx(SCREEN_DRC);

        OSSleepTicks(OSMillisecondsToTicks(3000));
        return false;
    }
    return true;
}

bool ReplaceContentInternal(const std::string &basePath, const std::string &subdir, CRLayerHandle *layerHandle) {
    std::string layerName = "SDCafiine /vol/" + subdir;
    std::string fullPath  = basePath + "/" + subdir;
    struct stat st {};
    if (stat(fullPath.c_str(), &st) < 0) {
        DEBUG_FUNCTION_LINE_WARN("Skip /vol/%s to %s redirection. Dir does not exist", subdir.c_str(), fullPath.c_str());
        return false;
    }

    auto res = ContentRedirection_AddFSLayer(layerHandle,
                                             layerName.c_str(),
                                             fullPath.c_str(),
                                             FS_LAYER_TYPE_CONTENT_MERGE);
    if (res == CONTENT_REDIRECTION_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE("Redirect /vol/%s to %s", subdir.c_str(), fullPath.c_str());
    } else {
        DEBUG_FUNCTION_LINE_ERR("Failed to redirect /vol/%s to %s", subdir.c_str(), fullPath.c_str());

        return false;
    }
    return true;
}


void console_print_pos(int x, int y, const char *format, ...) {
    char *tmp = nullptr;

    va_list va;
    va_start(va, format);
    if ((vasprintf(&tmp, format, va) >= 0) && tmp) {
        if (strlen(tmp) > 79) {
            tmp[79] = 0;
        }

        OSScreenPutFontEx(SCREEN_TV, x, y, tmp);
        OSScreenPutFontEx(SCREEN_DRC, x, y, tmp);
    }
    va_end(va);

    if (tmp) {
        free(tmp);
    }
}
