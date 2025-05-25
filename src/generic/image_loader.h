#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace polyp {

class ImageLoader
{
public:
    static ImageLoader load(const std::string& path, uint32_t channels = 0);

    bool empty() const { return mData.empty(); }

    bool hasError(std::string& message) const;

    uint32_t width() const { return mWidth; }

    uint32_t height() const { return mHeight; }

    uint32_t channels() const { return mChannels; }

    const std::byte* data() const { return mData.data(); }

private:
    ImageLoader() = default;

    uint32_t               mWidth    = 0;
    uint32_t               mHeight   = 0;
    uint32_t               mChannels = 0;
    std::vector<std::byte> mData;
    std::stringstream      mErrors;
};

}
