#include "Common.h"
#include "CommonHeaders.h"
#include "..\Engine\Components\Script.h"
#include "..\Graphics\Renderer.h"
#include "..\Platform\PlatformTypes.h"
#include "..\Platform\Platform.h"

#include <Windows.h>

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

using namespace Europa;

namespace {
	HMODULE GameCodeDLL{ nullptr };
	using _GetScriptCreator = Script::Internal::ScriptCreator(*)(size_t);
	_GetScriptCreator _getScriptCreator{ nullptr };
	using _GetScriptNames = LPSAFEARRAY(*)(void);
	_GetScriptNames _getScriptNames{ nullptr };
	Utilities::Vector<Graphics::RenderSurface> renderSurfaces;
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
Script::Internal::ScriptCreator EngineDLL_GetScriptCreator(const char* name) {
	return (GameCodeDLL && _getScriptCreator) ? _getScriptCreator(Script::Internal::StringHash()(name)) : nullptr;
}

EDITOR_INTERFACE 
LPSAFEARRAY EngineDLL_GetScriptNames() {
	return (GameCodeDLL && _getScriptNames) ? _getScriptNames() : nullptr;
}

EDITOR_INTERFACE uint32 CreateRenderSurface(HWND host, int32 width, int32 height) {
	assert(host);
	Platform::WindowInitInfo info{ nullptr, host, nullptr, 0, 0, width, height };
	Graphics::RenderSurface surface{ Platform::EngineCreateWindow(&info), {} };
	assert(surface.Window.IsValid());
	renderSurfaces.emplace_back(surface);
	return (uint32)renderSurfaces.size() - 1;
}

EDITOR_INTERFACE void RemoveRenderSurface(uint32 id) {
	assert(id < renderSurfaces.size());
	Platform::RemoveWindow(renderSurfaces[id].Window.GetID());
}

EDITOR_INTERFACE HWND GetWindowHandle(uint32 id) {
	assert(id < renderSurfaces.size());
	return (HWND)renderSurfaces[id].Window.Handle();
}
