#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <system_error>

namespace glEngine 
{
    class Shader
    {
    public:
        enum class Type
        {
            Vertex,
            Geometry,
            Fragment
        };

        Shader();

        void use() const;
        void load(Type type, const std::string& path);
        void compile(Type type, const std::string& shader);
        int link() {
#error Подумать на тем, что функи возвращали error_code https://habr.com/ru/post/340604/
            std::error_code
        }

        template<typename T, size_t dim>
        void set(const std::string& name, const T& value) const;

    private:
        int compile_status();

        uint32_t mProgramId;
    };
}

#endif // SHADER_H