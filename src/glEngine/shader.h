#ifndef SHADER_H
#define SHADER_H

#include "concepts.h"
#include <utils_errors.h>

#include <glad/gl.h>

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
        std::error_code load(Type type, const std::string& path);
        std::error_code compile(Type type, const std::string& shader);
        std::error_code link();

        template<GLSLType T>
        void set(const std::string& name, const T& value) const
        {
            auto location = glGetUniformLocation(mProgramId, name.c_str());

            if constexpr (std::is_same_v<T, bool>)
                glUniform1i(location, value);
            else if constexpr (std::is_same_v<T, int>)
                glUniform1i(location, value);
            else if constexpr (std::is_same_v<T, float>)
                glUniform1f(location, value);
            else if constexpr (std::is_same_v<T, glm::vec2>)
                glUniform2fv(location, 1, &value[0]);
            else if constexpr (std::is_same_v<T, glm::vec3>)
                glUniform3fv(location, 1, &value[0]);
            else if constexpr (std::is_same_v<T, glm::vec4>)
                glUniform4fv(location, &value[0]);
            else if constexpr (std::is_same_v<T, glm::mat2>)
                glUniformMatrix2fv(location, 1, GL_FALSE, &value[0][0]);
            else if constexpr (std::is_same_v<T, glm::mat3>)
                glUniformMatrix3fv(location, 1, GL_FALSE, &value[0][0]);
            else if constexpr (std::is_same_v<T, glm::mat4>)
                glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
        };

    private:
        uint32_t mProgramId;
        std::unordered_map<Type, uint32_t> mShaders;
    };

    std::string to_string(Shader::Type);
}

#endif // SHADER_H
