#include <d3d12shader.h>
#include <dxcapi.h>

#include "Graphics\Direct3D12\D3D12Core.h"
#include "Graphics\Direct3D12\D3D12Shaders.h"

using namespace Europa;
using namespace Europa::Graphics::D3D12::Shaders;
using namespace Microsoft::WRL;

#include <fstream>
#include <filesystem>
namespace {

	struct ShaderFileInfo {
		const char* File;
		const char* Function;
		EngineShader::ID ID;
		ShaderType::Type Type;
	};

	constexpr ShaderFileInfo ShaderFiles[]
	{
		{"FullScreenTriangle.hlsl", "FullScreenTriangleVS", EngineShader::FullscreenTriangleVS, ShaderType::Vertex},
		{"FillColor.hlsl", "FillColorPS", EngineShader::FillColorPS, ShaderType::Pixel},
				{"PostProcess.hlsl", "PostProcessPS", EngineShader::PostProcessPS, ShaderType::Pixel}
	};

	static_assert(_countof(ShaderFiles) == EngineShader::Count);

	constexpr const char* ShadersSourcePath{ "../../Engine/Graphics/Direct3D12/Shaders/" };

	std::wstring ToWString(const char* c) 
	{
		std::string s{ c };
		return { s.begin(), s.end() };
	}

