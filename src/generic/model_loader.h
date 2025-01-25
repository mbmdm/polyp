#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <sstream>

namespace polyp {

struct BoundingBox
{
    float     width;
    float     length;
    float     height;
    glm::vec3 center;

    std::vector<glm::vec3> vertices();
};

class ModelLoader
{
public:
    static ModelLoader load(const std::string& path);

    const std::vector<glm::vec3>& positions() const { return mPositions; }
    const std::vector<uint32_t>&  indices()   const { return mIndices; }
    const std::vector<glm::vec3>& colors()    const { return mColors; }

    BoundingBox boundingBox() const { return mBoundingBox; }

    glm::vec3 lookPosition() const;

    glm::vec3 center() const { return mBoundingBox.center; }

    bool empty() const { return mPositions.empty(); }

    bool hasError(std::string& message) const;

private:
    ModelLoader() = default;

    std::vector<glm::vec3> mPositions;
    std::vector<glm::vec3> mColors;
    std::vector<uint32_t>  mIndices;

    BoundingBox            mBoundingBox;
    std::stringstream      mErrors;
};

}
