#ifndef UTILS_ERRORS_H
#define UTILS_ERRORS_H

#include <system_error>

namespace utils {

    enum class GAPIErros // https://habr.com/ru/post/340604/
    {
        Success = 0,
        ShaderLink,
        ShaderCompile,
    };

    std::error_code make_error_code(GAPIErros);
}

namespace std
{
    template <>
    struct is_error_code_enum<utils::GAPIErros> : true_type {};
}

#endif // UTILS_ERRORS_H