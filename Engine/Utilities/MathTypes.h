#pragma once
#include "../Common/CommonHeaders.h"

namespace Europa::Math {
	constexpr float PI = 3.141592653589793238462643383279502884197f;
	constexpr float TwoPI = 2 * PI;
	constexpr float Epsilon = 1e-5f;
#if defined (_WIN64)
	using Vector2 = DirectX::XMFLOAT2;
	using Vector2A = DirectX::XMFLOAT2A;
	using Vector3 = DirectX::XMFLOAT3;
	using Vector3A = DirectX::XMFLOAT3A;
	using Vector4 = DirectX::XMFLOAT4;
	using Vector4A = DirectX::XMFLOAT4A;
	using Int32Vector2 = DirectX::XMINT2;
	using Int32Vector3 = DirectX::XMINT3;
	using Int32Vector4 = DirectX::XMINT4;
	using UInt32Vector2 = DirectX::XMUINT2;
	using UInt32Vector3 = DirectX::XMUINT3;
	using UInt32Vector4 = DirectX::XMUINT4;
	using Matrix3x3 = DirectX::XMFLOAT3X3; //NOTE: DirectX doesn't have aligned 3x3 matrix.
	using Matrix4x4 = DirectX::XMFLOAT4X4;
	using Matrix4x4A = DirectX::XMFLOAT4X4A;
#endif
}