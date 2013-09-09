/*
    Copyright 2013 Julian Gonggrijp
    License to be specified (copycenter)
*/

#ifndef JG_RICHTYPEDPTR_HPP
#define JG_RICHTYPEDPTR_HPP

#include <utility>

namespace richtypedptr {

template <class T>
class owner_ptr {

public:
    // move-constructible but not copyable or assignable
    owner_ptr ( ) = delete;
    owner_ptr (const owner_ptr &) = delete;
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

private:
    T * pointer;

    owner_ptr (T * raw) : pointer(raw) { }

};

template <class T, class ... Ts>
owner_ptr<T> make (Ts&& ... init) {
    return new T(std::forward<Ts>(init)...);
}

}  // namespace richtypedptr

#endif  // JG_RICHTYPEDPTR_HPP
