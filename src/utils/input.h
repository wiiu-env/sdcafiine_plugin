#pragma once

#include <cstdint>
#include <padscore/kpad.h>
#include <vpad/input.h>

uint32_t remapWiiMoteButtons(uint32_t buttons);
uint32_t remapClassicButtons(uint32_t buttons);