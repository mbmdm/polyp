#include "shader.h"

#include <glad/gl.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <type_traits>


glEngine::Shader::Shader() : mProgramId{0}
{ }

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

std::error_code glEngine::Shader::load(Type type, const std::string& path)
{
    std::string code;
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try 
    {
        file.open(path);
        std::stringstream ss;
        ss << file.rdbuf();
        file.close();
        code = ss.str();
    }
    catch (std::ifstream::failure& ex)
    {
        printf("ERROR: input/output file operation error\n");
        return tools::CommonErrors::IO;
    }

    return compile(type, code);
}

std::error_code glEngine::Shader::link()
{
    if (mProgramId)
    {
        printf("ERROR: shader has already be linked\n");
        return tools::GAPIErros::ShaderLink;
    }
    
    if (!mShaders.contains(Type::Vertex) || 
        !mShaders.contains(Type::Fragment)) 
    {
        printf("ERROR: at least vertex and fragment shaders should be proveided befor linkage\n");
        return tools::GAPIErros::ShaderLink;
    }

    mProgramId = glCreateProgram();

    std::for_each(mShaders.begin(), mShaders.end(), [this](const auto& data) {
        glAttachShader(mProgramId, data.second);
        });

    glLinkProgram(mProgramId);

    GLint status;
    GLchar buffer[BUFSIZ];

    glGetProgramiv(mProgramId, GL_LINK_STATUS, &status);

    if (!status) 
    {
        glGetProgramInfoLog(mProgramId, BUFSIZ, NULL, buffer);
        printf("ERROR: shader linkage error\n");
        printf("ERROR: message: %s\n", buffer);
        return tools::GAPIErros::ShaderLink;
    }

    std::for_each(mShaders.begin(), mShaders.end(), [this](const auto& data) {
        glDeleteShader(data.second);
        });
    mShaders.clear();

    return tools::GAPIErros::Success;
}

std::error_code glEngine::Shader::compile(Type type, const std::string& in_shader)
{
    switch (type)
    {
    case glEngine::Shader::Type::Vertex:
        mShaders[type] = glCreateShader(GL_VERTEX_SHADER);
        break;
    case glEngine::Shader::Type::Geometry:
        mShaders[type] = glCreateShader(GL_GEOMETRY_SHADER);
        break;
    case glEngine::Shader::Type::Fragment:
        mShaders[type] = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    default:
        return tools::GAPIErros::FunctionArgument;
    }

    auto pShader = in_shader.c_str();
    glShaderSource(mShaders[type], 1, &pShader, NULL);
    glCompileShader(mShaders[type]);

    GLint status;
    GLchar buffer[BUFSIZ];

    glGetShaderiv(mShaders[type], GL_COMPILE_STATUS, &status);

    if (!status) 
    {
        glGetShaderInfoLog(mShaders[type] , BUFSIZ, NULL, buffer);
        printf("ERROR: shader compilation error of type %s\n", to_string(type).c_str());
        printf("ERROR: message: %s\n", buffer);
        return tools::GAPIErros::ShaderCompile;
    }

    return tools::GAPIErros::Success;
}

std::string glEngine::to_string(glEngine::Shader::Type type)
{
    switch (type)
    {
    case glEngine::Shader::Type::Vertex:
        return "vertex";
    case glEngine::Shader::Type::Geometry:
        return "geometry";
    case glEngine::Shader::Type::Fragment:
        return "fragment";
    default:
        return "unrecognized";
    }
}
