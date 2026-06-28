#if INCLUDE_WINDOW_CPP
#include "Window.h"
void Europa::Platform::Window::SetFullscreen(bool isFullscreen) const {
	assert(IsValid());
	SetWindowFullscreen(ID, isFullscreen);
}

bool Europa::Platform::Window::IsFullscreen() const {
	assert(IsValid());
	return IsWindowFullscreen(ID);
}

void* Europa::Platform::Window::Handle() const {
	assert(IsValid());
	return GetWindowHandle(ID);
}

void Europa::Platform::Window::SetCaption(const wchar_t* caption) const {
	assert(IsValid());
	SetWindowCaption(ID, caption);
}

const Math::UInt32Vector4 Europa::Platform::Window::Size() const {
	assert(IsValid());
	return GetWindowSize(ID);
}

void Europa::Platform::Window::Resize(uint32 width, uint32 height) const {
	assert(IsValid());
	ResizeWindow(ID, width, height);
}

const uint32 Europa::Platform::Window::Width() const {
	Math::UInt32Vector4 s{ Size() };
	return s.z - s.x;
}

const uint32 Europa::Platform::Window::Height() const {
	Math::UInt32Vector4 s{ Size() };
	return s.w - s.y;
}

bool Europa::Platform::Window::IsClosed() const {
	assert(IsValid());
	return IsWindowClosed(ID);
}
#endif //INCLUDE_WINDOW_CPP