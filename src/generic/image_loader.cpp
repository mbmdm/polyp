#include "image_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

namespace polyp {

ImageLoader ImageLoader::load(const std::string& path, uint32_t channels)
{
    ImageLoader output;

    int w, h, ch;
    const auto* data = stbi_load(path.c_str(), &w, &h, &ch, channels);

    if (data == nullptr)
    {
        output.mErrors << "Failed to load image " << path << ". ";
        output.mErrors << "Internal error: " << std::string(stbi_failure_reason()) << ".";
        return output;
    }

    if (channels > ch)
    {
        output.mErrors << "Requested to load " << channels << ", but only " << ch << " exist.";
        stbi_image_free((void*)data);
        return output;
    }

    output.mWidth  = w;
    output.mHeight = h;
    output.mChannels = (channels != 0) ? channels : ch;
    output.mData.resize(w * h * output.mChannels);

    memcpy(output.mData.data(), data, output.mData.size());

    stbi_image_free((void*)data);
}

bool ImageLoader::hasError(std::string& message) const
{
    if (mErrors.str().empty())
        return false;

    message = mErrors.str();

    return true;
}

}
