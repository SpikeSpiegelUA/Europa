#include "Common.h"
#include "CommonHeaders.h"
#include <Windows.h>

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

namespace {
	HMODULE GameCodeDLL{ nullptr };
} //anonymous namespace

EDITOR_INTERFACE
uint32 LoadGameCodeDLL(const char* dll_path) {
	if (GameCodeDLL)
		return FALSE;
	GameCodeDLL = LoadLibraryA(dll_path);
	assert(GameCodeDLL);

	return GameCodeDLL ? TRUE : FALSE;
}

EDITOR_INTERFACE
uint32 UnloadGameCodeDLL() {
	if (!GameCodeDLL)
		return FALSE;
	assert(GameCodeDLL);
	assert(FreeLibrary(GameCodeDLL));
	GameCodeDLL = nullptr;
	return TRUE;
}
