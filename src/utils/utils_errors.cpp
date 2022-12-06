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
        case utils::GAPIErros::FunctionArgument:
            return "wrong argument provided";
        default:
            return "unrecognized error";
        }
    }

    const GAPIErrosCategory theGAPIErrosCategory{};

    struct CommonErrorsCategory : std::error_category
    {
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    const char* CommonErrorsCategory::name() const noexcept
    {
        return "GraphicsApi";
    }

    std::string CommonErrorsCategory::message(int ev) const
    {
        switch (static_cast<utils::CommonErrors>(ev))
        {
        case utils::CommonErrors::IO:
            return "input/output error";
        case utils::CommonErrors::ToDo:
            return "not implemented yet";
        default:
            return "unrecognized error";
        }
    }

    const CommonErrorsCategory theCommonErrorsCategory{};
}

std::error_code utils::make_error_code(GAPIErros e)
{
    return { static_cast<int>(e), theGAPIErrosCategory };
}

std::error_code utils::make_error_code(CommonErrors e)
{
    return { static_cast<int>(e), theCommonErrorsCategory };
}
