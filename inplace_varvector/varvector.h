#pragma once

#include "utils.h"

template <class Allocator, typename... Ts>
class varvector {
/************************************
 *          Implementation          *
 ***********************************/
private:
typedef std::allocator_traits<Allocator> A;

struct Impl : public Allocator {
    /************************************
     *               DATA               *
     ***********************************/
    pointer b_, e_, z_;

    /************************************
     *              typedefs            *
     ***********************************/
    using pointer   = typename A::pointer pointer;
    using size_type = typename A::size_type size_type;

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

    /************************************
     *          constructors            *
     ***********************************/
    Impl() : Allocator(), b_(nullptr), e_(nullptr), z_(nullptr) {}
    /* implicit */ Impl(const Allocator& alloc)
        : Allocator(alloc), b_(nullptr), e_(nullptr), z_(nullptr) {}
    /* implicit */ Impl(Allocator&& alloc)
        : Allocator(std::move(alloc)), b_(nullptr), e_(nullptr), z_(nullptr) {}

    /* implicit */ Impl(size_type n, const Allocator& alloc = Allocator())
        : Allocator(alloc) {
        init(n);
    }

    Impl(Impl&& other) noexcept
        : Allocator(std::move(other)),
            b_(other.b_),
            e_(other.e_),
            z_(other.z_) {
        other.b_ = other.e_ = other.z_ = nullptr;
    }

    /********************************
     *          destructor          *
     *******************************/
    ~Impl() {
        destroy();
    }

    /****************************************
     *          memory management           *
     ***************************************/
    // note that 'allocate' and 'deallocate' are inherited from Allocator
    T* D_allocate(size_type n) {
        if constexpr (usingStdAllocator() {
            return static_cast<T*>(checkedMalloc(n * sizeof(T)));
        } else {
            return std::allocator_traits<Allocator>::allocate(*this, n);
        }
    }

    void D_deallocate(T* p, size_type n) noexcept {
        if constexpr (usingStdAllocator()) {
            free(p);
        } else {
            std::allocator_traits<Allocator>::deallocate(*this, p, n);
        }
    }

    static void S_destroy_range_a(Allocator& a, T* first, T* last) noexcept {
        for (; first != last; ++first) {
            std::allocator_traits<Allocator>::destroy(a, first);
        }
    }

    static void S_destroy_range(T* first, T* last) noexcept {
        if (!std::is_trivially_destructible_v<T>) {
            for (; first != last; ++first) {
                (first)->~T();
            }
        }
    }

    /****************************
     *          helpers         *
     ***************************/
    void swapData(Impl& other) {
        std::swap(b_, other.b_);
        std::swap(e_, other.e_);
        std::swap(z_, other.z_);
    }

    /************************************
     *          data operations         *
     ***********************************/
    inline void destroy() noexcept {
        if (b_) {
        if constexpr (usingStdAllocator()) {
            S_destroy_range(b_, e_);
        } else {
            S_destroy_range_a(*this, b_, e_);
        }

        D_deallocate(b_, size_type(z_ - b_));
        }
    }

    void init(size_type n) {
        if (UNLIKELY(n == 0)) {
        b_ = e_ = z_ = nullptr;
        } else {
    //        size_type sz = folly::goodMallocSize(n * sizeof(T)) / sizeof(T);
        b_ = D_allocate(sz);
        e_ = b_;
        z_ = b_ + sz;
        }
    }

    void set(pointer newB, size_type newSize, size_type newCap) {
        z_ = newB + newCap;
        e_ = newB + newSize;
        b_ = newB;
    }

    void reset(size_type newCap) {
        destroy();
        try {
        init(newCap);
        } catch (...) {
        init(0);
        throw;
        }
    }
    void reset() { // same as reset(0)
        destroy();
        b_ = e_ = z_ = nullptr;
    }
} impl_;


  void push_back(const T& value) {
    if (impl_.e_ != impl_.z_) {
      M_construct(impl_.e_, value);
      ++impl_.e_;
    } else {
      emplace_back_aux(value);
    }
  }

/********************************
 *          construct           *
 *******************************/
// GCC is very sensitive to the exact way that construct is called. For
//  that reason there are several different specializations of construct.
template <typename U, typename... Args>
void M_construct(U* p, Args&&... args) {
    if constexpr (usingStdAllocator()) {
        new (p) U(std::forward<Args>(args)...);
    } else {
        std::allocator_traits<Allocator>::construct(
            impl_, p, std::forward<Args>(args)...);
    }
}

template <typename U, typename... Args>
static void S_construct(U* p, Args&&... args) {
    new (p) U(std::forward<Args>(args)...);
}

template <typename U, typename... Args>
static void S_construct_a(Allocator& a, U* p, Args&&... args) {
    std::allocator_traits<Allocator>::construct(
        a, p, std::forward<Args>(args)...);
}

// scalar optimization
// TODO we can expand this optimization to: default copyable and assignable
template <typename U,
          typename Enable = typename std::enable_if_t<std::is_scalar_v<U>>>
void M_construct(U* p, U arg) {
    if constexpr (usingStdAllocator()) {
        *p = arg;
    } else {
        std::allocator_traits<Allocator>::construct(impl_, p, arg);
    }
}

template <typename U,
          typename Enable = typename std::enable_if_t<std::is_scalar_v<U>>>
static void S_construct(U* p, U arg) {
    *p = arg;
}

template <typename U,
          typename Enable = typename std::enable_if_t<std::is_scalar_v<U>>>
static void S_construct_a(Allocator& a, U* p, U arg) {
    std::allocator_traits<Allocator>::construct(a, p, arg);
}

// const& optimization
template <typename U,
          typename Enable = typename std::enable_if_t<!std::is_scalar_v<U>>>
void M_construct(U* p, const U& value) {
    if constexpr (usingStdAllocator()) {
        new (p) U(value);
    } else {
        std::allocator_traits<Allocator>::construct(impl_, p, value);
    }
}

template <typename U,
          typename Enable = typename std::enable_if_t<!std::is_scalar_v<U>>>
static void S_construct(U* p, const U& value) {
    new (p) U(value);
}

template <typename U,
          typename Enable = typename std::enable_if_t<!std::is_scalar_v<U>>>
static void S_construct_a(Allocator& a, U* p, const U& value) {
    std::allocator_traits<Allocator>::construct(a, p, value);
}

/*******************************
 *          Modifiers          *
 *******************************/
public:
void push_back(const T& value) {
    if (impl_.e_ != impl_.z_) {
        M_construct(impl_.e_, value);
        ++impl_.e_;
    } else {
        emplace_back_aux(value);
    }
}
};