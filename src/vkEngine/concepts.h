#ifndef ENGINE_CONCEPTS_H
#define ENGINE_CONCEPTS_H

#include <concepts>

namespace polyp {
namespace engine {

#define CREATE_HAS_FIELD_CONCEPT(field)     \
    template<typename T>                    \
    concept has_field_##field = requires {  \
        T::field;                           \
    }

CREATE_HAS_FIELD_CONCEPT(mRoot); // concept has_field_mRoot

} // namespace engine
} // namespace polyp

#endif // ENGINE_CONCEPTS_H
