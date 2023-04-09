#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#include <system_error>

namespace tools {

/// https://habr.com/ru/post/340604/
enum class GAPIErros {
    Success = 0,
    ShaderLink,
    ShaderCompile,
    FunctionArgument,
};

/// https://habr.com/ru/post/340604/
enum class CommonErrors {
    IO = 1,
    ToDo = std::numeric_limits<CommonErrors>::max(),
};

std::error_code make_error_code(GAPIErros);

std::error_code make_error_code(CommonErrors);

} // namespace utils

namespace std {

template <>
struct is_error_code_enum<tools::GAPIErros> : std::true_type {};

template <>
struct is_error_code_enum<tools::CommonErrors> : std::true_type {};

} // namespace std

#endif // ERROR_CODES_H
