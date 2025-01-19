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
        output.mErrors << "Failed to parse model " << path << std::endl;
        if (!reader.Error().empty())
            output.mErrors << "Internal error: " << reader.Error() << std::endl;
        return output;
    }

    if (!reader.Warning().empty())
        output.mErrors << "Internal warning: " << reader.Warning() << std::endl;

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    output.mErrors << "TODO: material, normal and texcoord are not supported yet" << std::endl;

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

                color.r = attrib.colors[3*size_t(idx.vertex_index) + 0];
                color.g = attrib.colors[3*size_t(idx.vertex_index) + 1];
                color.b = attrib.colors[3*size_t(idx.vertex_index) + 2];

                output.mPositions.push_back(position);
                output.mIndices.push_back(output.mIndices.size());
            }

            indexOffset += verticesCount;
        }
    }

    return output;
}

bool ModelLoader::hasError(std::string& message) const
{
    if (mErrors.str().empty())
        return false;

    message = mErrors.str();

    return true;
}

}
