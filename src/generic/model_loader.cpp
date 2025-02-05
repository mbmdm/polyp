#include "model_loader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include <tiny_obj_loader.h>

#include <iostream>

namespace polyp {

ModelLoader ModelLoader::load(const std::string& path)
{
    ModelLoader output;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path))
    {
        output.mErrors << "Failed to parse model " << path << ". ";
        if (!reader.Error().empty())
            output.mErrors << "Internal error: " << reader.Error() << std::endl;
        return output;
    }

    if (!reader.Warning().empty())
        output.mErrors << "Internal warning: " << reader.Warning() << std::endl;

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    output.mErrors << "TODO: material, normal and texcoord are not supported yet" << std::endl;

    float minx, maxx, miny, maxy, minz, maxz;
    minx = miny = minz = std::numeric_limits<float>::max();
    maxx = maxy = maxz = std::numeric_limits<float>::min();

    for (size_t shapeIdx = 0; shapeIdx < shapes.size(); ++shapeIdx)
    {
        const auto& mesh = shapes[shapeIdx].mesh;

        output.mPositions.reserve(mesh.indices.size() + output.mPositions.size());
        output.mIndices.reserve(mesh.indices.size() + output.mIndices.size());

        size_t indexOffset = 0;
        for (size_t faceIdx = 0; faceIdx < mesh.num_face_vertices.size(); ++faceIdx)
        {
            glm::vec3 position, color;
            const size_t verticesCount = size_t(mesh.num_face_vertices[faceIdx]);

            for (size_t vertexId = 0; vertexId < verticesCount; ++vertexId)
            {
                auto idx = mesh.indices[indexOffset + vertexId];

                position.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                position.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                position.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                #define UPDATE_MIN_MAX(dim) do {                   \
                    min##dim = std::min(min##dim, position.##dim); \
                    max##dim = std::max(max##dim, position.##dim); \
                    } while(false)

                UPDATE_MIN_MAX(x);
                UPDATE_MIN_MAX(y);
                UPDATE_MIN_MAX(z);

                color.r = attrib.colors[3*size_t(idx.vertex_index) + 0];
                color.g = attrib.colors[3*size_t(idx.vertex_index) + 1];
                color.b = attrib.colors[3*size_t(idx.vertex_index) + 2];

                output.mPositions.push_back(position);
                output.mIndices.push_back(output.mIndices.size());
            }

            indexOffset += verticesCount;
        }
    }

    output.mBoundingBox.length = (maxx - minx);
    output.mBoundingBox.height = (maxy - miny);
    output.mBoundingBox.width  = (maxz - minz);

    output.mBoundingBox.center.x = minx + (output.mBoundingBox.length / 2);
    output.mBoundingBox.center.y = miny + (output.mBoundingBox.height / 2);
    output.mBoundingBox.center.z = minz + (output.mBoundingBox.width  / 2);

    return output;
}

glm::vec3 ModelLoader::lookPosition() const
{
    glm::vec3 position = mBoundingBox.center;
    position.z += (mBoundingBox.width) / 2 + (mBoundingBox.height / 2) * 3;
    return position;
}

bool ModelLoader::hasError(std::string& message) const
{
    if (mErrors.str().empty())
        return false;

    message = mErrors.str();

    return true;
}

std::vector<glm::vec3> BoundingBox::vertices()
{
    float minx = center.x - length / 2;
    float maxx = center.x + length / 2;
    float miny = center.y - height / 2;
    float maxy = center.y + height / 2;
    float minz = center.z - width  / 2;
    float maxz = center.z + width  / 2;

    std::vector<glm::vec3>  vertices =
    {
        { minx, miny, minz },
        { maxx, miny, minz },
        { maxx, maxy, minz },
        { maxx, maxy, minz },
        { minx, maxy, minz },
        { minx, miny, minz },

        { minx, miny, maxz },
        { maxx, miny, maxz },
        { maxx, maxy, maxz },
        { maxx, maxy, maxz },
        { minx, maxy, maxz },
        { minx, miny, maxz },

        { minx, maxy, maxz },
        { minx, maxy, minz },
        { minx, miny, minz },
        { minx, miny, minz },
        { minx, miny, maxz },
        { minx, maxy, maxz },

        { maxx, maxy, maxz },
        { maxx, maxy, minz },
        { maxx, miny, minz },
        { maxx, miny, minz },
        { maxx, miny, maxz },
        { maxx, maxy, maxz },

        { minx, miny, minz },
        { maxx, miny, minz },
        { maxx, miny, maxz },
        { maxx, miny, maxz },
        { minx, miny, maxz },
        { minx, miny, minz },

        { minx, maxy, minz },
        { maxx, maxy, minz },
        { maxx, maxy, maxz },
        { maxx, maxy, maxz },
        { minx, maxy, maxz },
        { minx, maxy, minz }
    };

    return vertices;
}

}
