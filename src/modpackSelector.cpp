#include <string>
#include <map>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <malloc.h>
#include "modpackSelector.h"

#include <coreinit/thread.h>
#include <memory/mappedmemory.h>

#include <utils/logger.h>
#include <fs/DirList.h>
#include <coreinit/screen.h>
#include <vpad/input.h>
#include <rpxloader.h>

#define TEXT_SEL(x, text1, text2)           ((x) ? (text1) : (text2))


void ReplaceContent(const std::string &basePath);

void HandleMultiModPacks(uint64_t titleID) {
    char TitleIDString[17];
    snprintf(TitleIDString, 17, "%016llX", titleID);

    std::map<std::string, std::string> modTitlePath;

    std::map<std::string, std::string> mounting_points;


    std::string modTitleIDPath = std::string("fs:/vol/external01/sdcafiine/") + TitleIDString;
    DirList modTitleDirList(modTitleIDPath.c_str(), nullptr, DirList::Dirs);

    modTitleDirList.SortList();

    for (int index = 0; index < modTitleDirList.GetFilecount(); index++) {
        std::string curFile = modTitleDirList.GetFilename(index);
        //DEBUG_FUNCTION_LINE("curFile %s \n",curFile.c_str());
        if (curFile.compare(".") == 0 || curFile.compare("..") == 0) {
            continue;
        }

        std::string packageName = curFile;
        modTitlePath[packageName] = modTitleIDPath.append("/").append(curFile);
        DEBUG_FUNCTION_LINE("found %s", packageName.c_str());
    }

    if (modTitlePath.empty()) {
        return;
    }

    int selected = 0;
    int initScreen = 1;
    int x_offset = -2;

    // Init screen and screen buffers
    OSScreenInit();
    uint32_t screen_buf0_size = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t screen_buf1_size = OSScreenGetBufferSizeEx(SCREEN_DRC);
    uint8_t *screenBuffer = (uint8_t *) MEMAllocFromMappedMemoryForGX2Ex(screen_buf0_size + screen_buf1_size, 0x100
    );
    if (screenBuffer == nullptr) {
        DEBUG_FUNCTION_LINE("Failed to alloc");
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

    VPADStatus vpad_data;
    VPADReadError error;

    int wantToExit = 0;
    int page = 0;
    int per_page = 13;
    int max_pages = (modTitlePath.size() / per_page) + 1;

    while (true) {

        error = VPAD_READ_NO_SAMPLES;
        VPADRead(VPAD_CHAN_0, &vpad_data, 1, &error);

        if (error == VPAD_READ_SUCCESS) {
            if (vpad_data.trigger & VPAD_BUTTON_A) {
                wantToExit = 1;
                initScreen = 1;
            } else if (vpad_data.trigger & VPAD_BUTTON_B) {
                break;
            } else if (vpad_data.trigger & VPAD_BUTTON_DOWN) {
                selected++;
                initScreen = 1;
            } else if (vpad_data.trigger & VPAD_BUTTON_UP) {
                selected--;
                initScreen = 1;
            } else if (vpad_data.trigger & VPAD_BUTTON_L) {
                selected -= per_page;
                initScreen = 1;
            } else if (vpad_data.trigger & VPAD_BUTTON_R) {
                selected += per_page;
                initScreen = 1;
            }
            if (selected < 0) { selected = 0; }
            if (selected >= modTitlePath.size()) { selected = modTitlePath.size() - 1; }
            page = selected / per_page;
        }

        if (initScreen) {
            OSScreenClearBufferEx(SCREEN_TV, 0);
            OSScreenClearBufferEx(SCREEN_DRC, 0);
            console_print_pos(x_offset, -1, "                  -- SDCafiine plugin %s by Maschell --");
            console_print_pos(x_offset, 1, "Select your options and press A to launch.");
            console_print_pos(x_offset, 2, "Press B to launch without mods");
            int y_offset = 4;
            int cur_ = 0;

            for (auto &it : modTitlePath) {
                std::string key = it.first;
                std::string value = it.second;

                if (wantToExit && cur_ == selected) {
                    ReplaceContent(value.append("/content"));
                    //snprintf(gModFolder, FS_MAX_ENTNAME_SIZE, "%s", value.c_str());
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
        OSSleepTicks(OSMillisecondsToTicks(100));
    }

    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);

    // Flip buffers
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);

    MEMFreeToMappedMemory(screenBuffer);

    return;
}

void ReplaceContent(const std::string &basePath) {
    if (RL_RedirectContentWithFallback(basePath.c_str())) {
        DEBUG_FUNCTION_LINE("redirect /vol/content to %s", basePath.c_str());
    } else {
        DEBUG_FUNCTION_LINE("ERROR: Failed to redirect /vol/content to %s", basePath.c_str());
    }
}

void console_print_pos(int x, int y, const char *format, ...) {
    char *tmp = NULL;

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
