#include "shader.h"

#include <glad/gl.h>

#include <type_traits>

glEngine::Shader::Shader() : mProgramId{0} 
{
    mShaders[Type::Vertex] = 0;
    mShaders[Type::Geometry] = 0;
    mShaders[Type::Fragment] = 0;
}

glEngine::Shader::~Shader()
{
    if (mProgramId)
        glDeleteProgram(mProgramId);
}

void glEngine::Shader::use() const
{
    if (mProgramId)
        glUseProgram(mProgramId);
}

std::error_code glEngine::Shader::link()
{
    return std::error_code();
}

std::error_code glEngine::Shader::compile(Type type, const std::string& shader)
{
    decltype(glCreateShader(GL_VERTEX_SHADER)) shaderProgram = 0;

    switch (type)
    {
    case glEngine::Shader::Type::Vertex:
        shaderProgram = glCreateShader(GL_VERTEX_SHADER);
        break;
    case glEngine::Shader::Type::Geometry:
        shaderProgram = glCreateShader(GL_GEOMETRY_SHADER);
        break;
    case glEngine::Shader::Type::Fragment:
        shaderProgram = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    default:
        break;
    }

    auto pShader = shader.data();
    glShaderSource(shaderProgram, 1, &pShader, NULL);
    glCompileShader(shaderProgram);

    //auto code = make_error_code(utils::GAPIErros::ShaderLink);
    return utils::GAPIErros::ShaderLink;
    //return code;
}
