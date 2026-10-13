#include <ShaderCompiler.hpp>

#include <cstdlib>
#include <utility>

#ifdef _WIN32
#include <Windows.h>
#include <wrl/client.h>
#else
#include <dxc/Support/WinAdapter.h>
#endif

#ifdef _WIN32
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
#else
template<typename T>
using ComPtr = CComPtr<T>;
#endif
#include <dxc/dxcapi.h>

#include <spirv_cross.hpp>
#include <spirv_msl.hpp>

namespace Ti::ShaderCompiler
{
    using namespace std::literals;

    struct DXCContext
    {
        ComPtr<IDxcCompiler3> compiler;
        ComPtr<IDxcUtils> utils;
        ComPtr<IDxcIncludeHandler> includeHandler;
    };

    static DXCContext& getContext()
    {
        static thread_local DXCContext context;

        HRESULT hres;

        if (!context.compiler)
        {
            hres = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(context.compiler.GetAddressOf()));
            if (FAILED(hres))
            {
                exit(-1);
            }
        }

        if (!context.utils)
        {
            hres = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(context.utils.GetAddressOf()));
            if (FAILED(hres))
            {
                exit(-1);
            }
        }
        if (!context.includeHandler)
        {
            hres = context.utils->CreateDefaultIncludeHandler(context.includeHandler.GetAddressOf());

            if (FAILED(hres))
            {
                exit(-1);
            }
        }

        return context;
    }

    std::expected<std::wstring_view, Compiler::GetCodeError> getShaderStageProfile(Compiler::ShaderStage shaderStage)
    {
        using enum Ti::ShaderCompiler::Compiler::ShaderStage;

        switch (shaderStage)
        {
        case Vertex:
            return L"vs_6_6";
        case Hull:
            return L"hs_6_6";
        case Domain:
            return L"ds_6_6";
        case Geometry:
            return L"gs_6_6";
        case Pixel:
            return L"ps_6_6";
        case Amplification:
            return L"as_6_6";
        case Mesh:
            return L"ms_6_6";
        case Compute:
            return L"cs_6_6";

        case RayGeneration:
        case AnyHit:
        case ClosestHit:
        case Miss:
        case Intersection:
        case Callable:
            return L"lib_6_6";

        default:
            break;
        }

        return std::unexpected(Compiler::GetCodeError::InvalidShaderStage);
    }

    std::wstring_view getMatrixConvention(Compiler::MatrixConvention conv)
    {
        return conv == Compiler::MatrixConvention::Row ? L"-Zpc" : L"-Zpr";
    }

    bool AddApiArgument(Compiler::ApiTarget api, std::vector<LPCWSTR>& args, const Compiler::ShaderStage shaderStage)
    {
        using enum Compiler::ApiTarget;

        switch (api)
        {
        case Dxil:
            return true;
        case Spirv:
            args.emplace_back(L"-spirv");
            args.emplace_back(L"-fspv-target-env=vulkan1.3");
            if (shaderStage == Compiler::ShaderStage::Mesh || shaderStage == Compiler::ShaderStage::Amplification)
            {
                args.emplace_back(L"-fspv-extension=SPV_EXT_mesh_shader");
            }
            return true;

        case Msl:
            args.emplace_back(L"-spirv");
            return true;
        }

        return false;
    }

    Compiler::Compiler()
    {
        std::ignore = getContext();
    }

    std::expected<std::vector<std::vector<std::byte>>, Compiler::CompileError>
    Compiler::getCode(const GetCodeInfo& getCodeInfo)
    {
        if (getCodeInfo.entryPoints.empty())
            return std::unexpected(GetCodeError::EmptyEntryPoints);

        HRESULT hres;

        ComPtr<IDxcBlobEncoding> sourceBlob;
        const std::span<const std::byte> source = getCodeInfo.code;
        if (source.empty())
            return std::unexpected(GetCodeError::InvalidSourceCode);

        hres =
            getContext().utils->CreateBlob(source.data(), static_cast<UINT32>(source.size()), DXC_CP_UTF8, &sourceBlob);

        if (FAILED(hres) || !sourceBlob)
            return std::unexpected(GetCodeError::LoadingCodeFailed);

        DxcBuffer buffer{};
        buffer.Ptr = sourceBlob->GetBufferPointer();
        buffer.Size = sourceBlob->GetBufferSize();
        buffer.Encoding = DXC_CP_UTF8;

        std::vector<std::vector<std::byte>> entryPointsCodes;
        entryPointsCodes.reserve(getCodeInfo.entryPoints.size());

        std::vector<LPCWSTR> args;
        args.emplace_back(getMatrixConvention(getCodeInfo.matrixConvention).data());
        for (const auto& shaderEntryPoint : getCodeInfo.entryPoints)
        {
            if (shaderEntryPoint.stage == ShaderStage::Geometry && getCodeInfo.apiTarget == ApiTarget::Msl)
                return std::unexpected(GetCodeError::UnexpectedError); // there is no geometry stage in metal 

            args.resize(1);
            // EntryPoint
            {
                args.emplace_back(L"-E");
                args.emplace_back(shaderEntryPoint.entryPoint.data());
            }

            // TargetProfile
            {
                std::expected<std::wstring_view, Compiler::GetCodeError> profile =
                    getShaderStageProfile(shaderEntryPoint.stage);
                if (!profile)
                    return std::unexpected(profile.error());

                args.emplace_back(L"-T");
                args.emplace_back(profile.value().data());
            }

            if (getCodeInfo.includePath)
            {
                args.emplace_back(L"-I");
                args.emplace_back((*getCodeInfo.includePath).data());
            }

            if (!AddApiArgument(getCodeInfo.apiTarget, args, shaderEntryPoint.stage))
            {
                return std::unexpected(GetCodeError::InvalidApi);
            }

            auto& ctx = getContext();

            ComPtr<IDxcResult> result;

            HRESULT hres = ctx.compiler->Compile(&buffer, args.data(), static_cast<UINT32>(args.size()),
                                                 ctx.includeHandler.Get(), IID_PPV_ARGS(&result));
            if (FAILED(hres))
            {
                HRESULT compileStatus;
                result->GetStatus(&compileStatus);

                ComPtr<IDxcBlobUtf8> errors;
                result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
                return std::unexpected(std::string(errors->GetStringPointer()));
            }

            ComPtr<IDxcBlob> code;
            if (result)
            {
                hres = result->GetResult(&code);
                if (FAILED(hres) || !code)
                {
                    return std::unexpected(GetCodeError::ShaderCompilationFailed);
                }

                const auto* dataPtr = static_cast<const std::byte*>(code->GetBufferPointer());
                const size_t dataSize = code->GetBufferSize();
                if (dataSize == 0 || !dataPtr)
                {
                    return std::unexpected(GetCodeError::ShaderCompilationFailed);
                }
                if (getCodeInfo.apiTarget == ApiTarget::Msl)
                {
                    // convert spirv to msl
                    spirv_cross::CompilerMSL glsl(
                        std::vector<uint32_t>{reinterpret_cast<const std::uint32_t*>(dataPtr),
                                              reinterpret_cast<const std::uint32_t*>(dataPtr + dataSize)});
                    const std::string codeMsl = glsl.compile();
                    if (codeMsl.empty())
                        return std::unexpected(GetCodeError::FailedCrossCompileToMSL);

                    entryPointsCodes.emplace_back(reinterpret_cast<const std::byte*>(codeMsl.data()),
                                                  reinterpret_cast<const std::byte*>(codeMsl.data() + codeMsl.size()));
                }
                else
                {
                    entryPointsCodes.emplace_back(dataPtr, dataPtr + dataSize);
                }
                
            }
            else
            {
                return std::unexpected(GetCodeError::ShaderCompilationFailed);
            }
        }

        return entryPointsCodes;
    }

} // Ti::ShaderCompiler
