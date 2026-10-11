#ifndef TITANIUM_SHADER_COMPILER_H
#define TITANIUM_SHADER_COMPILER_H

#include <cstring>
#include <vector>

namespace Ti::ShaderCompiler
{
    class Compiler
    {
    public:
        Compiler();
        ~Compiler() = default;
        enum class ShaderTarget
        {
            Dxil,
            Spriv,
            Msl
        };

        std::vector<uint8_t> getCode(const char* path, ShaderTarget ShaderTarget);

    private:
    };

} // Ti::ShaderCompiler

#endif // TITANIUM_SHADER_COMPILER_H
