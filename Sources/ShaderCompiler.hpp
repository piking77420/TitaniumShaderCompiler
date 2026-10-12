#ifndef TITANIUM_SHADER_COMPILER_H
#define TITANIUM_SHADER_COMPILER_H

#include <expected>
#include <string>
#include <string_view>
#include <optional>
#include <span>
#include <variant>
#include <vector>

namespace Ti::ShaderCompiler
{
    class Compiler
    {
    public:
        Compiler();
        ~Compiler() = default;
        enum struct ApiTarget
        {
            Dxil,
            Spirv,
            Msl
        };

        enum struct GetCodeError
        {
            EmptyEntryPoints,
            InvalidSourceCode,
            LoadingCodeFailed,
            InvalidShaderStage,
            InvalidApi,
            ShaderCompilationFailed,
            UnexpectedError
        };

        enum struct ShaderStage : uint8_t
        {
            Vertex,
            Hull,
            Domain,
            Geometry,
            Pixel,
            Amplification,
            Mesh,
            Compute,
            RayGeneration,
            AnyHit,
            ClosestHit,
            Miss,
            Intersection,
            Callable,
        };

        enum struct MatrixConvention
        {
            Row,
            Coloms
        };

        enum struct OptimisationLevel
        {
            None,
            O1,
            O2,
            O3,
            Od, // disable optimisation
        };

        struct ShaderEntryPoint
        {
            ShaderStage stage;

            std::wstring_view entryPoint;

            std::wstring_view outPutModuleName;
        };

        struct GetCodeInfo
        {
            std::span<const std::byte> code;

            ApiTarget apiTarget;

            std::span<const ShaderEntryPoint> entryPoints;

            MatrixConvention matrixConvention;

            OptimisationLevel optimisationLevel;

            std::optional<std::wstring_view> includePath;

            std::span<const std::wstring_view> defines;

            GetCodeInfo& setCode(const std::span<const std::byte>& newCode)
            {
                code = newCode;
                return *this;
            }

            GetCodeInfo& setApiTarget(ApiTarget newApiTarget)
            {
                apiTarget = newApiTarget;
                return *this;
            }

            GetCodeInfo& setEntryPoints(const std::span<const ShaderEntryPoint>& newEntryPoints)
            {
                entryPoints = newEntryPoints;
                return *this;
            }
            
            GetCodeInfo& setMatrixConvention(MatrixConvention newMatrixConvention)
            {
                matrixConvention = newMatrixConvention;
                return *this;
            }

            GetCodeInfo& setOptimisationLevel(OptimisationLevel newOptimisationLevel)
            {
                optimisationLevel = newOptimisationLevel;
                return *this;
            }

            GetCodeInfo& setIncludePath(std::wstring_view newIncludePath)
            {
                includePath = newIncludePath;
                return *this;
            }

            GetCodeInfo& setDefines(const std::span<const std::wstring_view>& newDefines)
            {
                 defines = newDefines;
                return *this;
            }

        };
        using CompileError = std::variant<GetCodeError, std::string>;

        std::expected<std::vector<std::vector<std::byte>>, CompileError> getCode(const GetCodeInfo& getCodeInfo);

    private:
    };

} // Ti::ShaderCompiler

#endif // TITANIUM_SHADER_COMPILER_H
