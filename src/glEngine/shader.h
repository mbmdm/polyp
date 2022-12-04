#ifndef SHADER_H
#define SHADER_H

#include <utils_errors.h>

#include <string>
#include <system_error>
#include <unordered_map>

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
        ~Shader();

        Shader(const Shader&) = delete;
        Shader(Shader&&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader& operator=(Shader&&) = delete;

        void use() const;
        void load(Type type, const std::string& path) {};
        std::error_code compile(Type type, const std::string& shader);
        std::error_code link();
        template<typename T, size_t dim>
        void set(const std::string& name, const T& value) const {};

    private:
        int compile_status() { return 1; };

        uint32_t mProgramId;
        std::unordered_map<Type, uint32_t> mShaders;

    };
}

#endif // SHADER_H
