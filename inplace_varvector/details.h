#pragma once

#include <type_traits>

namespace details {

    // Trivially_copyable structure fitting in 2bytes should be pass by value 
    template <typename T>
    using should_pass_by_value = std::bool_constant<
        std::is_trivially_copyable_v<T> && sizeof(T) <= 16>;
        
    template <typename T>
    using VT = std::conditional_t<should_pass_by_value, T, const T&>;

    template <typename T>
    using MT = std::conditional_t<should_pass_by_value, T, T&&>;

    template <class Allocator, typename T>
    using usingStdAllocator = std::bool_constant<
        std::is_same_v<Allocator, std::allocator<T>>>;

    using moveIsSwap = std::bool_constant<
        usingStdAllocator() || A::propagate_on_container_move_assignment()>;
} // namespace details