/*
    Copyright 2013 Julian Gonggrijp
    License to be specified (copycenter)
*/

#ifndef JG_RICH_TYPED_PTR_HPP
#define JG_RICH_TYPED_PTR_HPP

#include <utility>

namespace rich_typed_ptr {

template <class T> class weak_ptr;

template <class T>
class owner_ptr {

public:
    // move-constructible but not copyable or assignable
    owner_ptr ( ) = delete;
    owner_ptr (owner_ptr && source) : pointer(source.pointer) {
        source.pointer = 0;
    }
    ~owner_ptr ( ) { if (pointer) delete pointer; }
    owner_ptr & operator= (owner_ptr &&) = delete;

    // dereferencable
    T & operator* ( ) { return *pointer; }
    T & operator-> ( ) { return *pointer; }

    // factory function
    template <class U, class ... Us>
    friend owner_ptr<U> make (Us&& ...);

    // distributed access
    friend weak_ptr<T>;

private:
    T * pointer;

    owner_ptr (T * raw) : pointer(raw) { }

};

template <class T, class ... Ts>
owner_ptr<T> make (Ts&& ... init) {
    return new T(std::forward<Ts>(init)...);
}

template <class T>
class weak_ptr {

public:
    // copyable but not default-constructible
    weak_ptr ( ) = delete;
    weak_ptr (const owner_ptr<T> & source) : pointer(source.pointer) { }
    weak_ptr & operator= (const owner_ptr<T> & source) {
        pointer = source.pointer;
        return *this;
    }

    // dereferencable
    T & operator* ( ) { return *pointer; }
    T & operator-> ( ) { return *pointer; }

private:
    T * pointer;

};

}  // namespace rich_typed_ptr

#endif  // JG_RICH_TYPED_PTR_HPP
