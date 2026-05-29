#include "D3D12Shaders.h"
#include "Content/ContentLoader.h"

namespace Europa::Graphics::D3D12::Shaders {
	namespace {

		typedef struct CompiledShader {
			uint64 Size;
			const uint8* ByteCode;
		} const *CompiledShaderPtr ;

		//Each element in this array points to an offset within the shaders blob.
		CompiledShaderPtr EngineShaders[EngineShader::Count]{};

		//this is a chunk of memory that contains all compiled engine shaders.
		//The blob is an array of shader byte code consisting of a uint64 size and an array of bytes.
		std::unique_ptr <uint8[]> ShaderBlob{};

		bool LoadEngineShaders() 
		{
			assert(!ShaderBlob);
			uint64 size{ 0 };
			bool result{ Content::LoadEngineShaders(ShaderBlob, size) };
			if(!result)
				return result;
			assert(ShaderBlob && size);

			uint64 offset{ 0 };
			uint32 index{ 0 };
			while (offset < size && result) 
			{
				assert(index < EngineShader::Count);
				CompiledShaderPtr& shader{ EngineShaders[index] };
				assert(!shader);
				result &= index < EngineShader::Count && !shader;
				if (!result)
					break;
				shader = reinterpret_cast<const CompiledShaderPtr>(&ShaderBlob[offset]);
				offset += sizeof(uint64) + shader->Size;
				++index;
			}

			assert(offset == size && index == EngineShader::Count);

			return result;
		}

	} //anonymous namespace

	bool Initialize() 
	{
		return LoadEngineShaders();
	}

	void Shutdown() 
	{
		for (uint32 i{ 0 }; i < EngineShader::Count; ++i) 
		{
			EngineShaders[i] = {};
		}
		ShaderBlob.reset();
	}

	D3D12_SHADER_BYTECODE GetEngineShader(EngineShader::ID id) 
	{
		assert(id < EngineShader::Count);
		const CompiledShaderPtr shader{ EngineShaders[id] };
		assert(shader && shader->Size);
		return { &shader->ByteCode, shader->Size };
	}
}