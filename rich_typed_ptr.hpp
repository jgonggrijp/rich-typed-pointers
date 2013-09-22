/*
    Copyright 2013 Julian Gonggrijp
    License to be specified (copycenter)
*/

#ifndef JG_RICH_TYPED_PTR_HPP
#define JG_RICH_TYPED_PTR_HPP

#include <utility>
#include <cstddef>
#include <cassert>

namespace rich_typed_ptr {

template <class T> class data_ptr;
template <class T> class weak_ptr;

template <class T>
class owner_ptr {

public:
    // move-constructible but not copyable or assignable
    owner_ptr ( ) = delete;
    owner_ptr (owner_ptr && source) : pointer(source.pointer) {
        source.pointer = nullptr;
    }
    ~owner_ptr ( ) { if (pointer) delete pointer; }
    owner_ptr & operator= (owner_ptr &&) = delete;

    // factory function
    template <class U, class ... Us>
    friend owner_ptr<U> make (Us&& ...);

    // dereferencable
    T & operator*  ( ) const { assert(pointer); return *pointer; }
    T * operator-> ( ) const { assert(pointer); return  pointer; }

    // (implicit) nullptr check
    operator bool ( ) const { return pointer; }
    bool operator== (std::nullptr_t) const { return pointer == nullptr; }

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

template <class T>
class data_ptr {

public:
    // like owner_ptr, also assignable and initializable from nullptr
    data_ptr ( ) = delete;
    data_ptr (data_ptr && source)     : pointer(source.pointer) {
        source.pointer = nullptr;
    }
    data_ptr (std::nullptr_t)         : pointer(nullptr)        { }
    data_ptr (owner_ptr<T> && source) : pointer(source.pointer) {
        source.pointer = nullptr;
    }
    ~data_ptr ( ) { if (pointer) delete pointer; }
    data_ptr & operator= (data_ptr && source) {
        assert(! pointer);
        std::swap(pointer, source.pointer);
        return *this;
    }

    // dereferencable
    T & operator*  ( ) const { assert(pointer); return *pointer; }
    T * operator-> ( ) const { assert(pointer); return  pointer; }

    // (implicit) nullptr check
    operator bool ( ) const { return pointer; }
    bool operator== (std::nullptr_t) const { return pointer == nullptr; }

    // distributed access
    friend weak_ptr<T>;

private:
    T * pointer;

};

template <class T>
class weak_ptr {

public:
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

    // dereferencable
    T & operator*  ( ) const { assert(pointer); return *pointer; }
    T * operator-> ( ) const { assert(pointer); return  pointer; }

    // comparisons
    operator bool ( ) const { return pointer; }
    bool operator== (std::nullptr_t) const { return pointer == nullptr; }
    friend bool operator== (weak_ptr left, weak_ptr right) {
        return left.pointer == right.pointer;
    }

private:
    T * pointer;

};

}  // namespace rich_typed_ptr

#endif  // JG_RICH_TYPED_PTR_HPP
