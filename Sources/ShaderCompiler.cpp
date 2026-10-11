#include <ShaderCompiler.hpp>

#include <cstdlib>

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

namespace Ti::ShaderCompiler
{
    struct DXCContext
    {
        ComPtr<IDxcCompiler3> compiler;
        ComPtr<IDxcUtils> utils;
    };

    static DXCContext& GetContext()
    {
        static thread_local DXCContext context;

        HRESULT hres;
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        if (!context.compiler)
        {
            hres = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(context.compiler.GetAddressOf()));
            if (FAILED(hres))
            {
                // PC_LOGERROR("Failed to create CLSID_DxcLibrary error = {}", hres);
                exit(-1);
            }
        }
        if (!context.utils)
        {
            hres = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(context.utils.GetAddressOf()));
            if (FAILED(hres))
            {
                // PC_LOGERROR("Failed to create CLSID_DxcUtils error = {}", hres);
                exit(-1);
            }
        }

        return context;
    }

    Compiler::Compiler()
    {
        /*
        ComPtr<IDxcIncludeHandler> includeHandler;

        HRESULT hr = context.utils->CreateDefaultIncludeHandler(includeHandler.GetAddressOf());

        if (FAILED(hr))
            return false;

        hr = context.compiler->Compile(&source, args.data(), static_cast<UINT32>(args.size()), includeHandler.Get(),
                                       IID_PPV_ARGS(&result));*/
    }

} // Ti::ShaderCompiler
