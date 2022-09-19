#pragma once

#include <cstdint>
#include <string>

void HandleMultiModPacks(uint64_t titleid /*,bool showMenu = true*/);
void console_print_pos(int x, int y, const char *format, ...);
bool ReplaceContent(const std::string &basePath, const std::string &modpack);