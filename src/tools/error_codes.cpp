#include "error_codes.h"

namespace {

    struct GAPIErrosCategory : std::error_category {
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    struct CommonErrorsCategory : std::error_category {
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    const GAPIErrosCategory theGAPIErrosCategory{};
    const CommonErrorsCategory theCommonErrorsCategory{};

    const char* GAPIErrosCategory::name() const noexcept {
        return "GraphicsApi";
    }

    std::string GAPIErrosCategory::message(int ev) const {
        switch (static_cast<tools::GAPIErros>(ev))
        {
        case tools::GAPIErros::Success:
            return "not an error";
        case tools::GAPIErros::ShaderCompile:
            return "shader compilation failed";
        case tools::GAPIErros::ShaderLink:
            return "shader linkage failed";
        case tools::GAPIErros::FunctionArgument:
            return "wrong argument provided";
        default:
            return "unrecognized error";
        }
    }

    const char* CommonErrorsCategory::name() const noexcept {
        return "GraphicsApi";
    }

    std::string CommonErrorsCategory::message(int ev) const {
        switch (static_cast<tools::CommonErrors>(ev))
        {
        case tools::CommonErrors::IO:
            return "input/output error";
        case tools::CommonErrors::ToDo:
            return "not implemented yet";
        default:
            return "unrecognized error";
        }
    }

} // anonymous namespace

std::error_code tools::make_error_code(GAPIErros e) {
    return { static_cast<int>(e), theGAPIErrosCategory };
}

std::error_code tools::make_error_code(CommonErrors e) {
    return { static_cast<int>(e), theCommonErrorsCategory };
}
