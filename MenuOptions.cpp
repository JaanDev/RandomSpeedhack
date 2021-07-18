#include "MenuOptions.h"

bool __fastcall MenuOptions::initHook(void* self)
{
    // Add all the settings first
    bool result = MenuOptions::init(self);

    // Add my setting
    MenuOptions::addToggle(
        self,
        "Enable random speedhack",
        "random-speedhack",
        "Made by Jaan#2897 for Rektor#7809. Original idea by Virone#3483");

    return result;
}

void MenuOptions::mem_init() {
    // Hook
    size_t base = reinterpret_cast<size_t>(GetModuleHandle(0));
    MH_CreateHook(
        (PVOID)(base + 0x1DE8F0),
        MenuOptions::initHook,
        (PVOID*)&MenuOptions::init);

    // Get the function
    MenuOptions::addToggle = reinterpret_cast<decltype(MenuOptions::addToggle)>(base + 0x1DF6B0);
}