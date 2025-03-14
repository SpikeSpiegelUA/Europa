#include "Common.h"
#include "CommonHeaders.h"
#include "..\Engine\Components\Script.h"
#include <Windows.h>

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

namespace {
	HMODULE GameCodeDLL{ nullptr };
	using _GetScriptCreator = Europa::Script::Internal::ScriptCreator(*)(size_t);
	_GetScriptCreator _getScriptCreator{ nullptr };
	using _GetScriptNames = LPSAFEARRAY(*)(void);
	_GetScriptNames _getScriptNames{ nullptr };
} //anonymous namespace

EDITOR_INTERFACE
uint32 LoadGameCodeDLL(const char* dll_path) {
	if (GameCodeDLL)
		return FALSE;
	GameCodeDLL = LoadLibraryA(dll_path);
	assert(GameCodeDLL);

	_getScriptCreator = (_GetScriptCreator)GetProcAddress(GameCodeDLL, "GetScriptCreator");
	_getScriptNames = (_GetScriptNames)GetProcAddress(GameCodeDLL, "GetScriptNames");

	return (GameCodeDLL && _getScriptCreator && _getScriptNames) ? TRUE : FALSE;
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

EDITOR_INTERFACE
Europa::Script::Internal::ScriptCreator EngineDLL_GetScriptCreator(const char* name) {
	return (GameCodeDLL && _getScriptCreator) ? _getScriptCreator(Europa::Script::Internal::StringHash()(name)) : nullptr;
}

EDITOR_INTERFACE 
LPSAFEARRAY EngineDLL_GetScriptNames() {
	return (GameCodeDLL && _getScriptNames) ? _getScriptNames() : nullptr;
}
