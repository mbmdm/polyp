#include "utils_errors.h"

namespace {

    struct GAPIErrosCategory : std::error_category
    {
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    const char* GAPIErrosCategory::name() const noexcept
    {
        return "GraphicsApi";
    }

    std::string GAPIErrosCategory::message(int ev) const
    {
        switch (static_cast<utils::GAPIErros>(ev))
        {
        case utils::GAPIErros::Success:
            return "not an error";
        case utils::GAPIErros::ShaderCompile:
            return "shader compilation failed";
        case utils::GAPIErros::ShaderLink:
            return "shader linkage failed";
        default:
            return "unrecognized error";
        }
    }

    const GAPIErrosCategory theGAPIErrosCategory{};
}

std::error_code make_error_code(utils::GAPIErros e)
{
    return { static_cast<int>(e), theGAPIErrosCategory };
}