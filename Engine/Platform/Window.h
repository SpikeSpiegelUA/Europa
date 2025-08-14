#pragma once
#include "../Common/CommonHeaders.h"

namespace Europa::Platform {

	DEFINE_TYPED_ID(WindowID)

	class Window {
	public:
		constexpr explicit Window(WindowID id) : ID{ id } {}
		//Creates an entity with an invalid index.
		constexpr Window() : ID{ ID::InvalidID } {}
		constexpr WindowID GetID() const { return ID; }
		constexpr bool IsValid() const { return ID::IsValid(ID); }

		void SetFullscreen(bool isFullscreen) const;
		bool IsFullscreen() const;
		void* Handle() const;
		void SetCaption(const wchar_t* caption) const;
		const Math::UInt32Vector4 Size() const;
		void Resize(uint32 width, uint32 height) const;
		const uint32 Width() const;
		const uint32 Height() const;
		bool IsClosed() const;
	private:
		WindowID ID{ ID::InvalidID };
	};
}