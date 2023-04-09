#ifndef ENGINE_CONCEPTS_H
#define ENGINE_CONCEPTS_H

#include <concepts>

namespace polyp {
namespace engine {

template <typename T> struct  is_shared_ptr : std::false_type {};
template <typename T> struct  is_shared_ptr<std::shared_ptr<T>> : std::true_type {};
template <typename T> concept IsSharedPtr = is_shared_ptr<T>::value;

template<typename T>
concept DispatchableType = requires(T ptr) {
    IsSharedPtr<T>;
    ptr->vk;
};

#define CREATE_HAS_FIELD_CONCEPT(field)     \
    template<typename T>                    \
    concept has_field_##field = requires {  \
        T::field;                           \
    }

CREATE_HAS_FIELD_CONCEPT(mRoot); // concept has_field_mRoot

} // namespace engine
} // namespace polyp

#endif // ENGINE_CONCEPTS_H
