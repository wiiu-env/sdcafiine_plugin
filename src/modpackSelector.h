#ifndef _MODPACK_SELECTOR_H_
#define _MODPACK_SELECTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

void HandleMultiModPacks(uint64_t titleid/*,bool showMenu = true*/);
void console_print_pos(int x, int y, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif //_MODPACK_SELECTOR_H_
