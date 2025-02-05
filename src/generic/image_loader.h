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

private:
    ImageLoader() = default;

    uint32_t               mWidth    = 0;
    uint32_t               mHeight   = 0;
    uint32_t               mChannels = 0;
    std::vector<std::byte> mData;
    std::stringstream      mErrors;
};

}
