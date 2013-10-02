/*
    Copyright 2013 Julian Gonggrijp
    Version 0, alpha 1.
*/

#ifndef JG_RICH_TYPED_PTR_HPP
#define JG_RICH_TYPED_PTR_HPP

#include <utility>
#include <type_traits>
#include <cstddef>
#include <cassert>

namespace rich_typed_ptr {

template <class T> class data_ptr;
template <class T> class weak_ptr;

template <class T>
class owner_ptr {

public:
    using value_type = T;

    // move-constructible but not copyable or assignable
    owner_ptr ( ) = delete;
    owner_ptr (owner_ptr && source) : pointer(source.pointer) {
        source.pointer = nullptr;
    }
    ~owner_ptr ( ) { if (pointer) delete pointer; }
    owner_ptr & operator= (owner_ptr &&) = delete;

    // factory functions
    template <class U, class ... Us>
    friend owner_ptr<U> make (Us&& ...);
    template <class U1, class U2, class ... Us>
    friend owner_ptr<U1> make_dynamic (Us&& ...);

    // dereferenceable
    T & operator*  ( ) const { assert(pointer); return *pointer; }
    T * operator-> ( ) const { assert(pointer); return  pointer; }

    // comparisons
    bool operator== (std::nullptr_t) const { return pointer == nullptr; }
    friend bool operator== (const owner_ptr & left, const owner_ptr & right) {
        return left.pointer == right.pointer;
    }
    friend bool operator!= (const owner_ptr & left, const owner_ptr & right) {
        return left.pointer != right.pointer;
    }

    // distributed access, usage as initializer
    friend weak_ptr<T>;
    friend data_ptr<T>;

private:
    T * pointer;

    owner_ptr (T * raw) : pointer(raw) { }

};

template <class T, class ... Ts>
owner_ptr<T> make (Ts&& ... init) {
    return new T(std::forward<Ts>(init)...);
}

// safe dynamic binding
template <class T1, class T2, class ... Ts>
owner_ptr<T1> make_dynamic (Ts&& ... init) {
    static_assert(  std::is_base_of<T1, T2>::value,
                    "Rich-typed ptr to base can only reference "
                    "base or derived"   );
    static_assert(  std::has_virtual_destructor<T1>::value,
                    "Base type must have virtual destructor "
                    "in order to prevent memory leaks"      );
    return new T2(std::forward<Ts>(init)...);
}

template <class T>
class data_ptr {

public:
    using value_type = T;

    // like owner_ptr, also assignable and initializable from nullptr
    data_ptr ( ) = delete;
    data_ptr (std::nullptr_t)         : pointer(nullptr)        { }
    data_ptr (data_ptr && source)     : pointer(source.pointer) {
        source.pointer = nullptr;
    }
    data_ptr (owner_ptr<T> && source) : pointer(source.pointer) {
        source.pointer = nullptr;
    }
    ~data_ptr ( ) { if (pointer) delete pointer; }
    data_ptr & operator= (data_ptr && source) {
        assert(! pointer);
        std::swap(pointer, source.pointer);
        return *this;
    }

    // dereferenceable
    T & operator*  ( ) const { assert(pointer); return *pointer; }
    T * operator-> ( ) const { assert(pointer); return  pointer; }

    // comparisons
    bool operator== (std::nullptr_t) const { return pointer == nullptr; }
    friend bool operator== (const data_ptr & left, const data_ptr & right) {
        return left.pointer == right.pointer;
    }
    friend bool operator!= (const data_ptr & left, const data_ptr & right) {
        return left.pointer != right.pointer;
    }

    // distributed access
    friend weak_ptr<T>;

private:
    T * pointer;

};

template <class T>
class weak_ptr {

public:
    using value_type = T;

    // copyable but not default-constructible
    weak_ptr ( ) = delete;
    weak_ptr (const owner_ptr<T> & source) : pointer(source.pointer) { }
    weak_ptr (const  data_ptr<T> & source) : pointer(source.pointer) { }
    weak_ptr & operator= (const owner_ptr<T> & source) {
        pointer = source.pointer;
        return *this;
    }
    weak_ptr & operator= (const  data_ptr<T> & source) {
        pointer = source.pointer;
        return *this;
    }

    // dereferenceable
    T & operator*  ( ) const { assert(pointer); return *pointer; }
    T * operator-> ( ) const { assert(pointer); return  pointer; }

    // comparisons
    bool operator== (std::nullptr_t) const { return pointer == nullptr; }
    friend bool operator== (weak_ptr left, weak_ptr right) {
        return left.pointer == right.pointer;
    }
    friend bool operator!= (weak_ptr left, weak_ptr right) {
        return left.pointer != right.pointer;
    }

private:
    T * pointer;

};

template <class T>
inline bool operator!= (const T & left, std::nullptr_t) {
    return !(left == nullptr);
}

template <class Ptr>
struct ptr_traits {
    using value_type = typename Ptr::value_type;
    using weak_ptr_t = weak_ptr<value_type>;
};

template <class Ptr>
typename ptr_traits<Ptr>::weak_ptr_t
weak (const Ptr & pointer) {
    static_assert(
        std::is_convertible<
            Ptr,
            typename ptr_traits<Ptr>::weak_ptr_t
        >::value,
        "Argument to rich_typed_ptr::weak must be a rich typed pointer"
    );
    return pointer;
}

}  // namespace rich_typed_ptr

#endif  // JG_RICH_TYPED_PTR_HPP
