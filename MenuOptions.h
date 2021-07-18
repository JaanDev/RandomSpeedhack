#pragma once
#include "minhook/include/MinHook.h"

namespace MenuOptions {
	// Original functions
	inline int(__thiscall* addToggle)(void* self, const char* display, const char* key, const char* extraInfo);
	inline bool(__thiscall* init)(void*);

	// Hook function
	bool __fastcall initHook(void* self);

	// Hook init function
	void mem_init();
}