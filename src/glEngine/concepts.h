#ifndef CONCEPTS_H
#define CONCEPTS_H

#include <glm/glm.hpp>

#include <type_traits>


namespace glEngine
{
    template <typename T>
    concept GLSLType = requires(T)
    {
        requires 
            std::is_same_v<T, int> ||
            std::is_same_v<T, bool> ||
            std::is_same_v<T, float> ||
            std::is_same_v<T, glm::vec2> ||
            std::is_same_v<T, glm::vec3> ||
            std::is_same_v<T, glm::vec4> ||
            std::is_same_v<T, glm::mat2> ||
            std::is_same_v<T, glm::mat3> ||
            std::is_same_v<T, glm::mat4>;
    };
}

#endif // CONCEPTS_H