	class ShaderCompiler {
	public:
		ShaderCompiler() {
			HRESULT hr{ S_OK };
			DXCall(hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));
			if (FAILED(hr))
				return;
			DXCall(hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));
			if (FAILED(hr))
				return;
			DXCall(hr = utils->CreateDefaultIncludeHandler(&includeHandler));
			if (FAILED(hr))
				return;
		}

		DISABLE_COPY_AND_MOVE(ShaderCompiler);

		IDxcBlob* Compile(ShaderFileInfo info, std::filesystem::path fullPath) {
			assert(compiler && utils && includeHandler);
			HRESULT hr{ S_OK };	

			//Load the source file using Utils interface.
			ComPtr<IDxcBlobEncoding> sourceBlob{ nullptr };
			DXCall(hr = utils->LoadFile(fullPath.c_str(), nullptr, &sourceBlob));
			if (FAILED(hr))
				return nullptr;
			assert(sourceBlob && sourceBlob->GetBufferSize());

			std::wstring file{ ToWString(info.File) };
			std::wstring func{ ToWString(info.Function) };
			std::wstring prof{ ToWString(profileStrings[(uint32)info.Type]) };
			std::wstring inc{ ToWString(ShadersSourcePath) };

			LPCWSTR args[]
			{
				file.c_str(),				//Optional shader source file name for error reporting
				L"-E", func.c_str(),		//Entry function
				L"-T", prof.c_str(),		//Target profile
				L"-I", inc.c_str(),			//Include path
				DXC_ARG_ALL_RESOURCES_BOUND,
#if _DEBUG
				DXC_ARG_DEBUG,
				DXC_ARG_SKIP_OPTIMIZATIONS,
#else
				DXC_ARG_OPTIMIZATION_LEVEL3,
#endif
				DXC_ARG_WARNINGS_ARE_ERRORS,
				L"-Qstrip_reflect",			//Strip reflections into a separate blob
				L"-Qstrip_debug",			//Strip debug information into a separate blob
			};

			OutputDebugStringA("Compiling");
			OutputDebugStringA(info.File);

			return CompileBlob(sourceBlob.Get(), args, _countof(args));
		}

		IDxcBlob* CompileBlob(IDxcBlobEncoding* sourceBlob, LPCWSTR* args, uint32 numArgs) 
		{
			DxcBuffer buffer{};
			buffer.Encoding = DXC_CP_ACP; //auto-detect text format.
			buffer.Ptr = sourceBlob->GetBufferPointer();
			buffer.Size = sourceBlob->GetBufferSize();

			HRESULT hr{ S_OK };
			ComPtr<IDxcResult> results{ nullptr };
			DXCall(hr = compiler->Compile(&buffer, args, numArgs, includeHandler.Get(), IID_PPV_ARGS(&results)));
			if (FAILED(hr))
				return nullptr;

			ComPtr<IDxcBlobUtf8> errors{ nullptr };
			DXCall(hr = results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
			if (FAILED(hr))
				return nullptr;

			if (errors && errors->GetStringLength()) 
			{
				OutputDebugStringA("\nShader compilation error: \n");
				OutputDebugStringA(errors->GetStringPointer());
			}
			else {
				OutputDebugStringA(" [Succeeded]");
			}

			OutputDebugStringA("\n");

			HRESULT status{ S_OK };
			DXCall(hr = results->GetStatus(&status));
			if (FAILED(hr) || FAILED(status))
				return nullptr;

			ComPtr<IDxcBlob> shader{ nullptr };
			DXCall(hr = results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr));
			if (FAILED(hr))
				return nullptr;

			return shader.Detach();
		}
	private:
		//NOTE: Shader Model 6.x can also be used (AS and MS are only supported on SM6.5 on).
		constexpr static const char* profileStrings[]{ "vs_6_5", "hs_6_5", "ds_6_5", "gs_6_5", "ps_6_5", "cs_6_5", "as_6_5", "ms_6_5"};
		static_assert(_countof(profileStrings) == ShaderType::Count);

		ComPtr<IDxcCompiler3> compiler{ nullptr };
		ComPtr<IDxcUtils> utils{ nullptr };
		ComPtr<IDxcIncludeHandler> includeHandler{ nullptr };
	};

	decltype(auto) GetEngineShadersPath() 
	{
		return std::filesystem::path{ Graphics::GetEngineShadersPath(Graphics::GraphicsPlatform::Direct3D12) };
	}

	bool CompiledShadersAreUpToDate() {
		auto engineShadersPath = GetEngineShadersPath();
		if (!std::filesystem::exists(engineShadersPath))
			return false;
		auto shadersCompilationTime = std::filesystem::last_write_time(engineShadersPath);

		std::filesystem::path path{};
		std::filesystem::path fullPath{};

		//Check if either of engine shader source files is newer that the compiled shader file.
		//In that case, we need to recompile.
		for (uint32 i{ 0 }; i < EngineShader::Count; ++i) 
		{
			auto& info = ShaderFiles[i];

			path = ShadersSourcePath;
			path += info.File;
			fullPath = path;
			if (!std::filesystem::exists(fullPath))
				return false;

			auto shaderFileTime = std::filesystem::last_write_time(fullPath);
			if (shaderFileTime > shadersCompilationTime) 
			{
				return false;
			}
		}

		return true;
	}

	bool SaveCompiledShaders(Utilities::Vector<ComPtr<IDxcBlob>>& shaders)
	{
		auto engineShadersPath = GetEngineShadersPath();
		std::filesystem::create_directories(engineShadersPath.parent_path());
		std::ofstream file(engineShadersPath, std::ios::out | std::ios::binary);
		if (!file || !std::filesystem::exists(engineShadersPath)) {
			file.close();
			return false;
		}

		for (auto& shader : shaders)
		{
			const D3D12_SHADER_BYTECODE bytecode{ shader->GetBufferPointer(), shader->GetBufferSize() };
			file.write((char*)&bytecode.BytecodeLength, sizeof(bytecode.BytecodeLength));
			file.write((char*)bytecode.pShaderBytecode, bytecode.BytecodeLength);
 		}

		file.close();
		return true;
	}
} // Anonymous namespace

bool CompileShaders() 
{
	if (CompiledShadersAreUpToDate())
		return true;
	Utilities::Vector<ComPtr<IDxcBlob>> shaders;
	std::filesystem::path path{};
	std::filesystem::path fullPath{};

	ShaderCompiler compiler{};
	//Compile shaders and them together in a buffer in the same order of compilation.
	for (uint32 i{ 0 }; i < EngineShader::Count; ++i) 
	{
		auto& info = ShaderFiles[i];

		path = ShadersSourcePath;
		path += info.File;
		fullPath = path;
		if (!std::filesystem::exists(fullPath))
			return false;
		ComPtr<IDxcBlob> compiledShader{ compiler.Compile(info, fullPath)};
		if (compiledShader && compiledShader->GetBufferPointer() && compiledShader->GetBufferSize())
		{
			shaders.EmplaceBack(std::move(compiledShader));
		}
		else 
		{
			return false;
		}
	}

	return SaveCompiledShaders(shaders);
}