#include <gtest/gtest.h>
#include <string_view>

#include <ShaderCompiler.hpp>

using namespace Ti::ShaderCompiler;

Compiler c;

static constexpr const char* TriangleHLSL = R"(
struct VSOutput
{
    float4 position : SV_Position;
    float3 color    : COLOR0;
};

VSOutput VSMain(uint vertexID : SV_VertexID)
{
    float2 positions[3] = {
        float2( 0.0, -0.5),
        float2( 0.5,  0.5),
        float2(-0.5,  0.5)
    };

    float3 colors[3] = {
        float3(1.0, 0.0, 0.0),
        float3(0.0, 1.0, 0.0),
        float3(0.0, 0.0, 1.0)
    };

    VSOutput output;
    output.position = float4(positions[vertexID], 0.0, 1.0);
    output.color = colors[vertexID];

    return output;
}

float4 PSMain(VSOutput input) : SV_Target0
{
    return float4(input.color, 1.0);
}
)";
using namespace std::literals;

TEST(Spriv, BasicTriangle)
{
    const std::byte* begin = reinterpret_cast<const std::byte*>(TriangleHLSL);
    const std::byte* end = reinterpret_cast<const std::byte*>(TriangleHLSL + std::strlen(TriangleHLSL));

    const std::vector<Compiler::ShaderEntryPoint> entryPoint = {
        Compiler::ShaderEntryPoint{.stage = Compiler::ShaderStage::Vertex,
                                   .entryPoint = L"VSMain"sv,
                                   .outPutModuleName = L"Triangle.vert.spriv"},
        Compiler::ShaderEntryPoint{.stage = Compiler::ShaderStage::Pixel,
                                   .entryPoint = L"PSMain"sv,
                                   .outPutModuleName = L"Triangle.pixel.spriv"}};

    Compiler::GetCodeInfo info;
    // clang-format off
    info.setCode(std::span(begin, end))
        .setApiTarget(Compiler::ApiTarget::Spirv)
        .setEntryPoints(entryPoint);
    // clang-format on

    const std::expected exp = c.getCode(info);

    EXPECT_TRUE(exp);
    if (exp)
    {
        const std::vector<std::vector<std::byte>>& codes = exp.value();
        EXPECT_TRUE(codes.size() == 2);
    }
    else
    {
        const Compiler::CompileError& error = exp.error();

        if (std::holds_alternative<Compiler::GetCodeError>(error))
        {
            const Compiler::GetCodeError codeError = std::get<Compiler::GetCodeError>(error);
            switch (codeError)
            {
            case Compiler::GetCodeError::EmptyEntryPoints:
                std::cerr << "Empty entry points\n";
                break;

            case Compiler::GetCodeError::InvalidSourceCode:
                std::cerr << "Invalid source code\n";
                break;

            case Compiler::GetCodeError::LoadingCodeFailed:
                std::cerr << "Failed to load shader code\n";
                break;

            case Compiler::GetCodeError::InvalidShaderStage:
                std::cerr << "Invalid shader stage\n";
                break;

            case Compiler::GetCodeError::InvalidApi:
                std::cerr << "Invalid API target\n";
                break;

            case Compiler::GetCodeError::ShaderCompilationFailed:
                std::cerr << "Shader compilation failed\n";
                break;

            case Compiler::GetCodeError::UnexpectedError:
                std::cerr << "Unexpected error\n";
                break;

            default:
                std::cerr << "Unknown shader compiler error\n";
                break;
            }
        }
        else
        {
            std::cerr << std::get<std::string>(error) << '\n';
        }
    }
}
